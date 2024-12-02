
Parallel App Message types
    
    Setup Message:
        Message ID: 0
        Purpose:
            Sent from a client to a server to identify the client as online and ready to receive commands
        Message Content:
            TBD, probably nothing. 

    Stock List Message: 
        Message ID: 1
        Purpose:
            Sent from a server to a client to inform the client of the list of stocks it is expected to use.
            When receiving this message the client must download the latest stock data for each of the stocks on the list. 
        Message Content:
            List of comma deliniated stock tokens. For Example: "SYK,AAL,etc..."

    Stock Data Ready Message:
        Message ID: 2
        Purpose:
            Sent from a client to a server to inform the server all stock data has been downloaded.
        Message Content:
            TBD, probably nothing. 

    Begin Training Message:
        Message ID: 3 
        Purpose:
            Sent from a server to a client so the client begins training on the stock data. 
            Client will sequentially train a model on the stock list.
        Message Content:
            Training hyper parameter list
            [EPOCHS].[LEARNING_RATE]

    Training complete message:
        Message ID: 4 
        Purpose:
            Sent from a client to a server to indicate training has been completed.
        Message Content:
            TBD, probably nothing. 

    Make Predictions message:
        Message ID: 5
        Purpose: 
            Sent from a server to a client to use the trained model to make predictions on the stock list. 
        Message Content:
            TBD, probably nothing.

    Predictions complete message:
        Message ID: 6 
        Purpose: 
            Sent from a client to server returns the prediction results 
        Message Content:
            [STOCK]:[0 | 1], ...
            Contains the stock ticker and the clients recommendation whether or not to invest in the stock.


Parallel App Message structure

    [APP_NAME].[MESSAGE_ID].[MESSAGE_CONTENTS]

Client App naming convention
    Predictor_[MARKET_TYPE]
    For example: Predictor_Technology