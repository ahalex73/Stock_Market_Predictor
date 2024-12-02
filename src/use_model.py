import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.preprocessing import MinMaxScaler
import argparse
import os

# Load Stock Data from CSV
def load_stock_data_from_csv(csv_file_path):
    data = pd.read_csv(csv_file_path)
    # Assuming the CSV has a 'Date' column and 'Close' column
    # If your column names are different, adjust accordingly
    return data['Close'].values  # We will use closing prices for prediction

# Preprocess Data for LSTM
def preprocess_data(stock_data, sequence_length):
    # Normalize the data
    scaler = MinMaxScaler(feature_range=(0, 1))
    stock_data = stock_data.reshape(-1, 1)  # Reshape to 2D array for scaling
    scaled_data = scaler.fit_transform(stock_data)
    
    return scaled_data, scaler

# Load the trained model
def load_trained_model(model_save_path):
    if os.path.exists(model_save_path):
        model = tf.keras.models.load_model(model_save_path)
        print("Model loaded successfully!")
        return model
    else:
        print("No saved model found!")
        return None

def predict_next_days(model, recent_data, scaler, days=5):
    predictions = []
    current_input = recent_data[-1].reshape(1, -1)  # Get the most recent data
    
    for _ in range(days):
        prediction = model.predict(current_input)
        predictions.append(prediction)
        # Reshape the prediction to be 1D, and then update the current input with the new prediction
        prediction_1d = prediction.flatten()  # Flatten the 2D prediction to a 1D array
        current_input = np.concatenate((current_input[0][1:], prediction_1d), axis=0).reshape(1, -1)

    # Inverse transform the predictions to get the original scale
    predictions = scaler.inverse_transform(np.array(predictions).reshape(-1, 1))

    return predictions


# Generate a Rating (0 or 1) for Stock Movement
def generate_rating(predictions):
    # If the stock price is predicted to go up over the next 5 days, assign a rating of 1 (up)
    # If it is predicted to go down, assign a rating of 0 (down)
    
    # Check if the last predicted value is greater than the first predicted value
    if predictions[-1] > predictions[0]:
        return 1  # Stock price is predicted to go up
    else:
        return 0  # Stock price is predicted to go down
    


def main(saved_model_file_path, num_unrollings=50, batch_size=500, days=5):

    STOCKS_DATA_FOLDER = 'Stocks/'
    
    # Construct CSV filepath from saved_model_file_path
    filename = os.path.basename(saved_model_file_path)  # Extracts "SYK_model.keras"
    ticker = filename.split('_')[0]                     # Extracts SYK
    #print(ticker)                                      # Output: SYK
    model_name = (STOCKS_DATA_FOLDER + 'stock_market_data-' + ticker + '.csv')

    # Load stock data from csv
    stock_data = load_stock_data_from_csv(model_name)
    scaled_data, scaler = preprocess_data(stock_data, num_unrollings)
    
    # Get the most recent `num_unrollings` days for prediction
    recent_data = scaled_data[-num_unrollings:]
    
    # Load the trained model
    model = load_trained_model(saved_model_file_path)
    if model:
        # Predict the next 5 days
        predictions = predict_next_days(model, recent_data, scaler, days)
        print("Predictions for the next 5 days:", predictions)
        
        # Generate the rating (0 or 1)
        rating = generate_rating(predictions)
        print(f"Stock prediction rating: {'Up' if rating == 1 else 'Down'}")
    
    
    return rating # Returns a 1 for up, 0 for down

if __name__ == '__main__':
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Stock Predictor")
    parser.add_argument('--saved_model_path', help='Path to the saved model (example: --saved_model_path TrainedModels/SYK_model.keras)', required=True)
    parser.add_argument('--num_unrollings', help='Number of days of data to use for prediction (default: 50)', type=int, default=50)
    parser.add_argument('--batch_size', help='Batch size for prediction (default: 500)', type=int, default=500)
    parser.add_argument('--days', help='Number of days to predict (default: 5)', type=int, default=5)
    
    args = parser.parse_args()
    
    # Call the main function with the parsed arguments
    main(args.saved_model_path, args.num_unrollings, args.batch_size, args.days)
