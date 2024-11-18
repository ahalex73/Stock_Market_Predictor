# Make sure that you have all these libaries available to run the code successfully
from pandas_datareader import data
import matplotlib.pyplot as plt
import pandas as pd
import datetime as dt
import urllib.request, json
import os
import numpy as np
import tensorflow as tf # This code has been tested with TensorFlow 1.6
from sklearn.preprocessing import MinMaxScaler

def preProcessData(df):
    # Sort DataFrame by date
    print(df.columns)
    df = df.sort_values('Index')

    # # Double check the result
    print(df.head())
    print(df.loc[0, 'Index'])

    # First calculate the mid prices from the highest and lowest
    high_prices = df.loc[:,'High'].to_numpy()
    low_prices = df.loc[:,'Low'].to_numpy()
    mid_prices = (high_prices+low_prices)/2.0

    # plt.figure(figsize = (18,9))
    # plt.plot(range(df.shape[0]),(df['Low']+df['High'])/2.0)
    # plt.xticks(range(0,df.shape[0],500),df['Date'].loc[::500],rotation=45)
    # plt.xlabel('Date',fontsize=18)
    # plt.ylabel('Mid Price',fontsize=18)
    # plt.show()

    total_index = df.loc[0, 'Index']
    trainingProportion = 0.8
    split_index = int(total_index * trainingProportion)
    print(f"Split index: {split_index}")

    # Fit the scaler with the entire dataset
    scaler = MinMaxScaler(feature_range=(0, 1))
    scaler.fit(mid_prices.reshape(-1, 1))

    # Verify scaler is fitted
    print("Scaler fitted:", hasattr(scaler, 'data_min_'))
    print("Data min:", scaler.data_min_)
    print("Data max:", scaler.data_max_)

    # Transform entire dataset
    normalized_data = scaler.transform(mid_prices.reshape(-1, 1))

    # Split and reshape normalized data
    train_data = normalized_data[:split_index].flatten()
    test_data = normalized_data[split_index:].flatten()

    print("Train data range after normalization:", train_data.min(), train_data.max())
    print("Test data range after normalization:", test_data.min(), test_data.max())

    # train_data = mid_prices[:split_index]
    # test_data = mid_prices[split_index:]

    print(train_data[0])
    print(test_data[0])

    scaler = MinMaxScaler(feature_range=(0, 1))
    train_data = train_data.reshape(-1,1)
    test_data = test_data.reshape(-1,1)

    all_mid_data = np.concatenate([train_data,test_data],axis=0)
    # plt.figure(figsize = (18,9))
    # plt.plot(range(df.shape[0]),all_mid_data)
    # plt.xticks(range(0,df.shape[0],500),df['Date'].loc[::500],rotation=45)
    # plt.xlabel('Date',fontsize=18)
    # plt.ylabel('Reshaped Mid Price',fontsize=18)
    # plt.show()

    return all_mid_data, train_data, test_data

def makePrediction(df, train_data):
    window_size = 20
    N = train_data.size
    std_avg_predictions = []
    std_avg_x = []
    mse_errors = []

    for pred_idx in range(window_size,N):

        if pred_idx >= N:
            date = dt.datetime.strptime(k, '%Y-%m-%d').date() + dt.timedelta(days=1)
        else:
            date = df.loc[pred_idx,'Date']

        std_avg_predictions.append(np.mean(train_data[pred_idx-window_size:pred_idx]))
        mse_errors.append((std_avg_predictions[-1]-train_data[pred_idx])**2)
        std_avg_x.append(date)

    print('MSE error for standard averaging: %.5f'%(0.5*np.mean(mse_errors)))

    return train_data, std_avg_predictions