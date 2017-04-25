"""
__name__ = static.py
__author__ = Yash Patel
__description__ = Static branch predictor, used largely as a reference
point for all the dynamic predictors, to ensure their accuracy gains
are worth the additional time overhead
"""

from predictors.predictor import Predictor
class StaticPredictor(Predictor):
    def train(self, data):
        # nothing to train in the case of static predictor - just pass
        # on the training, since behavior is predetermined
        pass

    def predict(self, inst):
        return 'T'
