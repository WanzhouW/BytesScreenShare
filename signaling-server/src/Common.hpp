#ifndef __COMMON_HPP__  
#define __COMMON_HPP__  

#include <QString>  
#include <QByteArray>  
#include <QDateTime>  
#include <unordered_map>  

#define CRITICAL() qCritical() << "[CRITICAL]" << "[" << __FILE__ << ":" << __LINE__ <<"] "
#define DEBUG() qDebug() << "[DEBUG]" << "[" << __FILE__ << ":" << __LINE__ <<"] "
#define FATAL() qFatal() << "[FATAL]" << "[" << __FILE__ << ":" << __LINE__ <<"] "
#define INFO() qInfo() << "[INFO]" << "[" << __FILE__ << ":" << __LINE__ <<"] "
#define WARNING() qWarning() << "[WARNING]" << "[" << __FILE__ << ":" << __LINE__ <<"] "

/**  
* @struct SignalingTask  
* @brief Represents a signaling task containing client information, payload, and timestamp.  
*  
* The SignalingTask structure is used to encapsulate the details of a signaling task,  
* including the client ID, the raw signaling data, and the timestamp when the task was created.  
*/  
struct SignalingTask {  
 QString _clientId;       ///< The ID of the client that sent the signaling task.  
 QString _payload;        ///< The raw signaling data.  
 qint64 _timestamp;       ///< The timestamp when the task was created.  

 /**  
  * @brief Default constructor for SignalingTask.  
  * Initializes the timestamp to 0.  
  */  
 SignalingTask() : _timestamp(0) {}  

 /**  
  * @brief Constructs a SignalingTask with the given client ID and payload.  
  * @param id The ID of the client.  
  * @param data The raw signaling data.  
  */  
 SignalingTask(const QString& id, const QString& data)  
     : _clientId(id), _payload(data), _timestamp(QDateTime::currentMSecsSinceEpoch()) {  
 }  
};  

/**  
* @enum SignalingType  
* @brief Enumerates the types of signaling messages.  
*  
* This enumeration defines the various types of signaling messages that can be exchanged  
* between clients and the server.  
*/  
enum class SignalingType {  
 REGISTER_REQUEST,  ///< Client-to-server: Register request.  
 OFFER,             ///< Client-to-server: Offer message.  
 ANSWER,            ///< Client-to-server: Answer message.  
 ICE,               ///< Client-to-server: ICE candidate message.  

 REGISTER_SUCCESS,  ///< Server-to-client: Registration success message.  
 PEER_JOINED,       ///< Server-to-client: Notification of a new peer joining.  
 PEER_LEFT,         ///< Server-to-client: Notification of a peer leaving.  

 ERROR_MESSAGE,     ///< Server-to-client: Error message.  
 UNKNOWN            ///< Unknown signaling type.  
};  

/**  
* @brief Converts a string to a SignalingType.  
* @param str The string representation of the signaling type.  
* @return The corresponding SignalingType value.  
*/  
inline SignalingType string_to_stype(const QString& str) {  
  if (str == "REGISTER_REQUEST") return SignalingType::REGISTER_REQUEST;  
  if (str == "OFFER") return SignalingType::OFFER;  
  if (str == "ANSWER") return SignalingType::ANSWER;  
  if (str == "ICE") return SignalingType::ICE;  
  if (str == "REGISTER_SUCCESS") return SignalingType::REGISTER_SUCCESS;  
  if (str == "PEER_JOINED") return SignalingType::PEER_JOINED;  
  if (str == "PEER_LEFT") return SignalingType::PEER_LEFT;  
  if (str == "ERROR_MESSAGE") return SignalingType::ERROR_MESSAGE;  
  return SignalingType::UNKNOWN;  
}  

/**  
* @brief Converts a SignalingType to its string representation.  
* @param type The SignalingType value.  
* @return The string representation of the signaling type.  
*/  
inline QString stype_to_string(SignalingType type) {  
 switch (type) {  
     case SignalingType::REGISTER_REQUEST: return "REGISTER_REQUEST";  
     case SignalingType::OFFER: return "OFFER";  
     case SignalingType::ANSWER: return "ANSWER";  
     case SignalingType::ICE: return "ICE";  
     case SignalingType::REGISTER_SUCCESS: return "REGISTER_SUCCESS";  
     case SignalingType::PEER_JOINED: return "PEER_JOINED";  
     case SignalingType::PEER_LEFT: return "PEER_LEFT";  
     case SignalingType::ERROR_MESSAGE: return "ERROR_MESSAGE";  
     default: return "UNKNOWN";  
 }  
}  

#endif // __COMMON_HPP__
