/* Platform includes */
#define NOMINMAX
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

/* Project includes */
#include "windows_udp_transport.h"

bool WindowsUdpTransport::InitializeSendSocket(const std::string& ipAddress, uint16_t port)  
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    _sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sendSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return false;
    }

    _sendAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ipAddress.c_str(), &_sendAddress.sin_addr);
    _sendAddress.sin_port = htons(port);

    return true;
}

// Initialize the receive socket (used to receive messages)
bool WindowsUdpTransport::InitializeReceiveSocket(uint16_t localPort) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    _receiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_receiveSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create receive socket\n";
        WSACleanup();
        return false;
    }

    // Bind the receive socket to the specified local port
    _receiveAddress.sin_family = AF_INET;
    _receiveAddress.sin_addr.s_addr = INADDR_ANY;
    _receiveAddress.sin_port = htons(localPort);

    if (bind(_receiveSocket, reinterpret_cast<sockaddr*>(&_receiveAddress), sizeof(_receiveAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind receive socket\n";
        closesocket(_receiveSocket);
        WSACleanup();
        return false;
    }

    auto port = ntohs(_receiveAddress.sin_port);
    std::cout << "Port: " << port << "\n";
    return true;
}

void WindowsUdpTransport::DeInitialize() 
{
    if (_sendSocket != INVALID_SOCKET) {
        closesocket(_sendSocket);
        _sendSocket = INVALID_SOCKET;
    }

    if (_receiveSocket != INVALID_SOCKET) {
        closesocket(_receiveSocket);
        _receiveSocket = INVALID_SOCKET;
    }

    WSACleanup();
}

bool WindowsUdpTransport::TransportSendMessage(const std::string& message)
{
    // auto appName = message.substr(0, message.find('.'));
    // std::cout << "Sending a message to: " << appName << "\n";
    // auto app = _clientList.find(appName);
    // if (app == _clientList.end())
    // {
    //     std::cout << appName << " not on the Tranport client list\n";
    // }

    if (_sendSocket == INVALID_SOCKET) {
        std::cerr << "Socket not initialized\n";
        return false;
    }

    int result = sendto(
        _sendSocket,
        message.c_str(),
        message.size(),
        0,
        reinterpret_cast<sockaddr*>(&_sendAddress),
        sizeof(_sendAddress)
    );

    if (result == SOCKET_ERROR) {
        std::cerr << "Failed to send message\n";
        return false;
    }
    std::cout << "Sent message\n";
    return true;
}

bool WindowsUdpTransport::ReceiveMessage(std::string& senderIp, uint16_t& senderPort) 
{
    if (_receiveSocket == INVALID_SOCKET) {
        std::cerr << "Receive socket not initialized\n";
        return false;
    }

    char buffer[1024]; // Adjust buffer size as needed
    sockaddr_in senderAddress;
    int senderAddressSize = sizeof(senderAddress);

    int bytesReceived = recvfrom(
        _receiveSocket,
        buffer,
        sizeof(buffer),
        0,
        reinterpret_cast<sockaddr*>(&senderAddress),
        &senderAddressSize
    );

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Failed to receive message\n";
        return false;
    }
    buffer[bytesReceived] = '\0';
    // auto msgStr = std::string(buffer);
    // std::string appName = msgStr.substr(0, msgStr.find('.'));
    // _clientList[appName] = senderAddress;

    std::cout << senderAddress.sin_port << "\n";
    senderPort = ntohs(senderAddress.sin_port);
    // Convert sender's IP and port to string and uint16_t
    char ipBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &senderAddress.sin_addr, ipBuffer, sizeof(ipBuffer));
    senderIp = ipBuffer;

    MessageInfo messageInfo;
    messageInfo.senderIp = senderIp;
    messageInfo.senderPort = senderPort;
    messageInfo.message = std::string(buffer);
    std::cout << "Message from " << messageInfo.senderIp << ":" << messageInfo.senderPort << ", " << messageInfo.message << "\n";
    _rxMessageQueue.AddMessageToQueue(messageInfo);
    return true;
}

bool WindowsUdpTransport::PollReceiveSocket() 
{
    if (_receiveSocket == INVALID_SOCKET) {
        std::cerr << "Receive socket not initialized\n";
        return false;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(_receiveSocket, &readSet);

    timeval timeout = {0, 0}; // Non-blocking: immediately return if no data is available

    int result = select(0, &readSet, nullptr, nullptr, &timeout);
    if (result > 0 && FD_ISSET(_receiveSocket, &readSet)) {
        return true; // Data is available to read
    }

    return false; // No data available
}


WindowsUdpTransport::~WindowsUdpTransport() 
{
    DeInitialize();
}