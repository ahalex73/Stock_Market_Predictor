#include "stdafx.h"
#include "parallel_server.h"
#include "parallel_client.h"

InputParser::InputParser (int &argc, char **argv){
    for (int i=1; i < argc; ++i)
        this->tokens.push_back(std::string(argv[i]));
}

/// @author iain
const std::string& InputParser::getCmdOption(const std::string &option) const{
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
        return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
}

/// @author iain
bool InputParser::cmdOptionExists(const std::string &option) const{
    return std::find(this->tokens.begin(), this->tokens.end(), option)
            != this->tokens.end();
}

int main(int argc, char** argv)
{
    InputParser input(argc, argv);    
    const std::string &config_file_path = input.getCmdOption("-f");
    if (true == config_file_path.empty())
    {
        std::cout << "Parallel app needs a json config file passed. Example usage\n";
        std::cout << "./parallel_app -f config_file.json\n";
        return 1;
    }
    std::ifstream config_file(config_file_path);
    nlohmann::json jsonData = nlohmann::json::parse(config_file);
    bool isServer = jsonData["is_server"];

    if (true == isServer)
    {
        ParallelServer app(jsonData);
        if(false == app.InitializeTransport())
        {
            std::cout << "Failed to initialize transport\n";
        }
        else
        {
            app.Run();
        }
    }
    else
    {
        ParallelClient app(jsonData);
        if(false == app.InitializeTransport())
        {
            std::cout << "Failed to initialize transport\n";
        }
        else
        {
            app.Run();
        }
    }
}

bool IsUserQuit()
{
    #ifdef _WIN32
        return Windows_IsUserQuit();
    #elif defined(__linux__)
        return Linux_IsUserQuit();
    #else
        return false;
    #endif
}