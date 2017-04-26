"""
__name__ = bimodal.py
__author__ = Yash Patel
__description__ = Bimodal branch predictor, the simplest dynamic
branch predictor that involves satuarating state predictor branch
predictor. Can vary size of memory held in states
"""

from enum import Enum

import settings as s
from predictors.predictor import Predictor

class Taken(Enum):
    StronglyNotTaken = 1
    WeaklyNotTaken   = 2
    WeaklyTaken      = 3
    StronglyTaken    = 4

    def incr(taken_status):
        if taken_status == StronglyNotTaken:
            return WeaklyNotTaken
        elif taken_status == WeaklyNotTaken:
            return WeaklyTaken
        elif taken_status == WeaklyTaken:
            return StronglyTaken
        return StronglyTaken # staturating state for increasing case

    def decr(taken_status):
        if taken_status == StronglyTaken:
            return WeaklyTaken
        elif taken_status == WeaklyTaken:
            return WeaklyNotTaken
        elif taken_status == WeaklyNotTaken:
            return StronglyNotTaken
        return StronglyNotTaken # staturating state for decreasing case
    
class BimodalPredictor(Predictor):
    def __init__(self, n):
        # 2n two-bit saturating counters.
        num_counters  = 2 ** n
        self.n = n
        self.counters = zip(list(range(num_counters)),
                            num_counters * [Taken.StronglyNotTaken])

    def train(self, data):
        # the saturating counters are integrated with the prediction
        # step, i.e. there is no separate training before evaluation
        pass

    def predict(self, inst):
        # prediction updates the counters such that they reflect newly
        # seen data (i.e. correspondence between counter/prediction)
        pc = inst[s.PC]

        # select counter from the table using using lower n bits of PC
        predictor = int(bin(int(pc, 16))[-self.n:], 2)
        predict_counter = self.counters[prediction]

        if predict_counter = Taken.StronglyTaken or \\
           predict_counter = Taken.WeaklyTaken:

            prediction = 'T'
        else:
            prediction = 'N'

        # updates the counter statuses based on outcome of prediction
        if inst[s.BRANCH] == prediction:
            self.counters[prediction] = Taken.incr(self.counters[prediction])
        else:
            self.counters[prediction] = Taken.decr(self.counters[prediction])
        return prediction
