/* Platform includes */
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

/* Project includes */
#include "windows_udp_transport.h"

bool WindowsUdpTransport::Initialize(const std::string& ipAddress, uint16_t port)  
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return false;
    }

    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(port);

    return true;
}

void WindowsUdpTransport::DeInitialize() 
{
    if (udpSocket != INVALID_SOCKET) {
        closesocket(udpSocket);
        WSACleanup();
    }
}

bool WindowsUdpTransport::TransportSendMessage(const std::vector<uint8_t>& message)
{
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Socket not initialized\n";
        return false;
    }

    int result = sendto(
        udpSocket,
        reinterpret_cast<const char*>(message.data()),
        message.size(),
        0,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR) {
        std::cerr << "Failed to send message\n";
        return false;
    }

    return true;
}

WindowsUdpTransport::~WindowsUdpTransport() 
{
    DeInitialize();
}