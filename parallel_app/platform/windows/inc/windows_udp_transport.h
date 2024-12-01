#ifndef _WINDOWS_UDP_TRANSPORT_H_
#define _WINDOWS_UDP_TRANSPORT_H_

/* System includes */
#include <iostream> 
#include <vector>

/* Platform includes */
#include <winsock2.h>
#include <ws2tcpip.h>

/* Project includes */
#include "transport_interface.h"

class WindowsUdpTransport : public TransportInterface
{
    public:
        WindowsUdpTransport() : _sendSocket(INVALID_SOCKET), _receiveSocket(INVALID_SOCKET) {}

        bool InitializeSendSocket(const std::string& ipAddress, uint16_t port) override;
        bool InitializeReceiveSocket(uint16_t port) override;

        void DeInitialize() override;

        bool TransportSendMessage(const std::string& message) override;
        
        bool ReceiveMessage() override;

        bool PollReceiveSocket() override;

        ~WindowsUdpTransport() override;
    private:    
        SOCKET _sendSocket;        // Socket for sending data
        SOCKET _receiveSocket;     // Socket for receiving data
        sockaddr_in _sendAddress;  // Address to send messages
        sockaddr_in _receiveAddress; // Address to bind for receiving messages
};

#endif // _WINDOWS_UDP_TRANSPORT_H_