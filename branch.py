"""
__name__ = branch.py
__author__ = Yash Patel
__description__ = Branch prediction algorithm (simple non-neural
implementation) used for benchmarking. Visualization is handled
separately from the processing here
"""

import pandas as pd

def preprocess(filename):
    """
    Preprocesses the file at the specified name and returns only
    the branching (conditional) lines in the dump, defined to have
    (in the format specified for the offline dump versions):

    1) Reads the flags register (that is, conditionRegister == 'R'), and
    2) Is either taken or not taken (that is, TNnotBranch != '-').
    """
    pass

def main(filename):
    memdump = preprocess(filename)
    
if __name__ == "__main__":
    main(filename="data/gcc-1K.trace")
