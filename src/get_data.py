import pandas as pd
import datetime as dt
import urllib.request, json
import matplotlib.pyplot as plt
import numpy as np
import os
from sklearn.preprocessing import MinMaxScaler


def GetData(ticker):
    data_source = 'alphavantage' # alphavantage or kaggle
    # df
    if data_source == 'alphavantage':
        # ====================== Loading Data from Alpha Vantage ==================================

        api_key = 'FON7BSF9U902A7WO'

        # American Airlines stock market prices
        # ticker = "AAL"

        # JSON file with all the stock market data for AAL from the last 20 years
        url_string = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%s&outputsize=full&apikey=%s"%(ticker,api_key)

        # Save data to this file
        file_to_save = 'stock_market_data-%s.csv'%ticker

        # If you haven't already saved data,
        # Go ahead and grab the data from the url
        # And store date, low, high, volume, close, open values to a Pandas DataFrame
        if not os.path.exists(file_to_save):
            with urllib.request.urlopen(url_string) as url:
                data = json.loads(url.read().decode())
                # extract stock market data
                data = data['Time Series (Daily)']
                df = pd.DataFrame(columns=['Date','Low','High','Close','Open'])
                for k,v in data.items():
                    date = dt.datetime.strptime(k, '%Y-%m-%d')
                    data_row = [date.date(),float(v['3. low']),float(v['2. high']),
                                float(v['4. close']),float(v['1. open'])]
                    df.loc[-1,:] = data_row
                    df.index = df.index + 1
            print('Data saved to : %s'%file_to_save)        
            df.to_csv(file_to_save, index_label="Index")

        # If the data is already there, just load it from the CSV
        else:
            print('File already exists. Loading data from CSV')
            df = pd.read_csv(file_to_save)


    else:
        # ====================== Loading Data from Kaggle ==================================
        # You will be using HP's data. Feel free to experiment with other data.
        # But while doing so, be careful to have a large enough dataset and also pay attention to the data normalization
        df = pd.read_csv(os.path.join('Stocks','hpq.us.txt'),delimiter=',',usecols=['Date','Open','High','Low','Close'])
        print('Loaded data from the Kaggle repository')
    
    return df

# df = GetData("SYK")
# # Sort DataFrame by date
# print(df.columns)
# df = df.sort_values('Index')

# # # Double check the result
# print(df.head())
# print(df.loc[0, 'Index'])

# # First calculate the mid prices from the highest and lowest
# high_prices = df.loc[:,'High'].to_numpy()
# low_prices = df.loc[:,'Low'].to_numpy()
# mid_prices = (high_prices+low_prices)/2.0

# plt.figure(figsize = (18,9))
# plt.plot(range(df.shape[0]),(df['Low']+df['High'])/2.0)
# plt.xticks(range(0,df.shape[0],500),df['Date'].loc[::500],rotation=45)
# plt.xlabel('Date',fontsize=18)
# plt.ylabel('Mid Price',fontsize=18)
# plt.show()

# total_index = df.loc[0, 'Index']
# trainingProportion = 0.8
# split_index = int(total_index * trainingProportion)
# print(f"Split index: {split_index}")

# # Fit the scaler with the entire dataset
# scaler = MinMaxScaler(feature_range=(0, 1))
# scaler.fit(mid_prices.reshape(-1, 1))

# # Verify scaler is fitted
# print("Scaler fitted:", hasattr(scaler, 'data_min_'))
# print("Data min:", scaler.data_min_)
# print("Data max:", scaler.data_max_)

# # Transform entire dataset
# normalized_data = scaler.transform(mid_prices.reshape(-1, 1))

# # Split and reshape normalized data
# train_data = normalized_data[:split_index].flatten()
# test_data = normalized_data[split_index:].flatten()

# print("Train data range after normalization:", train_data.min(), train_data.max())
# print("Test data range after normalization:", test_data.min(), test_data.max())


# # train_data = mid_prices[:split_index]
# # test_data = mid_prices[split_index:]

# print(train_data[0])
# print(test_data[0])

# scaler = MinMaxScaler(feature_range=(0, 1))
# train_data = train_data.reshape(-1,1)
# test_data = test_data.reshape(-1,1)

# all_mid_data = np.concatenate([train_data,test_data],axis=0)
# plt.figure(figsize = (18,9))
# plt.plot(range(df.shape[0]),all_mid_data)
# plt.xticks(range(0,df.shape[0],500),df['Date'].loc[::500],rotation=45)
# plt.xlabel('Date',fontsize=18)
# plt.ylabel('Reshaped Mid Price',fontsize=18)
# plt.show()

# window_size = 20
# N = train_data.size
# std_avg_predictions = []
# std_avg_x = []
# mse_errors = []

# for pred_idx in range(window_size,N):

#     if pred_idx >= N:
#         date = dt.datetime.strptime(k, '%Y-%m-%d').date() + dt.timedelta(days=1)
#     else:
#         date = df.loc[pred_idx,'Date']

#     std_avg_predictions.append(np.mean(train_data[pred_idx-window_size:pred_idx]))
#     mse_errors.append((std_avg_predictions[-1]-train_data[pred_idx])**2)
#     std_avg_x.append(date)

# print('MSE error for standard averaging: %.5f'%(0.5*np.mean(mse_errors)))

# print(train_data[0])
# print(test_data[0])

# plt.figure(figsize = (18,9))
# plt.plot(range(df.shape[0]),all_mid_data,color='b',label='True')
# plt.plot(range(window_size,N),std_avg_predictions,color='orange',label='Prediction')
# plt.xticks(range(0, df.shape[0], 500), df['Date'].iloc[::500], rotation=45)
# plt.xlabel('Date')
# plt.ylabel('Mid Price')
# plt.legend(fontsize=18)
# plt.show()
