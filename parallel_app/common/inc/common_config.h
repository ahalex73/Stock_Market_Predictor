#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

#define SPIN_LOCK_SLEEP 10

enum class MessageTypes
{
    SETUP,
    STOCK_LIST,
    STOCK_DATA_READY,
    BEGIN_TRAINING,
    TRAINING_COMPLETE,
    MAKE_PREDICTIONS,
    PREDICTIONS_COMPLETE,
    UNKNOWN,
};
inline MessageTypes StringToMessageId(const std::string& idStr) {
    int idInt = std::atoi(idStr.c_str());

    switch (idInt) {
        case 0: return MessageTypes::SETUP;
        case 1: return MessageTypes::STOCK_LIST;
        case 2: return MessageTypes::STOCK_DATA_READY;
        case 3: return MessageTypes::BEGIN_TRAINING;
        case 4: return MessageTypes::TRAINING_COMPLETE;
        case 5: return MessageTypes::MAKE_PREDICTIONS;
        case 6: return MessageTypes::PREDICTIONS_COMPLETE;
        default: return MessageTypes::UNKNOWN;
    }
}
inline std::string MessageIdToString(MessageTypes msgId) 
{
    switch (msgId) 
    {
        case MessageTypes::SETUP :                  return "0";
        case MessageTypes::STOCK_LIST :             return "1";
        case MessageTypes::STOCK_DATA_READY :       return "2";
        case MessageTypes::BEGIN_TRAINING :         return "3";
        case MessageTypes::TRAINING_COMPLETE :      return "4";
        case MessageTypes::MAKE_PREDICTIONS :       return "5";
        case MessageTypes::PREDICTIONS_COMPLETE :   return "6";
        default:                                    return "7";
    }
}
#endif // _COMMON_CONFIG_H_