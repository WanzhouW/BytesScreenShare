#include "SignalingServer.h"

SignalingServer::SignalingServer(const QHostAddress& address, quint16 port, int workerNum, QObject* parent)
: QObject(parent),
_server(new QWebSocketServer(QStringLiteral("Signaling Server"),
    QWebSocketServer::NonSecureMode, this)),
_workerPool(new WorkerPool(this)),
_hostAddress(address),
_port(port),
_isRunning(false)
{
    registerHandlers();
    QObject::connect(_server, &QWebSocketServer::newConnection, this, &SignalingServer::onNewConnection);
    QObject::connect(_workerPool, &WorkerPool::sigWorkerResult, this, &SignalingServer::onWorkerResult);
    QObject::connect(this, &SignalingServer::sigAddSession, this, &SignalingServer::onAddSession);
    QObject::connect(this, &SignalingServer::sigRemoveSession, this, &SignalingServer::onRemoveSession);

    auto processor = [this](const SignalingTask& task, Worker* source) {
        this->dispatchMessage(task, source);
    };
    _workerPool->start(workerNum, processor);
}

SignalingServer::~SignalingServer()
{}

bool SignalingServer::start(const QHostAddress& address, quint16 port)
{
    if (_isRunning == false) {
        qInfo() << "Signaling Server is running! Listen on: " << address.toString() << ":" << port;
        _hostAddress = address;
        _port = port;
        _server->listen(_hostAddress, port);
        _isRunning = true;
        return true;
    }
    qWarning() << "The server has already started!";
    return false;
}

bool SignalingServer::stop()
{
    if (_isRunning == true) {
        _server->close();
        qInfo() << "Signaling Server is closed!";
        _isRunning = false;
        return true;
    }
    qWarning() << "The server has already shut down!";
    return false;
}



void SignalingServer::registerHandlers()
{
    _handlerMap["REGISTER_REQUEST"] = [this](const QJsonObject& j, const QString& id, Worker* w) {
        handleRegister(this->_session_list, j, id, w);
        };

    _handlerMap["OFFER"] = [this](const QJsonObject& j, const QString& id, Worker* w) {
        handleOffer(this->_session_list, j, id, w);
        };

    _handlerMap["ANSWER"] = [this](const QJsonObject& j, const QString& id, Worker* w) {
        handleAnswer(this->_session_list, j, id, w);
        };

    _handlerMap["ICE"] = [this](const QJsonObject& j, const QString& id, Worker* w) {
        handleIce(this->_session_list, j, id, w);
        };
}

void SignalingServer::dispatchMessage(const SignalingTask& task, Worker* worker)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(task._payload.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError || doc.isNull()) {
        handleError("Invalid JSON", task._clientId, worker);
        return;
    }

    QJsonObject rootJson = doc.object();
    
    // B. 获取消息类型
    if (!rootJson.contains("type") || !rootJson["type"].isString()) {
        handleError("Invalid type", task._clientId, worker);
        return;
    }

    QString type = rootJson["type"].toString();

    if (_handlerMap.contains(type)) {
        _handlerMap[type](rootJson, task._clientId, worker);
    }
    else {
        handleError("Invalid type", task._clientId, worker);
    }
    return;
}

void SignalingServer::handleRegister(const QJsonArray& sessionList, const QJsonObject& jsonObj, const QString& srcId, Worker* worker)
{
    QJsonObject data;
    data.insert("peerId", srcId);
    data.insert("message", "Welcome!");
    data.insert("peers", sessionList);

    QJsonObject jsonRet = jsonObj;
    jsonRet.insert("type", stype_to_string(SignalingType::REGISTER_SUCCESS));
    jsonRet.insert("from", "Server");
    jsonRet.insert("to", srcId);
    jsonRet.insert("data", data);

    QString ret = QJsonDocument(jsonRet).toJson(QJsonDocument::Compact);
    emit sigAddSession(srcId);
    emit worker->sigSendResponse(srcId, QString(ret));

    if (!sessionList.isEmpty()) {
        QJsonObject joinData;
        joinData.insert("id", srcId);
        
        QJsonObject jsonNotify;
        jsonNotify.insert("type", stype_to_string(SignalingType::PEER_JOINED));
        jsonNotify.insert("from", "Server");
        jsonNotify.insert("to", QJsonValue::Null);
        jsonNotify.insert("data", joinData);

        for (const QJsonValue& val : sessionList) {
            QString targetId = val.toString();
            // 跳过自己 (虽然 sessionList 通常不包含自己，但防御性编程)
            if (targetId == srcId) continue;

            jsonNotify["to"] = targetId;
            QString notifyPayload = QJsonDocument(jsonNotify).toJson(QJsonDocument::Compact);

            emit worker->sigSendResponse(targetId, notifyPayload);
        }
    }
}

void SignalingServer::handleOffer(const QJsonArray& sessionList, const QJsonObject& jsonObj, 
    const QString& srcId, Worker* worker)
{
    if (!jsonObj.contains("to") || !jsonObj["to"].isString()) {
        handleError("Missing 'to' field in OFFER", srcId, worker);
        return;
    }
    QString targetId = jsonObj["to"].toString();

    QJsonObject forwardJson;
    forwardJson.insert("type", stype_to_string(SignalingType::OFFER));
    forwardJson.insert("from", srcId);
    forwardJson.insert("to", targetId);
    if (!isOnline(sessionList, targetId)) {
        char buffer[DEFAULT_BUFFER_SIZE];
        memset(buffer, 0, DEFAULT_BUFFER_SIZE);
        snprintf(buffer, DEFAULT_BUFFER_SIZE, "%s is not online", targetId.toStdString().c_str());
        handleError(QString(buffer), srcId, worker);
    }

    if (jsonObj.contains("data")) {
        forwardJson.insert("data", jsonObj["data"]);
    }
    else {
        forwardJson.insert("data", QJsonObject());
    }


    QString payload = QJsonDocument(forwardJson).toJson(QJsonDocument::Compact);
    emit worker->sigSendResponse(targetId, payload);

    qDebug() << "Forwarded OFFER from" << srcId << "to" << targetId;
}

void SignalingServer::handleAnswer(const QJsonArray& sessionList, const QJsonObject& jsonObj, 
    const QString& srcId, Worker* worker)
{
    if (!jsonObj.contains("to") || !jsonObj["to"].isString()) {
        handleError("Missing 'to' field in ANSWER", srcId, worker);
        return;
    }
    QString targetId = jsonObj["to"].toString();
    if (!isOnline(sessionList, targetId)) {
        char buffer[DEFAULT_BUFFER_SIZE];
        memset(buffer, 0, DEFAULT_BUFFER_SIZE);
        snprintf(buffer, DEFAULT_BUFFER_SIZE, "%s is not online", targetId.toStdString().c_str());
        handleError(QString(buffer), srcId, worker);
    }

    QJsonObject forwardJson;
    forwardJson.insert("type", stype_to_string(SignalingType::ANSWER));
    forwardJson.insert("from", srcId);
    forwardJson.insert("to", targetId);

    if (jsonObj.contains("data")) {
        forwardJson.insert("data", jsonObj["data"]);
    }
    else {
        forwardJson.insert("data", QJsonObject());
    }

    QString payload = QJsonDocument(forwardJson).toJson(QJsonDocument::Compact);
    emit worker->sigSendResponse(targetId, payload);

    qDebug() << "Forwarded ANSWER from" << srcId << "to" << targetId;
}

// 处理 ICE Candidate 转发
void SignalingServer::handleIce(const QJsonArray& sessionList, const QJsonObject& jsonObj, 
    const QString& srcId, Worker* worker)
{
    if (!jsonObj.contains("to") || !jsonObj["to"].isString()) {
        handleError("Missing 'to' field in ICE", srcId, worker);
        return;
    }
    QString targetId = jsonObj["to"].toString();
    if (!isOnline(sessionList, targetId)) {
        char buffer[DEFAULT_BUFFER_SIZE];
        memset(buffer, 0, DEFAULT_BUFFER_SIZE);
        snprintf(buffer, DEFAULT_BUFFER_SIZE, "%s is not online", targetId.toStdString().c_str());
        handleError(QString(buffer), srcId, worker);
    }

    QJsonObject forwardJson;
    forwardJson.insert("type", stype_to_string(SignalingType::ICE));
    forwardJson.insert("from", srcId);
    forwardJson.insert("to", targetId);

    if (jsonObj.contains("data")) {
        forwardJson.insert("data", jsonObj["data"]);
    }
    else {
        forwardJson.insert("data", QJsonObject());
    }

    QString payload = QJsonDocument(forwardJson).toJson(QJsonDocument::Compact);
    emit worker->sigSendResponse(targetId, payload);
}

void SignalingServer::handleError(const QString& message, const QString& clientId, Worker* worker)
{
    QJsonObject data;
    data.insert("message", message);

    QJsonObject errorJson;
    errorJson.insert("type", stype_to_string(SignalingType::ERROR_MESSAGE));
    errorJson.insert("from", "Server");
    errorJson.insert("to", clientId);
    errorJson.insert("data", data);
    auto payload = QJsonDocument(errorJson).toJson(QJsonDocument::Compact);
    qInfo() << "[" << stype_to_string(SignalingType::ERROR_MESSAGE) << "] " <<
    "Client: " << clientId << " << " << message;
    emit worker->sigSendResponse(clientId, QString(payload));
}

QJsonArray SignalingServer::getPeertList()  
{  
   QJsonArray jsonArray;  
   for (auto it = _sessions.begin(); it != _sessions.end(); ++it) {  
       jsonArray.append(it.key());  
   }  
   return jsonArray;  
}

bool SignalingServer::isOnline(const QJsonArray& sessionList, const QString& clientId)
{
    for (auto it = sessionList.begin(); it != sessionList.end(); it++) {
        if ((*it).toString() == clientId) return true;
    }
    return false;
}

void SignalingServer::onNewConnection()
{
    auto webSocket = _server->nextPendingConnection();
    ClientSession* session = new ClientSession(webSocket, this);
    QString client_id = session->id();

    _sessions[client_id] = session;

    QObject::connect(session, &ClientSession::sigDisconnected, this, &SignalingServer::onDisconnected);
    QObject::connect(session, &ClientSession::sigDataReady, this,
        &SignalingServer::onClientDataReady);
}

void SignalingServer::onDisconnected()
{
     auto clientSession = qobject_cast<ClientSession*>(sender());
    _sessions.remove(clientSession->id());
    emit sigRemoveSession(clientSession->id());
}

void SignalingServer::onClientDataReady(const QString& srcId, const QString& data)
{
    SignalingTask task(srcId, data);
    _workerPool->submitTask(task);
}

void SignalingServer::onWorkerResult(const QString& targetClinet, const QString& message)
{
    if (!_sessions.contains(targetClinet) || _sessions[targetClinet] == nullptr) {
        qWarning() << targetClinet << " has already offlined";
        return;
    }
    auto session = _sessions[targetClinet];
    session->sendData(message);
}

void SignalingServer::onAddSession(const QString& clientId)
{
    _session_list.append(clientId);
}

void SignalingServer::onRemoveSession(const QString& clientId)
{
    if (_sessions.contains(clientId)) {
        _sessions.remove(clientId);
    }
    _session_list = getPeertList();
}

// ClientSession >>>>>>>>>>>>>>>>>

ClientSession::ClientSession(QWebSocket* sock, QObject* parent) :
	QObject(parent), _socket(sock)
{
	assert(sock != nullptr);
	_socket->setParent(this);

	_id = QUuid::createUuid().toString(QUuid::Id128);
	qDebug() << "ClientSession created. ID:" << _id << "Description: " <<
		"listen on port " << _socket->localPort() << 
        "; Peer address and port " << _socket->peerAddress() << ":" << _socket->peerPort();

	connect(_socket, &QWebSocket::textMessageReceived, this, &ClientSession::onTextMessageReceived);
	connect(_socket, &QWebSocket::disconnected, this, &ClientSession::onDisconnected);
}

ClientSession::~ClientSession() {}

QString ClientSession::id() const
{
	return _id;
}

void ClientSession::sendData(const QString& data)
{
    if (_socket == nullptr) {
        qWarning() << "ClientSession::sendData called with null socket. ID:" << _id;
        return;
    }

    if (_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "ClientSession::sendData failed. Socket not connected. ID:" << _id;
        return;
    }

    qDebug() << data;
    qint64 bytesSent = _socket->sendTextMessage(data);
    if (bytesSent == -1) {
        qWarning() << "ClientSession::sendData failed to send message. ID:" << _id
            << "Error:" << _socket->errorString();
        if (_socket->error() != QAbstractSocket::SocketTimeoutError) {
            _socket->close();
        }
    }
    else if (bytesSent != data.toUtf8().size()) {
        qDebug() << "ClientSession::sendData partial send. ID:" << _id
            << "Sent:" << bytesSent << "Expected:" << data.size();
    }
}

void ClientSession::onTextMessageReceived(const QString& message)
{
	emit sigDataReady(_id, message);
}

void ClientSession::onDisconnected()
{
    _socket->close();
	emit sigDisconnected(_id);
}


