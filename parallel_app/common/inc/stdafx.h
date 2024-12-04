#ifndef _STDAFX_H_
#define _STDAFX_H_

/* System includes */
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <queue>
#include <memory>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <regex>
#include <limits>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <chrono>

/* 3rd party includes */
#include <nlohmann/json.hpp>

/* Project includes */
#include "common_config.h"

#ifdef _WIN32
#include "windows_udp_transport.h"
#include "windows_platform_functions.h"
#elif defined(__linux__)
#include "linux_udp_transport.h"
#else
#error "Unsupported platform"
#endif

class InputParser{
    public:
        InputParser (int &argc, char **argv);
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const;
        /// @author iain
        bool cmdOptionExists(const std::string &option) const;
    private:
        std::vector <std::string> tokens;
};

bool IsUserQuit();

#endif //_STDAFX_H_