#ifndef __WORKER_H__  
#define __WORKER_H__  

#include "Common.hpp"  
#include "BlockingQueue.hpp"  

const int DEFAULT_TIMEOUT = 100;  

/**  
* @class Worker  
* @brief Represents a worker thread that processes tasks from a shared blocking queue.  
*  
* The Worker class is responsible for continuously fetching tasks from a shared blocking queue,  
* processing them, and emitting signals to notify the main thread of the results.  
*/  
class Worker : public QObject  
{  
  Q_OBJECT  

public:  
  /**  
   * @brief Type alias for the task processing function.  
   * @param task The task to be processed.  
   * @param source Pointer to the Worker instance that processes the task.  
   */  
  using SignalingProcessor = std::function<void(const SignalingTask& task, Worker* source)>;  

  /**  
   * @brief Constructs a Worker instance.  
   * @param id Unique identifier for the Worker.  
   * @param queue Pointer to the shared blocking queue containing tasks.  
   * @param processor Function to process tasks.  
   * @param parent Pointer to the parent QObject (default is nullptr).  
   */  
  explicit Worker(int id, BlockingQueue<SignalingTask>::bqPtr queue, SignalingProcessor processor, QObject* parent = nullptr);  
  /**  
   * @brief Destructor for the Worker class.  
   */  
  ~Worker();  
  /**  
   * @brief Requests the Worker to stop its processing loop.  
   */  
  void stop();
 
  /**  
   * @brief Starts the Worker thread's main processing loop.  
   *  
   * This function continuously fetches tasks from the queue and processes them  
   * until the Worker is requested to stop.  
   */  
  void startLoop();

  /**  
   * @brief Retrieves the unique identifier of the Worker.  
   * @return The Worker ID.  
   */  
  int getId();

signals:  
  /**  
   * @brief Signal emitted when a task is processed and a response is ready.  
   * @param targetId The ID of the target client.  
   * @param json The processed data in JSON format.  
   */  
  void sigSendResponse(const QString& targetId, const QString& json);  

  /**  
   * @brief Signal emitted when the Worker exits its processing loop and completes cleanup.  
   */  
  void finished();

private:  
  /**  
   * @brief Handles the processing of a single task.  
   * @param task The task to be processed.  
   */  
  void processMessage(const SignalingTask& task);  

private:  
  int _workerId;  ///< Unique identifier for the Worker.  
  BlockingQueue<SignalingTask>::bqPtr _queue;  ///< Shared blocking queue for tasks.  
  QAtomicInt _isRunning;  ///< Atomic flag indicating whether the Worker is running.  
  SignalingProcessor _processor;  ///< Function to process tasks.  
};  

/**  
* @class WorkerPool  
* @brief Manages a pool of Worker threads to process tasks concurrently.  
*  
* The WorkerPool class is responsible for creating and managing multiple Worker threads,  
* distributing tasks among them, and ensuring safe shutdown of all threads.  
*/  
class WorkerPool : public QObject  
{  
   Q_OBJECT  

public:  
   /**  
    * @brief Constructs a WorkerPool instance.  
    * @param parent Pointer to the parent QObject (default is nullptr).  
    */  
   explicit WorkerPool(QObject* parent = nullptr);  

   /**  
    * @brief Destructor for the WorkerPool class. Ensures safe shutdown of all threads.  
    */  
   ~WorkerPool();  

   Q_DISABLE_COPY(WorkerPool)  ///< Disables copy and assignment.

   /**  
    * @brief Starts the thread pool and creates Worker instances.  
    * @param threadCount The number of threads to create.  
    * @param processor The task processing logic to inject into each Worker.  
    * @return True if the thread pool starts successfully, false otherwise.  
    */  
   bool start(size_t threadCount, Worker::SignalingProcessor processor);  

   /**  
    * @brief Stops all Worker loops and waits for all threads to exit safely.
    * @return True if the thread pool stops successfully, false otherwise.
    */  
   bool stop();  

   /**  
    * @brief Producer interface: Submits a task to the queue.  
    * @param task The signaling task to be processed.  
    * @return True if the task is successfully submitted, false if the thread pool is stopped.  
    */  
   bool submitTask(const SignalingTask& task);  

   /**  
    * @brief Retrieves the current size of the task queue.  
    * @return The size of the task queue.  
    */  
   int getQueueSize() const;  

signals:  
   /**  
    * @brief Forwards the processing results from Workers to the TcpSignalingServer.  
    * @param targetId The target client ID.  
    * @param json The response data.  
    */  
   void sigWorkerResult(const QString& targetId, const QString& json);  

private:

    void onSendResponse(const QString& targetId, const QString& json);

private:  
   /**  
    * @brief Slot function: Handles cleanup after a Worker thread exits safely.  
    * @param worker Pointer to the Worker instance.  
    */  
   void handleWorkerFinished();  

private:  
   BlockingQueue<SignalingTask>::bqPtr _taskQueue;  ///< Task queue owned by the WorkerPool.  
   QVector<QThread*> _threads;                      ///< Container for QThread instances.  
   QVector<Worker*> _workers;                       ///< Container for Worker objects.  
   QAtomicInt _isRunning;                           ///< Atomic flag indicating whether the thread pool is running.  
   QMutex _mutex;                                   ///< Mutex to protect shared state.    
};  

#endif // __WORKER_H__
