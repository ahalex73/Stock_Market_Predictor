#ifndef _PARALLEL_SERVER_H_
#define _PARALLEL_SERVER_H_

#include "stdafx.h"
#include "transport_interface.h"
#include "transport_factory.h"


struct ClientInfo
{
    std::string _appName;
    SocketInfo _clientSocketInfo;
    std::shared_ptr<TransportInterface> _transport;
};

class ParallelServer
{
    public:
        ParallelServer(std::ifstream& config_file);
        ~ParallelServer();
        bool InitializeTransport();
        void Run();
        void MessageListenerThread();
        void MessageSendThread();

    private:
        bool _isQuit = false;
        SocketInfo _serverSocketInfo;
        std::shared_ptr<TransportInterface> _transport;
        ThreadSafeQueue _txMessageQueue;
        std::unordered_map<std::string, ClientInfo> _clientList;
};
#endif // _PARALLEL_SERVER_H_