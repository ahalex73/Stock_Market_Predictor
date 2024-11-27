#include "stdafx.h"
#include "transport_factory.h"

int main(int argc, char** argv)
{
    auto transport = UdpTransportFactory::CreateTransport();

    if (transport->Initialize("127.0.0.1", 8080)) {
        std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o'};
        if (transport->TransportSendMessage(message)) {
            std::cout << "Message sent successfully\n";
        } else {
            std::cerr << "Failed to send message\n";
        }
        transport->DeInitialize();
    } else {
        std::cerr << "Failed to initialize transport\n";
    }
    printf("Hello World!\n");
}