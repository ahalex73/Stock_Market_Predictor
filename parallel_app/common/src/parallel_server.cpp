#include "parallel_server.h"

ParallelServer::ParallelServer(std::ifstream& config_file)
{   
    nlohmann::json jsonData = nlohmann::json::parse(config_file);
    std::cout << "JSON Data: " << jsonData << "\n";

    _serverSocketInfo.ip = jsonData["server_socket"]["ip"];
    _serverSocketInfo.portNumber = stoi((std::string)jsonData["server_socket"]["port"]);

    std:: cout << "IP: " << _serverSocketInfo.ip << " Port Number: " << _serverSocketInfo.portNumber << "\n";

    _transport = UdpTransportFactory::CreateTransport();
}


ParallelServer::~ParallelServer()
{
    _transport->DeInitialize();
}

bool ParallelServer::InitializeTransport()
{
    if (false == _transport->InitializeReceiveSocket(_serverSocketInfo.portNumber)) 
    {
        std::cerr << "Failed to initialize transport\n";
        return false;
    } 
    else 
    {
        // std::string message = "Hello";
        // if (_transport->TransportSendMessage(message)) {
        //     std::cout << "Message sent successfully\n";
        // } else {
        //     std::cerr << "Failed to send message\n";
        // }
        return true;
    }
}

void ParallelServer::Run()
{
    std::thread messageRxThread(&ParallelServer::MessageListenerThread, this);
    std::thread messageTxThread(&ParallelServer::MessageSendThread, this);

    messageRxThread.join();
    messageTxThread.join();
}

void ParallelServer::MessageListenerThread()
{
    while (false == _isQuit)
    {
        if (true == _transport->PollReceiveSocket())
        {
            _transport->ReceiveMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_LOCK_SLEEP));
    }
}

void ParallelServer::MessageSendThread()
{
    while (false == _isQuit)
    {
        if (false == _txMessageQueue.IsQueueEmpty())
        {
            _transport->TransportSendMessage(_txMessageQueue.GetMessageFromQueue());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_LOCK_SLEEP));
    }
}