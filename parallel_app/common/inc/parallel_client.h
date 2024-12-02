#ifndef _PARALLEL_CLIENT_H
#define _PARALLEL_CLIENT_H

#include "stdafx.h"
#include "transport_interface.h"
#include "transport_factory.h"

class ParallelClient
{
    public:
        ParallelClient(nlohmann::json jsonData);
        ~ParallelClient();
        bool InitializeTransport();
        void Run();
        void MessageListenerThread();
        void MessageSendThread(); 
        void ProcessReceivedMessage();
        void AddMessageToQueue(MessageTypes msgId, const std::string msg);
        bool RunDownloadData(std::string msg);
        bool RunTrainModel(std::string msg);
        bool RunMakePredictions(std::string msg);

    private:
        bool _isQuit = false;
        std::string _appName;
        std::string _pythonScriptDir;
        std::pair<std::string, std::string> _downloadDataScript;
        std::pair<std::string, std::string> _trainModelScript;
        std::pair<std::string, std::string> _predictModelScript;
        SocketInfo _serverSocketInfo;
        SocketInfo _clientSocketInfo;
        std::shared_ptr<TransportInterface> _transport;
        ThreadSafeQueue<std::string> _txMessageQueue;
        std::vector<std::string> _stockList;
        std::string _trainedModelPath;
};

#endif // _PARALLEL_CLIENT_H