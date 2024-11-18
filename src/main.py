# Library imports 
import matplotlib.pyplot as plt
import tensorflow as tf # This code has been tested with TensorFlow 1.6
import numpy as np 
from sklearn.metrics import mean_absolute_error, mean_squared_error

# User imports
import get_data
import predictor
from data_generator import DataGeneratorSeq

def main():
    dataFrame = get_data.GetData("SYK")
    midData, trainData, testData = predictor.preProcessData(dataFrame)
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

    # Data generator for training data
    dg = DataGeneratorSeq(trainData, batch_size, num_unrollings, False)

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

    # Step 5: Training Loop (Example)
    @tf.function
    def train_step(model, batch_data, batch_labels):
        with tf.GradientTape() as tape:
            predictions = model(batch_data, training=True)
            loss_fn = tf.keras.losses.MeanSquaredError()
            loss = loss_fn(batch_labels, predictions)
        grads = tape.gradient(loss, model.trainable_variables)
        model.optimizer.apply_gradients(zip(grads, model.trainable_variables))
        return loss
    
    # In practice, you would replace this with real training data and loops
    epochs = 1  # Set to your desired number of epochs

    # Generate training data batch-wise and fit the model
    for epoch in range(epochs):
        for step in range(len(trainData) // batch_size):
            batch_data, batch_labels = dg.next_batch()

            # Reshape batch data for LSTM input
            batch_data = batch_data.reshape(batch_size, num_unrollings, D)
            batch_labels = batch_labels.reshape(batch_size, num_unrollings, 1)

            # Ensure that both batch_data and batch_labels are tensors
            batch_data = tf.convert_to_tensor(batch_data, dtype=tf.float32)
            batch_labels = tf.convert_to_tensor(batch_labels, dtype=tf.float32)

            # Train model on the batch
            loss = train_step(model, batch_data, batch_labels)
            print(f'Epoch {epoch+1}, Step {step+1}, Loss: {loss}')

   # Step 6: Making Predictions (optional)
    # Assuming test_data is generated similarly to how training data is created
    test_data = np.random.rand(500, 50, 1)  # Example shape (500 samples, 50 time steps, 1 feature)
    # test_labels = np.random.rand(500, 1)  # Replace with your actual labels

    # Step 2: Make predictions
    # model.reset_states()
    # Use a loop to handle predictions manually:
    dg_test = DataGeneratorSeq(testData, 32, num_unrollings, predict=True)
    # predictions = []
    # for batch_data in dg_test:
    #     pred = model.predict(batch_data, batch_size=32)  # predict on the batch
    #     predictions.append(pred)

    # Convert predictions to numpy array for easy analysis
    # predictions = np.concatenate(predictions, axis=0)
    predictions = model.predict(dg_test, steps=len(dg_test))

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