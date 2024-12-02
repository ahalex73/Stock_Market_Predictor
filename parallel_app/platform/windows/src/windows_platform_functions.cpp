#include "windows_platform_functions.h"

bool Windows_IsUserQuit()
{
    bool retVal = false;        
    if (_kbhit()) {
        // Read the pressed key
        char ch = _getch();
        if (ch == 'q' || ch == 'Q') {
            retVal = true;
            std::cout << "\nQuit key pressed. Exiting..." << std::endl;
        }
    }
    return retVal;
}