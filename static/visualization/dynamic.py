"""
__name__ = dynamic.py
__author__ = Yash Patel
__description__ = Branch prediction visualiazation for
when performing the processing (both live and offline dumps)
for both the bimodal and rough ball
"""

from plotly.graph_objs import Scatter, Figure, Layout
from plotly.offline import plot

from predictors.bimodal import BimodalPredictor
from predictors.gshare  import GSharePredictor
import visualization.settings as s

def visualize_test(data):
    from branch import evaluate
    ns = list(range(1, s.MAX_N))
    accuracies_bimodal = [evaluate(BimodalPredictor(n=n), data) for n in ns]
    accuracies_gshare  = [evaluate(GSharePredictor(n=n), data) for n in ns]
    plot([
            Scatter(x=ns, y=accuracies_bimodal),
            Scatter(x=ns, y=accuracies_gshare)
        ], filename="output/dynamic.html")
