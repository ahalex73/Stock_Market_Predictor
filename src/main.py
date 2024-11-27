# Library imports 
import matplotlib.pyplot as plt
import tensorflow as tf 
from tensorflow.keras.preprocessing import timeseries_dataset_from_array
import numpy as np 
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import argparse

# User imports
import get_data
import predictor
from data_generator import DataGeneratorSeq

def main(ticker):
    dataFrame = get_data.GetData(ticker)
    midData, trainData, testData = predictor.preProcessData(dataFrame)
    print(f"{len(trainData)}")
    # trainData, std_avg_predictions = predictor.makePrediction(dataFrame, trainData)

    # plt.figure(figsize = (18,9))
    # plt.plot(range(dataFrame.shape[0]),midData,color='b',label='True')
    # plt.xticks(range(0, dataFrame.shape[0], 500), dataFrame['Date'].iloc[::500], rotation=45)
    # plt.xlabel('Date')
    # plt.ylabel('Mid Price')
    # plt.legend(fontsize=18)
    # plt.show()

    # Step 2: Model Configuration Parameters
    D = 1  # Dimensionality of the data
    num_unrollings = 50  # Number of time steps to look into the future
    batch_size = 500  # Number of samples in a batch
    num_nodes = [200, 200, 150]  # Number of hidden nodes in each LSTM layer
    dropout = 0.2  # Dropout amount

    # Reset state 
    tf.keras.backend.clear_session()
    
    # Parameters
    max_samples = (len(trainData) - num_unrollings) // batch_size * batch_size
    trainData = trainData[:max_samples + num_unrollings]

    print(f"Traindata len: {len(trainData)}")
    # Generate the dataset
    dataset = timeseries_dataset_from_array(
        data=trainData,  # Input data
        targets=trainData[num_unrollings:],  # Target is the next value
        sequence_length=num_unrollings,
        batch_size=batch_size,
        shuffle=True,
    )
    for i, (batch_x, batch_y) in enumerate(dataset):
        print(f"Batch {i} - Input batch shape: {batch_x.shape}, Target batch shape: {batch_y.shape}")

        # If it's the last batch, check the size
        if i == len(dataset) - 1:
            print(f"Last batch - Input batch size: {batch_x.shape[0]}, Target batch size: {batch_y.shape[0]}")

    # Step 3: Build the LSTM Model
    # Define LSTM layers with dropout
    lstm_layers = [
        tf.keras.layers.LSTM(
            units=num_nodes[li],
            return_sequences=True if li < len(num_nodes) - 1 else False,
            return_state=True,
            dropout=dropout,
            recurrent_dropout=dropout
        )
        for li in range(len(num_nodes))
    ]

    # Define the input placeholder for model
    train_inputs = tf.keras.Input(shape=(num_unrollings, D), batch_size=batch_size)

    # Initial states for the first LSTM layer
    initial_states = [tf.zeros((batch_size, num_nodes[0])) for _ in range(2)]  # c0 and h0 for first LSTM layer
    lstm_outputs, *state = lstm_layers[0](train_inputs, initial_state=initial_states)

    # Pass through subsequent LSTM layers
    for lstm_layer in lstm_layers[1:]:
        lstm_outputs, *state = lstm_layer(lstm_outputs)

    # Final output layer (Dense)
    dense_layer = tf.keras.layers.Dense(units=1)
    outputs = dense_layer(lstm_outputs)

    # Define the complete model
    model = tf.keras.Model(inputs=train_inputs, outputs=outputs)

    # Step 4: Compile the Model
    model.compile(optimizer=tf.keras.optimizers.Adam(), loss='mse')

    model.fit(dataset, epochs=1, verbose=1)

    # Make predictions on the test data
    sequence_length = num_unrollings  

    # Adjusting test batch size so generated time series data sets are the correct size. 
    adjusted_batch_size = batch_size + sequence_length - 1

    # Split the test data into windows of sequences
    test_data = np.array(testData)  # Convert to a numpy array if it's not already
    total_samples = len(test_data)

    # Calculate the number of full batches that can be produced from test_data 
    full_batches = total_samples // adjusted_batch_size
    remaining_samples = total_samples % adjusted_batch_size

    print(f"Test Data size before adjustment: {total_samples}")
    print(f"Adjusted Batch size: {adjusted_batch_size}")
    print(f"full batches: {full_batches}, remaining samples: {remaining_samples}")

    # Drop the incomplete batch from the beginning of the data set. 
    test_data = test_data[-(full_batches * adjusted_batch_size):]
    print(f"Adjusted test data size: {len(test_data)}")

    # Create the dataset (we will use a for loop to handle dynamic batch sizes)
    predictions = []

    for i in range(full_batches):
        # Get the current batch
        batch_start = i * adjusted_batch_size
        batch_end = (i + 1) * adjusted_batch_size
        batch_data = test_data[batch_start:batch_end]
        print(f"Batch {i} shape: {batch_data.shape}")
        
        # Create the dataset for the batch
        batch_dataset = tf.keras.utils.timeseries_dataset_from_array(
            data=batch_data,
            targets=batch_data,
            sequence_length=sequence_length,
            batch_size=batch_size,
            shuffle=False
        )
        for batch_x, batch_y in batch_dataset.take(1):
            print("Input batch shape:", batch_x.shape)  # Should be (batch_size, sequence_length, 1)
            print("Target batch shape:", batch_y.shape)  # Should be (batch_size, 1)

        # Make predictions for this batch
        batch_predictions = model.predict(batch_dataset, batch_size=batch_size)
        predictions.append(batch_predictions)

    # Convert predictions to a single numpy array (if needed)
    predictions = np.concatenate(predictions, axis=0)

    # Remove the data points where predictions were not made. 
    actual_values = test_data[sequence_length - 1:sequence_length - 1 + len(predictions)]
    print(f"Predictions len {len(predictions)}, Actual data len: {len(actual_values)}")

    # Calculate metrics
    mae = mean_absolute_error(actual_values, predictions)
    mse = mean_squared_error(actual_values, predictions)
    rmse = np.sqrt(mse)
    r2 = r2_score(actual_values, predictions)
    mape = np.mean(np.abs((actual_values - predictions) / actual_values)) * 100

    print(f"Mean Absolute Error (MAE): {mae}")
    print(f"Mean Squared Error (MSE): {mse}")
    print(f"Root Mean Squared Error (RMSE): {rmse}")
    print(f"Mean Absolute Percentage Error (MAPE): {mape}%")
    print(f"R² Score: {r2}")

    # Optionally, plot predictions vs actual data (e.g., for the first batch)
    plt.figure(figsize=(10, 6))
    plt.plot(actual_values, label="Actual Values", color="blue")
    plt.plot(predictions, label="Predictions", color="orange", linestyle="--")

    plt.title("Predictions vs Actual Values")
    plt.xlabel("Time Step")
    plt.ylabel("Value")
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == '__main__':
    # Initialize parser
    parser = argparse.ArgumentParser(description="Stock Predictor")

    # Add arguments
    parser.add_argument('--stock', help='Stock Ticker (example: --stock AAL)')

    # Parse arguments
    args = parser.parse_args()

    # Access arguments
    print(f"Input: {args.stock}")

    main(args.stock)