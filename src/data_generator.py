import numpy as np
from tensorflow.keras.utils import Sequence

class DataGeneratorSeq(Sequence):
    def __init__(self, prices, batch_size, num_unroll, predict, **kwargs):
        super().__init__(**kwargs)  # Ensure super constructor is called
        # Ensure prices is a numpy array for efficient indexing
        self._prices = np.array(prices, dtype=np.float32)
        self._prices_length = len(self._prices) - num_unroll
        self._batch_size = batch_size
        self._num_unroll = num_unroll
        self._predict = predict 
        # Define the number of segments for batching
        self._segments = self._prices_length // self._batch_size
        # Initialize cursors for each batch
        self._cursor = [offset * self._segments for offset in range(self._batch_size)]
    
    
    def __getitem__(self, index):
        # Initialize batch_data and batch_labels with the correct shape
        batch_data = np.zeros((self._batch_size, self._num_unroll, 1), dtype=np.float32)
        batch_labels = np.zeros((self._batch_size, self._num_unroll, 1), dtype=np.float32)

        # Fill in the data and labels for each batch
        for b in range(self._batch_size):
            for n in range(self._num_unroll):
                # If cursor exceeds the price length, reset to a random index
                if self._cursor[b] + 1 >= self._prices_length:
                    self._cursor[b] = np.random.randint(0, (b + 1) * self._segments)
                
                # Assign data and labels for this time step
                batch_data[b, n, 0] = self._prices[self._cursor[b]]
                batch_labels[b, n, 0] = self._prices[self._cursor[b] + 1]
                
                # Update cursor for the next time step
                self._cursor[b] = (self._cursor[b] + 1) % self._prices_length

        if self._predict:
            return batch_data  # Return only data during prediction phase
        else:
            return batch_data, batch_labels

    def unroll_batches(self):
        # Function to unroll multiple batches
        unroll_data, unroll_labels = [], []
        for ui in range(self._num_unroll):
            data, labels = self.next_batch()    
            unroll_data.append(data)
            unroll_labels.append(labels)

        if self._predict:
            return unroll_data  # Return unrolled data for prediction
        else:
            return unroll_data, unroll_labels

    def reset_indices(self):
        # Reset the cursor for each batch at the beginning of each epoch
        for b in range(self._batch_size):
            self._cursor[b] = np.random.randint(0, min((b + 1) * self._segments, self._prices_length - 1))

    def __len__(self):
        # Return the number of steps per epoch
        return self._segments // self._batch_size
    
    def get_data(self):
        # This method returns data and labels as required for model prediction.
        while True:
            data, labels = self.next_batch()
            yield data, labels