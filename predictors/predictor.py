"""
__name__ = predictor.py
__author__ = Yash Patel
__description__ = Abstract predictor class used as the common
interface for all the implemented branch predictors
"""

from abc import ABC, abstractmethod
class Predictor(ABC):
    def __init__(self):
        pass
    
    @abstractmethod
    def predict(self, inst):
        pass
