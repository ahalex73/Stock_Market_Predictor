#include "parallel_client.h"

ParallelClient::ParallelClient(nlohmann::json jsonData)
{
    std::cout << "JSON Data: " << jsonData << "\n";

    _serverSocketInfo.ip = jsonData["server_socket"]["ip"];
    _serverSocketInfo.portNumber = stoi((std::string)jsonData["server_socket"]["port"]);

    std::cout << "Server IP: " << _serverSocketInfo.ip << " Port Number: " << _serverSocketInfo.portNumber << "\n";
    
    _clientSocketInfo.ip = jsonData["client_socket"]["ip"];
    _clientSocketInfo.portNumber = stoi((std::string)jsonData["client_socket"]["port"]);

    std::cout << "Client IP: " << _clientSocketInfo.ip << " Port Number: " << _clientSocketInfo.portNumber << "\n";

    _appName = jsonData["app_name"];
    std::cout << "App Name: " << _appName << "\n";

    _pythonScriptDir = jsonData["python_model_script_dir"];

    _downloadDataScript = {jsonData["download_data"]["script_name"], jsonData["download_data"]["function_name"]};
    _trainModelScript = {jsonData["train_model"]["script_name"], jsonData["train_model"]["function_name"]};
    _predictModelScript = {jsonData["predict_model"]["script_name"], jsonData["predict_model"]["function_name"]};
    _trainedModelPath = jsonData["predict_model"]["trained_model_path"];
    _venvPath = jsonData["python_venv"];
    _transport = UdpTransportFactory::CreateTransport();
}

ParallelClient::~ParallelClient()
{
    _transport->DeInitialize();
}

bool ParallelClient::InitializeTransport()
{
    if (false == _transport->InitializeReceiveSocket(_clientSocketInfo.portNumber)) 
    {
        std::cerr << "Failed to initialize receive socket\n";
        return false;
    } 
    else if (false == _transport->InitializeSendSocket(_serverSocketInfo.ip, _serverSocketInfo.portNumber))
    {
        std::cerr << "Failed to initialize the send socket\n";
        return false;
    }
    else 
    {
        return true;
    }
}

void ParallelClient::Run()
{
    std::thread messageRxThread(&ParallelClient::MessageListenerThread, this);
    std::thread messageTxThread(&ParallelClient::MessageSendThread, this);

    AddMessageToQueue(MessageTypes::SETUP, "setup");
    while (false == _isQuit)
    {
        if (false == _transport->_rxMessageQueue.IsQueueEmpty())
        {
            ProcessReceivedMessage();
        }
        _isQuit = IsUserQuit();
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_LOCK_SLEEP));
    }

    messageRxThread.join();
    messageTxThread.join();
}

void ParallelClient::MessageListenerThread()
{
    std::string senderIp;
    uint16_t senderPort;
    while (false == _isQuit)
    {
        if (true == _transport->PollReceiveSocket())
        {
            if(false == _transport->ReceiveMessage(senderIp, senderPort))
            {
                std::cout << "Failed to receive message on socket\n";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_LOCK_SLEEP));
    }
}

void ParallelClient::MessageSendThread()
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

void ParallelClient::ProcessReceivedMessage()
{
    auto rxMessageInfo = _transport->_rxMessageQueue.GetMessageFromQueue();
    std::string rxMessage = rxMessageInfo.message;
    MessageContents messageContent;

    std::regex pattern(R"(([^.]+)\.(\d+)\.(.+))");
    std::smatch match;

    if (true == std::regex_match(rxMessage, match, pattern)) {
        // Extract APP_NAME, MESSAGE_ID, and MESSAGE_CONTENT
        messageContent.appName = match[1];
        messageContent.messageId = StringToMessageId(match[2]);
        messageContent.messageContent = match[3];

        std::cout << "AppName: " << messageContent.appName << std::endl;
        std::cout << "Message ID: " << match[2] << std::endl;
        std::cout << "Message Content: " << messageContent.messageContent << std::endl;
    } else {
        std::cout << "Input does not match the expected format." << std::endl;
        return;
    }

    switch (messageContent.messageId)
    {
        case MessageTypes::STOCK_LIST:
            if (false == RunDownloadData(messageContent.messageContent))
            {
                std::cout << "Failed to download stock data\n";
            }
            break;

        case MessageTypes::BEGIN_TRAINING:
            if (false == RunTrainModel(messageContent.messageContent))
            {
                std::cout << "Failed to train model\n";
            }
            break;

        case MessageTypes::MAKE_PREDICTIONS:
            if (false == RunMakePredictions(messageContent.messageContent))
            {
                std::cout << "Failed to make predictions\n";
            }
            break;

        default:
            std::cout << "Unknown message type received\n";
            break;
    }
}

void ParallelClient::AddMessageToQueue(MessageTypes msgId, const std::string msg)
{
    std::string txMsg(_appName + "." + MessageIdToString(msgId) + "." + msg);
    std::cout << "Message to send: " << txMsg << "\n";
    _txMessageQueue.AddMessageToQueue(txMsg);
}

bool ParallelClient::RunDownloadData(std::string msg)
{
    std::stringstream ss(msg);
    std::string ticker;

    while (std::getline(ss, ticker, ',')) 
    {
        _stockList.push_back(ticker);
    }

    for (const auto& t : _stockList) 
    {
        std::cout << "Downloading stock " << t << "\n";
        std::string command = _venvPath + " " +  _pythonScriptDir + "\\" + _downloadDataScript.first + ".py --stock " + t;
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }
    
    AddMessageToQueue(MessageTypes::STOCK_DATA_READY, "ready");
    return true;
}

bool ParallelClient::RunTrainModel(std::string msg)
{
    for (const auto& t : _stockList) 
    {
        std::cout << "Training Model on stock " << t << "\n";
        std::string command = _venvPath + " " +  _pythonScriptDir + "\\" + _trainModelScript.first + ".py --stock " + t;
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }
    AddMessageToQueue(MessageTypes::TRAINING_COMPLETE, "training_done");
    return true;
}

bool ParallelClient::RunMakePredictions(std::string msg)
{
    for (const auto& t : _stockList) 
    {
        std::cout << "Predicting Model on stock " << t << "\n";
        std::string command = _venvPath + " " +  _pythonScriptDir + "\\" + _predictModelScript.first + ".py --saved_model_path " + _pythonScriptDir + "\\" + _trainedModelPath + "/" + t +"_model.keras";
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }
    AddMessageToQueue(MessageTypes::PREDICTIONS_COMPLETE, "predictions_done");
    return true;
}
