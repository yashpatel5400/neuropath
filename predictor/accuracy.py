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

def visualize_bps(executable):
    files = os.listdir(s.OUTPUT_DIR)
    exec_name = s.EXEC_NAMES[executable]
    to_visualize = [f for f in files if exec_name in f]
    
    data = []
    for f in to_visualize:
        props = open("{}/{}".format(s.OUTPUT_DIR, f), "r").readlines()
        prop_values = [float(prop.split(":")[1].strip()) for prop in props]
        prop_labels = ["Conditional", "Indirect", "Latency"]
        data.append(Bar(x=prop_labels, y=prop_values, name=f.split("_")[0]))
        
    layout = Layout(
        barmode='group'
    )

    fig = Figure(data=data, layout=layout)
    plot(fig, filename="{}/{}.html".format(s.FIGURE_DIR, exec_name))
    
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
    for i in range(len(s.BP_NAMES)):
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
        with open("{}/{}_{}.txt".format(s.OUTPUT_DIR,
            name, s.EXEC_NAMES[executable]), "w") as f:
            for attribute, value in zip(attributes, attribute_values):
                f.write("{} : {}\n".format(attribute, value))
                
if __name__ == "__main__":
    for executable in range(6,13): # range(, 13):
        analyze_executable("X86", executable)
        # visualize_bps(executable)
