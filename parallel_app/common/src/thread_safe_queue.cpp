#include "thread_safe_queue.h"

// Add a message to the queue in a thread-safe manner.
void ThreadSafeQueue::AddMessageToQueue(const std::string& message) 
{
    std::unique_lock<std::mutex> lock(queueMutex);
    messageQueue.push(message);
}

// Retrieve and remove a message from the queue in a thread-safe manner.
std::string ThreadSafeQueue::GetMessageFromQueue() 
{
    std::string retVal;
    std::unique_lock<std::mutex> lock(queueMutex);
    if (!messageQueue.empty()) 
    {
        std::string message = messageQueue.front();
        messageQueue.pop();
    }
    return retVal;
}

bool ThreadSafeQueue::IsQueueEmpty()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    return messageQueue.empty();
}