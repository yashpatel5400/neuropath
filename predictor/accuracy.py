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
    for i in [6]:
        os.system(command.format(isa, executable, i))
        name = s.BP_NAMES[i]
        dump = open(s.INPUT_FILE, "r").readlines()

        attributes = {"conditional" : "condIncorrect",
                      "indirect"    : "branchPredindirectMispredicted",
                      "latency"     : "host_seconds"}
        attribute_values = [[l.strip() for l in dump
            if attribute in l][0].split()[1] for attribute in attributes.values()]

        print("===============================================")
        print("Completed {}".format(name))
        print("===============================================")
        for attribute, value in zip(attributes, attribute_values):
            with open("m5cached/{}_{}.txt".format(
                name, s.EXEC_NAMES[executable]), "w") as f:
                f.write("{} : {}\n".format(attribute, value))
                
if __name__ == "__main__":
    for executable in [6]:
        analyze_executable("X86", executable)
