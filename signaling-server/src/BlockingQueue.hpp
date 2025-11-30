#ifndef __BLOCKING_QUEUE_HPP__  
#define __BLOCKING_QUEUE_HPP__  

#include <QQueue>  
#include <QMutex>  
#include <QWaitCondition>  
#include <chrono>
#include <memory>

/**  
* @class BlockingQueue  
* @brief A thread-safe blocking queue implementation.  
*  
* This class provides a thread-safe queue that supports blocking operations  
* for adding and removing elements. It uses Qt's QQueue, QMutex, and QWaitCondition  
* to ensure thread safety and synchronization.  
*  
* @tparam T The type of elements stored in the queue.  
*/  
template<class T>  
class BlockingQueue  
{  
public:
 using bqPtr = std::shared_ptr<BlockingQueue<T>>;

 /**  
  * @brief Constructs a BlockingQueue object.  
  */  
 BlockingQueue() {}  

 /**  
  * @brief Destroys the BlockingQueue object.  
  */  
 ~BlockingQueue() {}  

 /**  
  * @brief Pushes an element into the queue.  
  * @param ele The element to be added to the queue.  
  * @return true if the element is successfully added.  
  */  
 bool push(const T& ele) {  
     {  
         QMutexLocker guard(&_mutex);  
         _queue.enqueue(ele);  
     }  
     notifyOne();  
     return true;  
 }  

 /**  
  * @brief Pops an element from the queue with a timeout.  
  * @param value Reference to store the dequeued element.  
  * @param timeoutMs The maximum time to wait in milliseconds.  
  * @return true if an element is successfully dequeued, false if timeout occurs.  
  */  
 bool pop(T& value, int timeoutMs) {  
     QMutexLocker guard(&_mutex);  
     while (_queue.isEmpty()) {  
         if (!_cond.wait(&_mutex, timeoutMs)) { 
             if (_queue.isEmpty()) {
                 return false;
             }  
         }  
     }  
     value = std::move(_queue.dequeue());
     return true;  
 }  

 /**  
  * @brief Attempts to pop an element from the queue without blocking.  
  * @param value Reference to store the dequeued element.  
  * @return true if an element is successfully dequeued, false otherwise.  
  */  
 bool tryPop(T& value) {
     QMutexLocker guard(&_mutex);
     if (_queue.isEmpty()) return false;
     value = std::move(_queue.dequeue());
     return true;
 }

 /**  
  * @brief Gets the current size of the queue.  
  * @return The number of elements in the queue.  
  */  
 size_t size() {  
     QMutexLocker guard(&_mutex);  
     return _queue.size();  
 }  

 /**  
  * @brief Checks if the queue is empty.  
  * @return true if the queue is empty, false otherwise.  
  */  
 bool empty() {  
     QMutexLocker guard(&_mutex);  
     return _queue.isEmpty();  
 }  

 /**  
  * @brief Notifies one waiting thread.  
  */  
 void notifyOne() {  
     _cond.wakeOne();  
     return;  
 }  

 /**  
  * @brief Notifies all waiting threads.  
  */  
 void notifyAll() {  
     _cond.wakeAll();  
     return;  
 }  

private:  
 QQueue<T> _queue; ///< The underlying queue to store elements.  
 QMutex _mutex; ///< Mutex to ensure thread safety.  
 QWaitCondition _cond; ///< Condition variable for synchronization.  
};  

#endif // __BLOCKING_QUEUE_HPP__
