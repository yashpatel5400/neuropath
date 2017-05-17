"""
__name__ = settings.py
__author__ = Yash Patel
__description__ = Settings file for the accuracy predictor plot
"""

# NOTE: All the below paths are relative to the starting directory of
# gem5, since this is the default behavior for gem5 execution

# --------------------------- Sim Params  ---------------------------- #
BP_NAMES = [
    "LocalBP",      # simple local history branch predictor
    "TournamentBP", # mixed global/local history predictor
    "BiModeBP",     # 2-bit history mode predictor
    "LTAGE",        # often best-performing current mainstream predictor
    "AlwaysBP",     # always true branch predictor (static)
    "NeuroBP",      # single perceptron neural branch predictor
    "NeuroPathBP"   # neural path branch predictor
]

EXEC_NAMES = [
    "ConnCompSmall",  # 0
    "ConnCompMedium", # 1

    "Bubblesort",     # 2
    "IntMM",          # 3
    "Oscar",          # 4
    "Perm",           # 5
    "Puzzle",         # 6
    "Queens",         # 7
    "Quicksort",      # 8
    "RealMM",         # 9
    "Towers",         # 10
    "Treesort",       # 11
]

# --------------------------- Input Specs  ---------------------------- #
# location of the final results dump file
INPUT_FILE = "m5out/stats.txt"

# --------------------------- Output Specs ---------------------------- #
# outputs are by convention specified by executable
OUTPUT_DIR = "m5cached"

# outputs are by convention specified by executable
FIGURE_DIR = "m5cached/{}/figures"
