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
struct ClientActionStatus
{
    int _numDownloadMsgsRx = 0;
    int _numTrainMsgsRx = 0;
    int _numPredictMsgsRx = 0;
    bool _downloadsComplete = false;
    bool _trainingComplete = false;
    bool _predictionsComplete = false;
    std::chrono::high_resolution_clock::time_point _downloadStartTime;
    std::chrono::high_resolution_clock::time_point _downloadEndTime;
    std::chrono::milliseconds _downloadElapsedTime;
    std::chrono::high_resolution_clock::time_point _trainingStartTime;
    std::chrono::high_resolution_clock::time_point _trainingEndTime;
    std::chrono::milliseconds _trainingElapsedTime;
    std::chrono::high_resolution_clock::time_point _predictionsStartTime;
    std::chrono::high_resolution_clock::time_point _predictionsEndTime;
    std::chrono::milliseconds _predictionsElapsedTime;
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
        void HandleDownloadMessage();
        void HandleTrainMessage();
        void HandlePredictMessage();

    private:
        bool _isQuit = false;
        int _numClients = 0;
        std::string _appName;
        SocketInfo _serverSocketInfo;
        std::shared_ptr<TransportInterface> _transport;
        ThreadSafeQueue<std::pair<std::string, std::string>> _txMessageQueue;
        std::unordered_map<std::string, ClientInfo> _clientList;
        ClientActionStatus _clientActionStatus;
};
#endif // _PARALLEL_SERVER_H_