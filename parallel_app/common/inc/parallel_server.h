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
    std::string _stockList;
};

class ParallelServer
{
    public:
        ParallelServer(nlohmann::json jsonData);
        ~ParallelServer();
        bool InitializeTransport();
        void Run();
        void MessageListenerThread();
        void MessageSendThread();
        void ProcessReceivedMessage();
        void AddMessageToQueue(MessageTypes msgId, const std::string destApp, const std::string msg);
        void TestRunnerThread();
        bool RunDownloadData();
        bool RunTrainModel();
        bool RunMakePredictions();

    private:
        bool _isQuit = false;
        std::string _appName;
        SocketInfo _serverSocketInfo;
        std::shared_ptr<TransportInterface> _transport;
        ThreadSafeQueue<std::pair<std::string, std::string>> _txMessageQueue;
        std::unordered_map<std::string, ClientInfo> _clientList;
        std::chrono::high_resolution_clock::time_point _startTime;
        std::chrono::high_resolution_clock::time_point _endTime;
};
#endif // _PARALLEL_SERVER_H_