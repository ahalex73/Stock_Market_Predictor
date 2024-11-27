#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

/* System includes */

/* Project includes */
#include "stdafx.h"

class TransportInterface
{
public:
    virtual ~TransportInterface() = default;

    // Initialize the UDP transport layer.
    virtual bool Initialize(const std::string& ipAddress, uint16_t port) = 0;

    // Deinitialize the UDP transport layer.
    virtual void DeInitialize() = 0;

    // Send a message through the UDP transport layer.
    virtual bool TransportSendMessage(const std::vector<uint8_t>& message) = 0;

protected:    
    // Message queue for thread-safe communication.
    std::queue<std::string> messageQueue;

    // Mutex for synchronizing access to the message queue.
    std::mutex queueMutex;

    // Add a message to the queue in a thread-safe manner.
    void AddMessageToQueue(const std::string& message) {
        std::unique_lock<std::mutex> lock(queueMutex);
        messageQueue.push(message);
    }

    // Retrieve and remove a message from the queue in a thread-safe manner.
    bool GetMessageFromQueue(std::string& message) {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!messageQueue.empty()) {
            message = messageQueue.front();
            messageQueue.pop();
            return true;
        }
        return false;
    }

};
#endif _TRANSPORT_H_