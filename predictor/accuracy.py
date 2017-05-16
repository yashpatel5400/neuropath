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
from plotly.graph_objs import Bar, Figure, Layout
from plotly.offline import plot

def visualize_bps(cond_incorrects, indirect_incorrect, filename):
    data = []
    for predictor in cond_incorrects.keys():
        x = ["Conditional", "Indirect"]
        y = [cond_incorrects[predictor], indirect_incorrect[predictor]]        
        data.append(Bar(x=x, y=y, name=predictor))

    layout = Layout(
        barmode='group'
    )

    fig = Figure(data=data, layout=layout)
    plot(fig, filename=filename)
    
def analyze_executable(isa, executable):
    """
    Given int corresponding to the executable to test on, runs the
    simulations for all the branch predictors, outputting results to 
    the gem5/m5cached directory, as both a figure and text output
    @param executable The integer corresponding to which executable to run
    @return void
    """
    command = "build/{}/gem5.opt configs/branch/predict.py --exec {} --pred {}"
    
    cond_incorrects    = {}
    indirect_incorrect = {}
    for i, name in enumerate(s.BP_NAMES):
        os.system(command.format(isa, executable, i))
        
        dump = open(s.INPUT_FILE, "r").readlines()
        # only the stats relevant to the branch predictor
        pred_stats = [l for l in dump if "system.cpu.branchPred" in l]

        cond_incorrects[name] = [l.strip() for l in pred_stats
            if "condIncorrect" in l][0].split()[1]
        indirect_incorrect[name] = [l.strip() for l in pred_stats
            if "branchPredindirectMispredicted" in l][0].split()[1]
        
        print("===============================================")
        print("Completed {}".format(name))
        print("===============================================")
        with open("m5cache/conditional_{}_{}.txt".format(
                name, s.EXEC_NAMES[executable]), "w") as f:
            f.write("{} : {}\n".format(name, cond_incorrects[name]))

        with open("m5cache/indirect_{}_{}.txt".format(
                name, s.EXEC_NAMES[executable]), "w") as f:
            f.write("{} : {}\n".format(name, indirect_incorrect[name]))

    visualize_bps(cond_incorrects, indirect_incorrect,
                  "{}.html".format(s.EXEC_NAMES[executable]))
                
if __name__ == "__main__":
    for executable in [5]:
        analyze_executable("X86", executable)
