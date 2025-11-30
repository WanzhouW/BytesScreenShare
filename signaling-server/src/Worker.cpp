#include "Worker.h"

Worker::Worker(int id, BlockingQueue<SignalingTask>::bqPtr queue, SignalingProcessor processor, QObject* parent)
	: QObject(parent), _workerId(id), _queue(queue), _isRunning(false), _processor(processor)
{}

Worker::~Worker()
{}

void Worker::stop()
{
    if (!_isRunning.loadRelaxed()) {
        return;
    }
    _isRunning.storeRelaxed(false);
}

void Worker::startLoop()
{
    qDebug() << "Worker" << _workerId << "start";
    _isRunning.testAndSetRelaxed(false, true);
    while (true) {
        SignalingTask task;
        if (_isRunning.loadRelaxed()) {
            if (_queue->pop(task, DEFAULT_TIMEOUT)) {
                processMessage(task);
            }
        }
        else {
            if (_queue->tryPop(task)) {
                processMessage(task);
            }
            else {
                break;
            }
        }
    }
    qDebug() << "Worker" << _workerId << "exit";
    emit finished();
}

int Worker::getId()
{
    return _workerId;
}

void Worker::processMessage(const SignalingTask& task)
{
    _processor(task, this);
}

WorkerPool::WorkerPool(QObject* parent):
    QObject(parent), _taskQueue(new BlockingQueue<SignalingTask>), _isRunning(false)
{}

WorkerPool::~WorkerPool()
{
    if (_isRunning.loadRelaxed()) {
        stop();
    }
}

bool WorkerPool::start(size_t threadCount, Worker::SignalingProcessor processor)
{
    if (!_isRunning.testAndSetRelaxed(false, true)) {
        qWarning() << "WorkerPool: Already running.";
        return false;
    }
    else if (threadCount == 0) {
        qWarning() << "Thread count must be greater than 0, you input: " << threadCount;
        return false;
    }

    for (int i = 0; i < threadCount; ++i) {
        QThread* thread = new QThread(this);
        Worker* worker = new Worker(i + 1, _taskQueue, processor, nullptr);

        worker->moveToThread(thread);

        // registe signal with slot function
        connect(thread, &QThread::started, worker, &Worker::startLoop);
        connect(worker, &Worker::sigSendResponse,
            this, &WorkerPool::onSendResponse);

        _threads.append(thread);
        _workers.append(worker);
        thread->start();
    }
    qDebug() << "WorkerPool started with" << threadCount << "threads.";
    
    return true;
}

void WorkerPool::stop()
{
    if (!_isRunning.testAndSetOrdered(true, false)) { return; }

    for (Worker* worker : _workers) {
        worker->stop();
    }

    if (_taskQueue != nullptr) {
        _taskQueue->notifyAll();
    }

    for (QThread* thread : _threads) {
        if (thread->isRunning()) {
            thread->quit();
            thread->wait();
        }
    }
    qInfo() << "wait successfully!";
    _threads.clear();
    _workers.clear();
}

bool WorkerPool::submitTask(const SignalingTask& task)
{
    if (_isRunning.loadRelaxed() == false || _taskQueue == nullptr) {
        qWarning() << "WorkerPool is not running!";
        return false;
    }
    _taskQueue->push(task);
    return true;
}

int WorkerPool::getQueueSize() const { return _taskQueue->size(); }

void WorkerPool::onSendResponse(const QString& targetId, const QString& json)
{
    emit sigWorkerResult(targetId, json);
}

void WorkerPool::handleWorkerFinished() {
    QThread* thread = qobject_cast<QThread*>(QObject::sender());
    if (thread) {
        qDebug() << "Thread finished:" << thread;
    }
}