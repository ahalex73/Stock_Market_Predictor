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
private:
    SOCKET udpSocket;
    sockaddr_in serverAddress;

public:
    WindowsUdpTransport() : udpSocket(INVALID_SOCKET) {}

    bool Initialize(const std::string& ipAddress, uint16_t port) override;

    void DeInitialize() override;

    bool TransportSendMessage(const std::vector<uint8_t>& message) override;

    ~WindowsUdpTransport() override;
};

#endif // _WINDOWS_UDP_TRANSPORT_H_