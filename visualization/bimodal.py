"""
__name__ = visualizer.py
__author__ = Yash Patel
__description__ = Branch prediction visualiazation for
when performing the processing (both live and offline dumps)
"""

from plotly.graph_objs import Scatter, Figure, Layout

import visualization.settings as s

def visualize_test():
    from branch import evaluate
    ns = list(range(s.MAX_N))
    accuracies = [evaluate(BimodalPredictor(n=n)) for n in ns]
    plot([Scatter(x=ns, y=accuracies)])
