#ifndef _THREAD_SAFE_QUEUE_H_
#define _THREAD_SAFE_QUEUE_H_

#include "stdafx.h"

template <typename QueueType>
class ThreadSafeQueue
{
    public:
        // Add a message to the queue in a thread-safe manner.
        void AddMessageToQueue(const QueueType& message)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            messageQueue.push(message);
        }

        // Retrieve and remove a message from the queue in a thread-safe manner.
        QueueType GetMessageFromQueue()
        {
            QueueType retVal{};
            std::unique_lock<std::mutex> lock(queueMutex);
            if (!messageQueue.empty()) 
            {
                retVal = messageQueue.front();
                messageQueue.pop();
            }
            else
            {
                std::cout << "Error message queue is empty!\n";
            }
            return retVal;
        }

        // Check if queue is empty
        bool IsQueueEmpty()
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            return messageQueue.empty();
        }
    
    private:
        // Message queue for thread-safe communication.
        std::queue<QueueType> messageQueue;

        // Mutex for synchronizing access to the message queue.
        std::mutex queueMutex;

};

#endif // _THREAD_SAFE_QUEUE_H_