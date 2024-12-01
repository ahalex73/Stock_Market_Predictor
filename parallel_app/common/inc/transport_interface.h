#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

/* System includes */

/* Project includes */
#include "stdafx.h"
#include "thread_safe_queue.h"

struct SocketInfo
{
    std::string ip;
    int portNumber; 

};

class TransportInterface
{
public:
    virtual ~TransportInterface() = default;

    // Initialize the UDP transport layer.
    virtual bool InitializeSendSocket(const std::string& ipAddress, uint16_t port) = 0;
    virtual bool InitializeReceiveSocket(uint16_t port) = 0;

    // Deinitialize the UDP transport layer.
    virtual void DeInitialize() = 0;

    // Send a message through the UDP transport layer.
    virtual bool TransportSendMessage(const std::string& message) = 0;
    
    // Receive a message 
    virtual bool ReceiveMessage() = 0;

    // Poll the receive socket
    virtual bool PollReceiveSocket() = 0;

public:    
    ThreadSafeQueue _rxMessageQueue;

};
#endif _TRANSPORT_H_