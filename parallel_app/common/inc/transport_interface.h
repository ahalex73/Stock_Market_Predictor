#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

/* System includes */

/* Project includes */
#include "stdafx.h"
#include "thread_safe_queue.h"

struct SocketInfo
{
    std::string ip;
    uint16_t portNumber; 

};

struct MessageInfo
{
    std::string senderIp;
    uint16_t senderPort;
    std::string message;
};

struct MessageContents
{
    std::string appName;
    MessageTypes messageId;
    std::string messageContent;
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
    virtual bool ReceiveMessage(std::string& senderIp, uint16_t& senderPort) = 0;

    // Poll the receive socket
    virtual bool PollReceiveSocket() = 0;

public:    
    ThreadSafeQueue<MessageInfo> _rxMessageQueue;

};
#endif _TRANSPORT_H_