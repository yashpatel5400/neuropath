# neuropath
Branch prediction has become an essential part of the CPU pipeline, as more and more
tricks are integrated into their designs to increase ILP (instruction-level
parallelism). Specifically, branch predictors are responsible for pre-fetching
instructions for decoding/execution in conditional branch locations, i.e. points
in the code where it is impossible to know which instruction to fetch next. In
other words, the CPU guesses the outcome of the branch and fetches instructions
accordingly, hoping to avoid the seeming possible of needing to wait for the
conditional branch to finish executing to fetch/process further instructions.
Modern branch predictors typically integrate global history and local history, which
respectively correspond to applying the result of previous branch outcomes in the
overall program and at particular values for the program counter (i.e. by line
number in the code). However, with the uprising of neural nets, it was
unavoidable that they would too be tested in this regime.

The main issue with neural net implementations is the latency, i.e. how long it
would take for the prediction to occur relative to the time spent. Despite maybe
a few percentage points extra, perceptron use was largely relegated as not likely
to be adopted, seeing as modern methods already achieve accuracies of upwards of
90-95%. However, recent research investigations have revealed otherwise, citing
that perceptrons may in fact be comparable in accuracy and latency to those methods
that are currently most widespread. This code sought to clarify/explore that claim
in the Gem5 CPU simulator environment for an X86 ISA.

For COS 583: Great Moments in Computing at Princeton University

Results available at: http://fast-caverns-95520.herokuapp.com/

## Conclusions
### Conditional
Conditional represents the total number of conditional branches predicted incorrectly by the predictor. The following were the main takeaways from the conditional graphs:
* LTAGE branch predictor almost always annihilated the rest of the predictors in sheer performance
* NeuroBP (standard neural predictor) is quite heavily correlated with the LocalBP predictor. This makes sense, since the two base weight predictions based on past outcomes at a given branch
* NeuroPathBP performance seems to generally be quite mediocre (worse than NeuroBP) - focus is on large speedup (not implemented due to needing parallelism)
  * Especially programs where the path may not be significant seems to throw off the NeuroPathBP predictor (i.e. sorting scripts)
  * Performs relatively fine on the matrix multiplications, where presumably the traces through successive rows being similar is captured
* Very similar outputs for the integer matrix multiplication result vs. real matrices

### Indirect
Indirect represents the . The following were the main takeaways from the indirect graphs:
* 

### Latency
Latency is simply how long the branch predictor took to run the program. Having held the environment relatively constant (i.e. same processes running in the computer background), this time rougly corresponds to how long the BP takes to make its predictions. The following were the main takeaways from the latency graphs:
* The neural branch predictor and fast-path predictor often had at least significantly more latency of the others
  * Fast-path predictor often had more latency than the standard, even though its primary advantage was supposedly speedly, likely due to lack of the parallel implementation (though similar effects may appear in other BPs as well)
* All of the rest were relatively comparable, meaning that LTAGE is still quite handily the superior branch predictor, given its performance and latency

## Future Steps:
(1) Consider other ISAs

(2) Parallelize codebase