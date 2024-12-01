#ifndef _THREAD_SAFE_QUEUE_H_
#define _THREAD_SAFE_QUEUE_H_

#include "stdafx.h"

class ThreadSafeQueue
{
    public:
        // Add a message to the queue in a thread-safe manner.
        void AddMessageToQueue(const std::string& message);

        // Retrieve and remove a message from the queue in a thread-safe manner.
        std::string GetMessageFromQueue();

        // Check if queue is empty
        bool IsQueueEmpty();
    
    private:
        // Message queue for thread-safe communication.
        std::queue<std::string> messageQueue;

        // Mutex for synchronizing access to the message queue.
        std::mutex queueMutex;

};

#endif // _THREAD_SAFE_QUEUE_H_