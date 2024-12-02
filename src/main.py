import matplotlib.pyplot as plt
import tensorflow as tf 
from tensorflow.keras.preprocessing import timeseries_dataset_from_array
import numpy as np 
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import argparse
import get_data
import predictor
from data_generator import DataGeneratorSeq

def train_and_save_model(ticker, model_save_path):
    dataFrame = get_data.GetData(ticker)
    midData, trainData, testData = predictor.preProcessData(dataFrame)
    print(f"{len(trainData)}")

    # Model Configuration Parameters
    D = 1                        # Dimensionality of the data
    num_unrollings = 50          # Number of time steps to look into the future
    batch_size = 500             # Number of samples in a batch
    num_nodes = [200, 200, 150]  # Number of hidden nodes in each LSTM layer
    dropout = 0.2                # Dropout amount

    tf.keras.backend.clear_session()

    max_samples = (len(trainData) - num_unrollings) // batch_size * batch_size
    trainData = trainData[:max_samples + num_unrollings]

    dataset = tf.keras.preprocessing.timeseries_dataset_from_array(
        data=trainData, 
        targets=trainData[num_unrollings:],  
        sequence_length=num_unrollings,
        batch_size=batch_size,
        shuffle=True,
    )

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

    train_inputs = tf.keras.Input(shape=(num_unrollings, D), batch_size=batch_size)

    initial_states = [tf.zeros((batch_size, num_nodes[0])) for _ in range(2)] 
    lstm_outputs, *state = lstm_layers[0](train_inputs, initial_state=initial_states)

    for lstm_layer in lstm_layers[1:]:
        lstm_outputs, *state = lstm_layer(lstm_outputs)

    dense_layer = tf.keras.layers.Dense(units=1)
    outputs = dense_layer(lstm_outputs)

    model = tf.keras.Model(inputs=train_inputs, outputs=outputs)
    model.compile(optimizer=tf.keras.optimizers.Adam(), loss='mse')

    model.fit(dataset, epochs=1, verbose=1)

    # Construct File Extension
    model_save_path += ticker
    model_save_path += '_'
    model_save_path += 'model.keras'

    # Save the trained model
    model.save(model_save_path)
    print(f"Model saved to {model_save_path}")

    return model


def main(ticker):
    """ Train and Save a model """
    MODEL_SAVE_FOLDER = 'TrainedModels/'
    model = train_and_save_model(ticker, MODEL_SAVE_FOLDER)
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Stock Predictor")
    parser.add_argument('--stock', help='Stock Ticker (example: --stock SYK)')
    args = parser.parse_args()
    main(args.stock)
