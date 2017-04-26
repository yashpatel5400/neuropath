"""
__name__ = neural.py
__author__ = Yash Patel
__description__ = Neural branch predictor, which was of original
interest and to be compared to these other class benchmarks. Here
it is only presented as a simply preceptron unit, though we expand
into networks and reinforcement learning as well
"""

import numpy as np
from keras.models import Sequential
from keras.layers import Dense, Activation

import settings as s
from predictors.predictor import Predictor

class NeuralPredictor(Predictor):
    def __init__(self):
        self.model = Sequential([
            Dense(1, input_shape=(1,)),
            Activation('softmax'),
        ])

        self.model.compile(optimizer='rmsprop',
              loss='binary_crossentropy',
              metrics=['accuracy'])

    def train(self, data):
        # nothing to train in the case of static predictor - just pass
        # on the training, since behavior is predetermined
        pass

    def predict(self, inst):
        inp    = [np.array([int(inst[s.PC], 16)])]
        output = int(inst[s.BRANCH] == 'T')

        self.model.predict(inp)
        self.model.fit(inp, output, batch_size=1)
