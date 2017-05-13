"""
__name__ = bimodal.py
__author__ = Yash Patel
__description__ = Bimodal branch predictor, the simplest dynamic
branch predictor that involves satuarating state predictor branch
predictor. Can vary size of memory held in states
"""

import settings as s
from predictors.predictor import Predictor
from predictors.taken import Taken
    
class BimodalPredictor(Predictor):
    def __init__(self, n):
        # 2n two-bit saturating counters.
        num_counters  = 2 ** n
        self.n = n
        self.counters = dict(zip(list(range(num_counters)),
                            num_counters * [Taken.StronglyNotTaken]))
        
    def predict(self, inst):
        # prediction updates the counters such that they reflect newly
        # seen data (i.e. correspondence between counter/prediction)
        pc = inst[s.PC]

        # select counter from the table using using lower n bits of PC
        predictor = int(bin(int(pc, 16))[-self.n:], 2)
        predict_counter = self.counters[predictor]

        if predict_counter == Taken.StronglyTaken or \
           predict_counter == Taken.WeaklyTaken:

            prediction = 'T'
        else:
            prediction = 'N'

        # updates the counter statuses based on outcome of prediction
        if inst[s.BRANCH] == prediction:
            self.counters[predictor] = Taken.incr(self.counters[predictor])
        else:
            self.counters[predictor] = Taken.decr(self.counters[predictor])
        return prediction
