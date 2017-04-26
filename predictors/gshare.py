"""
__name__ = gshare.py
__author__ = Yash Patel
__description__ = Global share branch predictor, also used 
for reference for dynamic predictors, to ensure their accuracy 
gains are worth the additional time overhead
"""

import settings as s
from predictors.predictor import Predictor
from predictors.taken import Taken
    
class GSharePredictor(Predictor):
    def __init__(self, n):
        # 2n two-bit saturating counters.
        num_counters  = 2 ** n
        self.n = n
        self.history  = ["0"] * n
        self.counters = dict(zip(list(range(num_counters)),
                            num_counters * [Taken.StronglyNotTaken]))

    def train(self, data):
        # the saturating counters are integrated with the prediction
        # step, i.e. there is no separate training before evaluation
        pass

    def predict(self, inst):
        # prediction updates the counters such that they reflect newly
        # seen data (i.e. correspondence between counter/prediction)
        pc = inst[s.PC]

        # select counter from the table using using lower n bits of PC
        original  = bin(int(pc, 16))[-self.n:]
        predictor = int(int("".join(self.history), 2) ^ int(original, 2))
        predict_counter = self.counters[predictor]

        if predict_counter == Taken.StronglyTaken or \
           predict_counter == Taken.WeaklyTaken:

            prediction = 'T'
        else:
            prediction = 'N'

        # updates history
        self.history.append(str(int(inst[s.BRANCH] == 'T')))
        self.history = self.history[1:]
        
        # updates the counter statuses based on outcome of prediction
        if inst[s.BRANCH] == prediction:
            self.counters[predictor] = Taken.incr(self.counters[predictor])
        else:
            self.counters[predictor] = Taken.decr(self.counters[predictor])
        return prediction
