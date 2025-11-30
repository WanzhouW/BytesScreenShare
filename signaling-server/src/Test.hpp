#ifndef __TEST_HPP__
#define __TEST_HPP__
#include "BlockingQueue.hpp"
#include "SignalingServer.h"
#include "Worker.h"
#include <thread>  
#include <vector>  
#include <Windows.h>  
#include <iostream>  
#include <QDebug>

class Test {
public:
    Test() {}
    ~Test() {}

    // @brief Test for class BlockingQueue
    void testBlockingQueue() {
        BlockingQueue<int>::bqPtr qPtr = std::make_shared<BlockingQueue<int>>();
        std::vector<std::thread> threads;
        if (qPtr->empty()) {
            std::cout << "is empty!" << std::endl;
        }

        // Producer thread 1  
        threads.emplace_back([&]() {
            for (int cot = 0; cot < 100; ++cot) {
                qPtr->push(cot);
                if (cot == 20) {
                    std::cout << qPtr->size() << std::endl;
                }
                Sleep(1); // Sleep for 1 millisecond  
            }
            });

        // Producer thread 2  
        threads.emplace_back([&]() {
            for (int cot = 101; cot < 200; ++cot) {
                qPtr->push(cot);
                Sleep(1); // Sleep for 1 millisecond  
            }
            });

        // Consumer thread  
        threads.emplace_back([&]() {
            int cot = 200;
            while (cot--) {
                int ele = 0;
                if(qPtr->pop(ele, 500))
                    qDebug() << ele;
            }
            });

        Sleep(5000); // Sleep for 5 seconds  

        for (auto& thread : threads) {
            thread.join();
        }
    }

    // @brief Test for class WorkerPool.
    void testWorkerPool(QObject* parent) {
        WorkerPool* workerPool = new WorkerPool(parent);
        workerPool->start(2, [&](const SignalingTask& task, Worker* self) {
            qDebug() << self->getId() << "consumes a task: " << task._payload;
            Sleep(1);
        });

        for (int i = 0; i < 100; i++) {
            
            SignalingTask task(QString::number(0), QString::number(i + 1));
            workerPool->submitTask(task);
            qDebug() << "The size of queue is: " << workerPool->getQueueSize();
        }

        workerPool->stop();
        workerPool->submitTask(SignalingTask(QString::number(0), QString::number(10000)));
        Sleep(100);
        return;
    }
};
#endif // __TEST_HPP__

