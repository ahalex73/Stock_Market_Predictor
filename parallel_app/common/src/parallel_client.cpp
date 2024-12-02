#include "parallel_client.h"
#include "Python.h"

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
        std::string command = "python " +  _pythonScriptDir + "\\" + _downloadDataScript.first + ".py --stock " + t;
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }


    // std::string venvPath = "C:\\Users\\AJCic\\Documents\\CS-5300\\Stock_Market_Predictor\\myenv";  // Replace with your virtual environment path
    // std::string pythonExePath = venvPath + "\\Scripts\\python.exe";  // Path to the Python executable
    // std::string pythonLibPath = venvPath + "\\Lib";  // Path to the Python Lib directory

    // // Set the environment variables for Python
    // _putenv_s("PYTHONHOME", venvPath.c_str());  // Set PYTHONHOME to the virtual environment
    // _putenv_s("PYTHONEXECUTABLE", pythonExePath.c_str());  // Set Python executable path
    // _putenv_s("PYTHONPATH", pythonLibPath.c_str());  // Set PYTHONPATH to the Lib directory

    // // Initialize Python interpreter
    // Py_Initialize();

    // // Clear sys.path to avoid any global Python interference
    // PyRun_SimpleString("import sys\nsys.path.clear()");

    // // Add virtual environment paths to sys.path
    // std::string commandAppend = "import sys\nsys.path.append(r'" + pythonLibPath + "')\n";
    // commandAppend += "sys.path.append(r'" + venvPath + "\\Scripts')";  // Ensure Scripts is also added
    // PyRun_SimpleString(commandAppend.c_str());

    // std::stringstream ss(msg);
    // std::string ticker;
    // std::vector<std::string> tickerList;

    // while (std::getline(ss, ticker, ',')) {
    //     tickerList.push_back(ticker);
    // }

    // PyRun_SimpleString("import sys\nprint(sys.executable)");

    // // Add the directory containing your Python script to the Python path
    // std::string commandScript = "import sys\nsys.path.append('" + _pythonScriptDir + "')";
    
    // PyRun_SimpleString(commandScript.c_str());

    // // Import the Python script (example: script.py)
    // PyObject* pName = PyUnicode_DecodeFSDefault(_downloadDataScript.first.c_str());  // Name of the Python script without extension
    // PyObject* pModule = PyImport_Import(pName);
    // Py_XDECREF(pName);

    // if (pModule != nullptr) {
    //     // Get the function from the module
    //     PyObject* pFunc = PyObject_GetAttrString(pModule, _downloadDataScript.second.c_str());

    //         // Print the parsed tickers
    //     for (const auto& t : tickerList) {
    //         std::cout << "Downloading stock " << t << "\n";
        
    //         if (pFunc && PyCallable_Check(pFunc)) {
    //             // Call the function with no arguments (if it takes arguments, you can use Py_BuildValue)  
    //             PyObject* pArg = PyUnicode_FromString(t.c_str());

    //             // Call the function with arguments
    //             PyObject* pValue = PyObject_CallObject(pFunc, pArg);

    //             if (pValue != nullptr) {
    //                 // If the function returns a value, print it
    //                 std::cout << "Returned value: " << PyLong_AsLong(pValue) << std::endl; // assuming it's an integer
    //                 Py_XDECREF(pValue);
    //             } else {
    //                 PyErr_Print();
    //             }
    //         } else {
    //             if (PyErr_Occurred()) {
    //                 PyErr_Print();
    //             }
    //         }
    //     }
    //     Py_XDECREF(pFunc);
    //     Py_XDECREF(pModule);
    // } else {
    //     PyErr_Print();
    // }

    // // Finalize Python interpreter
    // Py_Finalize();
    // return true;
}

bool ParallelClient::RunTrainModel(std::string msg)
{
    for (const auto& t : _stockList) 
    {
        std::cout << "Training Model on stock " << t << "\n";
        std::string command = "python " +  _pythonScriptDir + "\\" + _trainModelScript.first + ".py --stock " + t;
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }
    return true;
}

bool ParallelClient::RunMakePredictions(std::string msg)
{
    for (const auto& t : _stockList) 
    {
        std::cout << "Predicting Model on stock " << t << "\n";
        std::string command = "python " +  _pythonScriptDir + "\\" + _predictModelScript.first + ".py --saved_model_path " + _pythonScriptDir + "\\" + _trainedModelPath + "/" + t +"_model.keras";
        std::cout << "Command: " << command << "\n";
        system(command.c_str());
    }
    return true;
}
