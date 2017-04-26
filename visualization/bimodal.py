"""
__name__ = visualizer.py
__author__ = Yash Patel
__description__ = Branch prediction visualiazation for
when performing the processing (both live and offline dumps)
"""

from plotly.graph_objs import Scatter, Figure, Layout
from plotly.offline import plot

from predictors.bimodal import BimodalPredictor
import visualization.settings as s

def visualize_test(data):
    from branch import evaluate
    ns = list(range(1, s.MAX_N))
    accuracies = [evaluate(BimodalPredictor(n=n), data) for n in ns]
    plot([Scatter(x=ns, y=accuracies)], filename="output/bimodal.html")
