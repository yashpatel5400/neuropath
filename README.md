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

Results available at: http://fast-caverns-95520.herokuapp.com/table.html

## Progress Steps:
(3) Consider other ISAs

(4) Complete analyses/publish to site

(5) Parallelize codebase
