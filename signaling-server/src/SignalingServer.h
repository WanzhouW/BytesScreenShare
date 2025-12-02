#pragma once  

#include "Common.hpp"
#include "Worker.h"  

const int DEFAULT_BUFFER_SIZE = 64;  
const int DEFAULT_WORKER_NUMBER = 2;  

class ClientSession;  

/**  
* @class SignalingServer  
* @brief Manages WebSocket connections and dispatches signaling tasks to workers.  
*  
* The SignalingServer class is responsible for handling WebSocket connections,  
* managing client sessions, and dispatching signaling tasks to a worker pool.  
*/  
class SignalingServer : public QObject  
{  
   Q_OBJECT  

public:  
   /**  
    * @brief Type alias for handler functions.  
    * @param json The JSON object containing the signaling message.  
    * @param clientId The ID of the client sending the message.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   using handlerFunc = std::function<void(const QJsonObject& json, const QString& clientId, Worker* worker)>;  
private:
   /**  
    * @brief Constructs a SignalingServer instance.  
    * @param address The address to bind the WebSocket server to.  
    * @param port The port to bind the WebSocket server to.  
    * @param workerNum The number of worker threads to create.  
    * @param parent Pointer to the parent QObject (default is nullptr).  
    */  
   SignalingServer(const QHostAddress& address, quint16 port, int workerNum);

   Q_DISABLE_COPY(SignalingServer)

public:

    /**  
    * @brief Retrieves the singleton instance of the SignalingServer.  
    *  
    * This method ensures that only one instance of the SignalingServer exists  
    * throughout the application. If the instance does not already exist, it is  
    * created with the specified address, port, and number of worker threads.  
    *  
    * @param address The address to bind the WebSocket server to. Defaults to QHostAddress::Any.  
    * @param port The port to bind the WebSocket server to. Defaults to 11290.  
    * @param workerNum The number of worker threads to create. Defaults to DEFAULT_WORKER_NUMBER.  
    * @return A pointer to the singleton instance of the SignalingServer.  
    */  
    static SignalingServer* getInstance(const QHostAddress& address = QHostAddress::Any, quint16 port = 11290, int workerNum = DEFAULT_WORKER_NUMBER);
   
    
    /**  
    * @brief Destructor for the SignalingServer class.  
    */  
   ~SignalingServer();  

   /**  
    * @brief Starts the WebSocket server.  
    * @param address The address to bind the server to.  
    * @param port The port to bind the server to.  
    * @return True if the server starts successfully, false otherwise.  
    */  
   bool start(const QHostAddress& address = QHostAddress::Any, quint16 port = 11290);

   /**  
    * @brief Stops the WebSocket server.  
    * @return True if the server stops successfully, false otherwise.  
    */  
   bool stop();  

private:  
   /**  
    * @brief Registers handler functions for solving signaling messages.  
    */  
   void registerHandlers();  

   /**  
    * @brief Dispatches a signaling task to a worker.  
    * @param task The signaling task to be processed.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void dispatchMessage(const SignalingTask& task, Worker* worker);  

   /**  
    * @brief Handles a "register" signaling message.  
    * @param sessionList The list of active sessions.  
    * @param jsonObj The JSON object containing the message.  
    * @param srcId The ID of the source client.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void handleRegister(const QJsonArray& sessionList, const QJsonObject& jsonObj, const QString& srcId, Worker* worker);  

   /**  
    * @brief Handles an "offer" signaling message.  
    * @param sessionList The list of active sessions.  
    * @param jsonObj The JSON object containing the message.  
    * @param srcId The ID of the source client.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void handleOffer(const QJsonArray& sessionList, const QJsonObject& jsonObj, const QString& srcId, Worker* worker);  

   /**  
    * @brief Handles an "answer" signaling message.  
    * @param sessionList The list of active sessions.  
    * @param jsonObj The JSON object containing the message.  
    * @param srcId The ID of the source client.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void handleAnswer(const QJsonArray& sessionList, const QJsonObject& jsonObj, const QString& srcId, Worker* worker);  

   /**  
    * @brief Handles an "ice" signaling message.  
    * @param sessionList The list of active sessions.  
    * @param jsonObj The JSON object containing the message.  
    * @param srcId The ID of the source client.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void handleIce(const QJsonArray& sessionList, const QJsonObject& jsonObj, const QString& srcId, Worker* worker);  

   /**  
    * @brief Handles an error message.  
    * @param message The error message.  
    * @param srcId The ID of the source client.  
    * @param worker Pointer to the Worker instance processing the task.  
    */  
   void handleError(const QString& message, const QString& srcId, Worker* worker);  

   /**  
    * @brief Retrieves the list of peers.  
    * @return A JSON array containing the list of peers.  
    */  
   QJsonArray getPeerList();  

   /**  
    * @brief Checks if a client is online.  
    * @param sessionList The list of active sessions.  
    * @param clientId The ID of the client to check.  
    * @return True if the client is online, false otherwise.  
    */  
   bool isOnline(const QJsonArray& sessionList, const QString& clientId);  

signals:  
   /**  
    * @brief Signal emitted when a new session is added.  
    * @param clientId The ID of the new client session.  
    */  
   void sigAddSession(const QString& clientId);  

   /**  
    * @brief Signal emitted when a session is removed.  
    * @param clientId The ID of the removed client session.  
    */  
   void sigRemoveSession(const QString& clientId);  

private:  
   /**  
    * @brief Handles a new WebSocket connection.  
    */  
   void onNewConnection();  

   /**  
    * @brief Handles a WebSocket disconnection.  
    */  
   void onDisconnected();  

   /**  
    * @brief Processes data received from a client.  
    * @param srcId The ID of the source client.  
    * @param data The data received from the client.  
    */  
   void onClientDataReady(const QString& srcId, const QString& data);  

   /**  
    * @brief Handles the result of a worker's task.  
    * @param targetClient The ID of the target client.  
    * @param message The result message.  
    */  
   void onWorkerResult(const QString& targetClient, const QString& message);  

   /**  
    * @brief Adds a new session to the session list.  
    * @param clientId The ID of the new client session.  
    */  
   void onAddSession(const QString& clientId);  

   /**  
    * @brief Removes a session from the session list.  
    * @param clientId The ID of the removed client session.  
    */  
   void onRemoveSession(const QString& clientId);  

private:  
   QWebSocketServer* _server;  ///< Pointer to the WebSocket server instance.  
   QHash<QString, ClientSession*> _sessions;  ///< Hash map of client sessions.  
   WorkerPool* _workerPool;  ///< Pointer to the worker pool instance.  
   QHash<QString, handlerFunc> _handlerMap;  ///< Map of handler functions for signaling messages.  
   QJsonArray _session_list;  ///< List of active client sessions.  
   QHostAddress _hostAddress;  ///< Address the server is bound to.  
   quint16 _port;  ///< Port the server is bound to.  
   bool _isRunning;  ///< Flag indicating whether the server is running.  
};  

/**  
* @class ClientSession  
* @brief Represents a client session for a WebSocket connection.  
*  
* The ClientSession class manages the lifecycle of a WebSocket connection  
* and provides an interface for sending and receiving data.  
*/  
class ClientSession : public QObject  
{  
   Q_OBJECT  

public:  
   /**  
    * @brief Constructs a ClientSession instance.  
    * @param sock Pointer to the QWebSocket instance.  
    * @param parent Pointer to the parent QObject (default is nullptr).  
    */  
   explicit ClientSession(QWebSocket* sock, QObject* parent = nullptr);  

   /**  
    * @brief Destructor for the ClientSession class.  
    */  
   ~ClientSession();  

   /**  
    * @brief Retrieves the unique identifier of the session.  
    * @return The session ID.  
    */  
   QString id() const;  

   /**  
    * @brief Sends data to the client.  
    * @param data The data to send.  
    */  
   void sendData(const QString& data);  

signals:  
   /**  
    * @brief Signal emitted when new data is received from the client.  
    * @param sessionId The ID of the client session.  
    * @param data The data received from the client.  
    */  
   void sigDataReady(const QString& sessionId, const QString& data);  

   /**  
    * @brief Signal emitted when the client disconnects.  
    * @param sessionId The ID of the disconnected client session.  
    */  
   void sigDisconnected(const QString& sessionId);  

private:  
   /**  
    * @brief Handles text messages received from the client.  
    * @param message The message received from the client.  
    */  
   void onTextMessageReceived(const QString& message);  

   /**  
    * @brief Handles the disconnection of the client.  
    */  
   void onDisconnected();  

private:  
   QWebSocket* _socket;  ///< Pointer to the QWebSocket instance.  
   QString _id;  ///< Unique identifier for the client session.  
};
