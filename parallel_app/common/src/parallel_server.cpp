#include "parallel_server.h"

static void PrintMenu() 
{
    const std::string title = "STOCK PREDICTOR SERVER";
    const int boxWidth = 50;  // Width of the box
    const std::string border(boxWidth, '*');
    
    std::cout << border << '\n';
    std::cout << '*' << std::setw((boxWidth - title.length()) / 2 + title.length()) 
              << std::right << title 
              << std::setw(((boxWidth - title.length()) / 2)-1) << '*' << '\n';
    std::cout << border << '\n';

    std::cout << "*  1. Download Data                              *\n";
    std::cout << "*  2. Train Model                                *\n";
    std::cout << "*  3. Make Predictions                           *\n";
    std::cout << "*  q. Quit                                       *\n";
    std::cout << border << '\n';
}

char GetOption() 
{
    char option = '\0';

    while (true) {
        std::cout << "Enter your choice (1-3): ";
        std::cin >> option;

        if ((option >= '1' && option <= '3') || option == 'q') {
            return option; // Valid input, return the option
        }

        // Invalid input: Clear error state and discard input
        std::cin.clear(); 
        std::cin.ignore(10000, '\n'); // Use a large number to discard input
        std::cout << "Invalid choice. Please enter a number between 1 and 3, or q to quit.\n";
    }
    return option;
}

ParallelServer::ParallelServer(nlohmann::json jsonData)
{   
    std::cout << "JSON Data: " << jsonData << "\n";

    _serverSocketInfo.ip = jsonData["server_socket"]["ip"];
    _serverSocketInfo.portNumber = stoi((std::string)jsonData["server_socket"]["port"]);

    std:: cout << "IP: " << _serverSocketInfo.ip << " Port Number: " << _serverSocketInfo.portNumber << "\n";

    _appName = "Predictor_Server";
    // Access the client_list
    for (const auto& client : jsonData["client_list"]) 
    {
        ClientInfo newClient;
        newClient._appName = client["app_name"];
        newClient._clientSocketInfo.ip = client["ip"];
        std::string port = client["port_number"];
        newClient._clientSocketInfo.portNumber = stoi(port);
        newClient._transport = UdpTransportFactory::CreateTransport();
        newClient._stockList = client["stock_list"];

        std::cout << "App Name: " << newClient._appName << std::endl;
        std::cout << "Client IP: " << newClient._clientSocketInfo.ip << std::endl;
        std::cout << "Client Port: " << newClient._clientSocketInfo.portNumber << std::endl;    
        std::cout << "Stock List: " << newClient._stockList << std::endl;
        
        newClient._transport->InitializeSendSocket(newClient._clientSocketInfo.ip, newClient._clientSocketInfo.portNumber);


        _clientList[newClient._appName] = newClient;
        _numClients++;
    }
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
    std::thread testRunnerThread(&ParallelServer::TestRunnerThread, this);

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
    testRunnerThread.join();
}

void ParallelServer::MessageListenerThread()
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

void ParallelServer::MessageSendThread()
{
    while (false == _isQuit)
    {
        if (false == _txMessageQueue.IsQueueEmpty())
        {
            auto msg = _txMessageQueue.GetMessageFromQueue();
            auto appName = msg.first;
            _clientList[appName]._transport->TransportSendMessage(msg.second);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_LOCK_SLEEP));
    }
}

void ParallelServer::TestRunnerThread()
{
    int testOption = 0;
    while (false == _isQuit)
    {
        PrintMenu();
        testOption = GetOption();

        switch (testOption)
        {
            case '1':
                if (false == RunDownloadData())
                {
                    std::cout << "Failed to Download stock data\n";
                }
                break;
            
            case '2':
                if (false == RunTrainModel())
                {
                    std::cout << "Failed to train model\n";
                }
                break;
            
            case '3':
                if (false == RunMakePredictions())
                {
                    std::cout << "Failed to make predictions\n";
                }
                break;

            case 'q':
                std::cout << "\n Q pressed. Quitting test\n";
                _isQuit = true;
                break;
            
            default:
                std::cout << "Invalid Option\n";
                break;
        }
    }
}

void ParallelServer::ProcessReceivedMessage()
{
    auto rxMessageInfo = _transport->_rxMessageQueue.GetMessageFromQueue();
    std::string rxMessage = rxMessageInfo.message;
    std::cout << "Message Received: " << rxMessage << "\n";
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


    if (_clientList.find(messageContent.appName) == _clientList.end()) 
    {
        std::cout << "App not on list\n";
        // AddClientToList(messageContent.appName, rxMessageInfo.senderIp, rxMessageInfo.senderPort);
        // std::cout << "Added client to list\n";
    } 
    else 
    {
        // AddMessageToQueue(MessageTypes::STOCK_LIST, messageContent.appName, "AAL,SYK");
        // App already exists
        std::cout << "App \"" << messageContent.appName << "\" already exists in the map.\n";
    }

    switch (messageContent.messageId)
    {
        case MessageTypes::SETUP:
            break;

        case MessageTypes::STOCK_DATA_READY:
            HandleDownloadMessage();
            break;

        case MessageTypes::TRAINING_COMPLETE:
            HandleTrainMessage();
            break;

        case MessageTypes::PREDICTIONS_COMPLETE:
            HandlePredictMessage();
            break;

        default:
            std::cout << "Unknown message type received\n";
            break;
    }
}

void ParallelServer::AddMessageToQueue(MessageTypes msgId, const std::string destApp, const std::string msg)
{
    std::string txMsg(_appName + "." + MessageIdToString(msgId) + "." + msg);
    std::cout << "Message to send: " << txMsg << "\n";
    _txMessageQueue.AddMessageToQueue({destApp, txMsg});
}

bool ParallelServer::RunDownloadData()
{
    bool retVal = false;
    _clientActionStatus._numDownloadMsgsRx = 0;
    _clientActionStatus._downloadsComplete = false;
    _clientActionStatus._downloadStartTime = std::chrono::high_resolution_clock::now();
    for (const auto& app : _clientList)
    {
        std::cout << "Adding Stock List message to queue for " << app.first << "\n";
        AddMessageToQueue(MessageTypes::STOCK_LIST, app.first, app.second._stockList); 
        retVal = true;
    }
    return retVal;
}

bool ParallelServer::RunTrainModel()
{
    bool retVal = false;
    _clientActionStatus._numTrainMsgsRx = 0;
    _clientActionStatus._trainingComplete = false;
    _clientActionStatus._trainingStartTime = std::chrono::high_resolution_clock::now();
    for (const auto& app : _clientList)
    {
        std::cout << "Adding Stock List message to queue for " << app.first << "\n";
        AddMessageToQueue(MessageTypes::BEGIN_TRAINING, app.first, "Begin_Training"); 
        retVal = true;
    }
    return retVal;
}

bool ParallelServer::RunMakePredictions()
{
    bool retVal = false;
    _clientActionStatus._numPredictMsgsRx = 0;
    _clientActionStatus._predictionsComplete = false;
    _clientActionStatus._predictionsStartTime = std::chrono::high_resolution_clock::now();
    for (const auto& app : _clientList)
    {
        std::cout << "Adding Stock List message to queue for " << app.first << "\n";
        AddMessageToQueue(MessageTypes::MAKE_PREDICTIONS, app.first, "Make_Predictions"); 
        retVal = true;
    }
    return retVal;
}

void ParallelServer::HandleDownloadMessage()
{
    _clientActionStatus._numDownloadMsgsRx++;
    if (_numClients == _clientActionStatus._numDownloadMsgsRx)
    {
        _clientActionStatus._downloadsComplete = true;
        _clientActionStatus._downloadEndTime = std::chrono::high_resolution_clock::now();
        _clientActionStatus._downloadElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(_clientActionStatus._downloadEndTime - _clientActionStatus._downloadStartTime);
        std::cout << "Download Elapsed time: " << _clientActionStatus._downloadElapsedTime.count() << " milliseconds\n";
        
    }
}

void ParallelServer::HandleTrainMessage()
{
    _clientActionStatus._numTrainMsgsRx++;
    if (_numClients == _clientActionStatus._numTrainMsgsRx)
    {
        _clientActionStatus._trainingComplete = true;
        _clientActionStatus._trainingEndTime = std::chrono::high_resolution_clock::now();
        _clientActionStatus._trainingElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(_clientActionStatus._trainingEndTime - _clientActionStatus._trainingStartTime);
        std::cout << "Training Elapsed time: " << _clientActionStatus._trainingElapsedTime.count() << " milliseconds\n";
    }
}

void ParallelServer::HandlePredictMessage()
{
    _clientActionStatus._numPredictMsgsRx++;
    if (_numClients == _clientActionStatus._numPredictMsgsRx)
    {
        _clientActionStatus._predictionsComplete = true;
        _clientActionStatus._predictionsEndTime = std::chrono::high_resolution_clock::now();
        _clientActionStatus._predictionsElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(_clientActionStatus._predictionsEndTime - _clientActionStatus._predictionsStartTime);
        std::cout << "Predictions Elapsed time: " << _clientActionStatus._predictionsElapsedTime.count() << " milliseconds\n";
    }
}
