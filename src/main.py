# Library imports 
import matplotlib.pyplot as plt
import tensorflow as tf 
from tensorflow.keras.preprocessing import timeseries_dataset_from_array
import numpy as np 
from sklearn.metrics import mean_absolute_error, mean_squared_error

# User imports
import get_data
import predictor
from data_generator import DataGeneratorSeq

def main():
    dataFrame = get_data.GetData("SYK")
    midData, trainData, testData = predictor.preProcessData(dataFrame)
    print(f"{len(trainData)}")
    # trainData, std_avg_predictions = predictor.makePrediction(dataFrame, trainData)

    # plt.figure(figsize = (18,9))
    # plt.plot(range(dataFrame.shape[0]),midData,color='b',label='True')
    # plt.plot(range(20,trainData.size),std_avg_predictions,color='orange',label='Prediction')
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

    # # Data generator for training data
    # dg = DataGeneratorSeq(trainData, batch_size, num_unrollings, False)

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

    # Check if the last batch size is smaller than the batch size

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
    # # Step 5: Training Loop (Example)
    # @tf.function
    # def train_step(model, batch_data, batch_labels):
    #     with tf.GradientTape() as tape:
    #         predictions = model(batch_data, training=True)
    #         loss_fn = tf.keras.losses.MeanSquaredError()
    #         loss = loss_fn(batch_labels, predictions)
    #     grads = tape.gradient(loss, model.trainable_variables)
    #     model.optimizer.apply_gradients(zip(grads, model.trainable_variables))
    #     return loss
    
    # # In practice, you would replace this with real training data and loops
    # epochs = 1  # Set to your desired number of epochs

    # # Generate training data batch-wise and fit the model
    # for epoch in range(epochs):
    #     for step in range(len(trainData) // batch_size):
    #         batch_data, batch_labels = dg.next_batch()

    #         # Reshape batch data for LSTM input
    #         batch_data = batch_data.reshape(batch_size, num_unrollings, D)
    #         batch_labels = batch_labels.reshape(batch_size, num_unrollings, 1)

    #         # Ensure that both batch_data and batch_labels are tensors
    #         batch_data = tf.convert_to_tensor(batch_data, dtype=tf.float32)
    #         batch_labels = tf.convert_to_tensor(batch_labels, dtype=tf.float32)

    #         # Train model on the batch
    #         loss = train_step(model, batch_data, batch_labels)
    #         print(f'Epoch {epoch+1}, Step {step+1}, Loss: {loss}')

   # Step 6: Making Predictions (optional)
    # Assuming test_data is generated similarly to how training data is created
    test_data = np.random.rand(500, 50, 1)  # Example shape (500 samples, 50 time steps, 1 feature)
    # test_labels = np.random.rand(500, 1)  # Replace with your actual labels

    # Step 2: Make predictions
    # model.reset_states()
    # Use a loop to handle predictions manually:
    # dg_test = DataGeneratorSeq(testData, 32, num_unrollings, predict=True)
    # predictions = []
    # for batch_data in dg_test:
    #     pred = model.predict(batch_data, batch_size=32)  # predict on the batch
    #     predictions.append(pred)

    # Convert predictions to numpy array for easy analysis
    # predictions = np.concatenate(predictions, axis=0)# Define parameters (same as used in training)
    # predict_sequence_length = 50
    # predict_batch_size = 500

    # # Prepare the test dataset (Ensure it has the same structure as trainData)
    # test_dataset = tf.keras.utils.timeseries_dataset_from_array(
    #     data=testData,  # Use testData here
    #     targets=testData[predict_sequence_length:],  # Shifted version of testData for the target
    #     sequence_length=predict_sequence_length,
    #     batch_size=predict_batch_size,
    #     shuffle=False  # Do not shuffle for prediction, as the order matters
    # )

    # # Make predictions on the test data
    # predictions = model.predict(test_dataset)
    # Define your batch size
    batch_size = 500
    sequence_length = 50  # Your sequence length

    # Split the test data into windows of sequences
    test_data = np.array(testData)  # Convert to a numpy array if it's not already
    total_samples = len(test_data)

    # Calculate the number of batches (full and partial)
    full_batches = total_samples // batch_size
    remaining_samples = total_samples % batch_size
    print(f"full batches: {full_batches}, remaining samples: {remaining_samples}")

    # Create the dataset (we will use a for loop to handle dynamic batch sizes)
    predictions = []

    for i in range(full_batches):
        # Get the current batch
        batch_start = i * batch_size
        batch_end = (i + 1) * batch_size
        batch_data = test_data[batch_start:batch_end]
        print(f"Batch {i} shape: {batch_data.shape}")
        
        # Create the dataset for the batch
        batch_dataset = tf.keras.utils.timeseries_dataset_from_array(
            data=batch_data,
            targets=batch_data[sequence_length:],
            sequence_length=sequence_length,
            batch_size=batch_size,
            shuffle=False
        )
        for batch_x, batch_y in dataset.take(1):
            print("Input batch shape:", batch_x.shape)  # Should be (batch_size, sequence_length, 1)
            print("Target batch shape:", batch_y.shape)  # Should be (batch_size, 1)

        # Make predictions for this batch
        batch_predictions = model.predict(batch_dataset, batch_size=batch_size)
        predictions.append(batch_predictions)

    # Handle the remaining smaller batch if there are any leftover samples
    if remaining_samples > 0:
        # Get the last batch (which might be smaller than batch_size)
        last_batch_data = test_data[full_batches * batch_size:]
        
        # Create the dataset for the smaller batch
        last_batch_dataset = tf.keras.utils.timeseries_dataset_from_array(
            data=last_batch_data,
            targets=last_batch_data[sequence_length:],
            sequence_length=sequence_length,
            batch_size=remaining_samples,  # Smaller batch size
            shuffle=False
        )
        
        # Make predictions for the last batch
        last_batch_predictions = model.predict(last_batch_dataset)
        predictions.append(last_batch_predictions)

    # Convert predictions to a single numpy array (if needed)
    predictions = np.concatenate(predictions, axis=0)

    # The predictions will be in the shape of (num_samples, sequence_length, 1), adjust as needed
    print(predictions)

    # Optionally, plot predictions vs actual data (e.g., for the first batch)
    plt.figure(figsize=(18, 9))
    plt.plot(range(len(predictions)), predictions, color='orange', label='Predictions')

    # We assume that the last value of each batch corresponds to the target
    # (i.e., the stock price) in the actual data
    plt.plot(range(len(dg_test[0][0])), dg_test[0][0][:, -1, 0], color='blue', label='Actual')

    plt.xlabel('Time Step')
    plt.ylabel('Stock Price')
    plt.legend()
    plt.show()

    # Calculate Mean Absolute Error (MAE)
    mae = mean_absolute_error(test_labels, predictions)
    print(f"Mean Absolute Error (MAE): {mae}")

    # Calculate Mean Squared Error (MSE)
    mse = mean_squared_error(test_labels, predictions)
    print(f"Mean Squared Error (MSE): {mse}")

    # Calculate Root Mean Squared Error (RMSE)
    rmse = np.sqrt(mse)
    print(f"Root Mean Squared Error (RMSE): {rmse}")



if __name__ == '__main__':
    main()