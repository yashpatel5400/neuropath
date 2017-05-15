"""
__name__ = accuracy.py
__author__ = Yash Patel
__description__ = Visualizes the output dumps provided by gem5, specifically
focusing on the differences in accuracy resulting from running the various
branch predictors, i.e. accuracy relative to the same input programs. Also
highlights the latency/time in prediction
"""

import settings as s

import os
import numpy as np
from plotly.graph_objs import Scatter, Figure, Layout
from plotly.offline import plot

def visualize_bps(cond_incorrects, indirect_incorrects, filename):
    trace1 = go.Bar(
        x=list(cond_incorrects.keys()),
        y=list(cond_incorrects.values()),
        name="Incorrect Conditional Predictions"
    )

    trace2 = go.Bar(
        x=list(indirect_incorrects.keys()),
        y=list(indirect_incorrects.values()),
        name="Incorrect Indirect Predictions"
    )
    
    data = [trace1, trace2]
    layout = go.Layout(
        barmode='group'
    )

    fig = go.Figure(data=data, layout=layout)
    plot(fig, filename=filename)
    
def analyze_executable(executable):
    """
    Given int corresponding to the executable to test on, runs the
    simulations for all the branch predictors, outputting results to 
    the gem5/m5cached directory, as both a figure and text output
    @param executable The integer corresponding to which executable to run
    @return void
    """
    command = "build/X86/gem5.opt configs/branch/predict.py --exec {} --pred {}"
    
    cond_incorrects    = {}
    indirect_incorrect = {}
    for i, name in enumerate(s.BP_NAMES):
        os.system(command.format(executable, i))
        
        dump = open(s.INPUT_FILE, "r").readlines()
        # only the stats relevant to the branch predictor
        pred_stats = [l for l in dump if "system.cpu.branchPred" in l]

        cond_incorrects[name] = [l.strip() for l in pred_stats
            if "condIncorrect" in l][0].split()[1]
        indirect_incorrect[name] = [l.strip() for l in pred_stats
            if "branchPredindirectMispredicted" in l][0].split()[1]
        
        print("Completed {}".format(name))
        print("===============================================")
    
    visualize_bps(cond_incorrects, indirect_incorrects,
                  s.EXEC_NAMES[executable])
    
    with open("conditional_{}.txt".format(s.EXEC_NAMES[executable])) as f:
        for name in cond_incorrects:
            f.writeline("{} : {}".format(name, cond_incorrects[name]))

    with open("indirect_{}.txt".format(s.EXEC_NAMES[executable])) as f:
        for name in indirect_incorrects:
            f.writeline("{} : {}".format(name, indirect_incorrects[name]))
            
if __name__ == "__main__":
    analyze_executable(0)
