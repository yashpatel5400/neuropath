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

def visualize_bps(isa, executable):
    """
    Given int corresponding to the executable, takes all the outputs saved
    in the output directory (specified in settings) and plots/saves
    parallel bar graphs of the outputs
    @param executable The integer corresponding to which executable to run
    @return void
    """
    files = os.listdir("{}/{}".format(s.OUTPUT_DIR, isa))
    exec_name = s.EXEC_NAMES[executable]
    to_visualize = [f for f in files if exec_name in f]
    
    data = []
    for f in to_visualize:
        props = open("{}/{}/{}".format(s.OUTPUT_DIR, isa, f), "r").readlines()
        prop_values = [float(prop.split(":")[1].strip()) for prop in props]
        prop_labels = ["Conditional", "Indirect", "Latency"]
        data.append(Bar(x=prop_labels, y=prop_values, name=f.split("_")[0]))
        
    layout = Layout(
        barmode='group'
    )

    fig = Figure(data=data, layout=layout)
    plot(fig, filename="{}/{}.html".format(
        s.FIGURE_DIR.format(isa), exec_name), auto_open=False)

def create_table(isa, pred):
    """
    Given int corresponding to the branch predictor (from settings), creates
    the HTML formatted code to update the website page
    @param pred The integer corresponding to which BP to analyze
    @return void
    """
    template = """
                    <tr>
                      <td>{}</td>
                      <td>{}</td>
                      <td>{}</td>
                      <td>{}</td>
                    </tr>"""
    
    files   = os.listdir(s.OUTPUT_DIR)
    bp_name = s.BP_NAMES[pred]
    to_tabulate = [f for f in files if bp_name in f]

    full_table = []
    for f in to_tabulate:
        props = open("{}/{}/{}".format(s.OUTPUT_DIR, isa, f), "r").readlines()
        print(props)
        full_table.append(template.format(
            f.split("_")[1],
            props[0].split(":")[1].strip(),
            props[1].split(":")[1].strip(),
            props[2].split(":")[1].strip()))
    return "\n".join(full_table)
    
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
    for i in range(6):
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
        with open("{}/{}/{}_{}.txt".format(s.OUTPUT_DIR, isa,
            name, s.EXEC_NAMES[executable]), "w") as f:
            for attribute, value in zip(attributes, attribute_values):
                f.write("{} : {}\n".format(attribute, value))
                
if __name__ == "__main__":
    for executable in range(len(s.EXEC_NAMES)): # range(, 13):
        # analyze_executable("X86", executable)
        analyze_executable("ARM", executable)
        # visualize_bps("ARM", executable)

    """
    for bp in range(len(s.BP_NAMES)):
        name = s.BP_NAMES[bp]
        table = create_table(bp)
        with open("{}/{}_table.txt".format(s.TABLE_DIR, name) as f):
            f.write(table)
    """
