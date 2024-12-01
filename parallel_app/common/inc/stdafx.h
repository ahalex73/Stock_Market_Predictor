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

/* 3rd party includes */
#include <nlohmann/json.hpp>

/* Project includes */
#include "common_config.h"

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
#endif //_STDAFX_H_