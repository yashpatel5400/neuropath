# Gem5 Branch Predictors

The code contained in this directory is the actual one used for the analysis performed herein. The gem5 architecture simulator is quite complex; however, the basic idea of the files is provided below. Note: to use this branch predictor in the gem5 ecosystem, simply transfer all the files in this directory (not included accuracy.py the tests/ subdirectory) to the gem5/src/cpu/pred/ directory and run scons from the gem5 root:

## Files/Descriptions
neurobranch.*: Implementation/header of the basic neural branch predictor

neuropath.*: Implementation/header of the neural path branch predictor

BranchPredictor.py: gem5-specific python "packaging" script, that allows the objects described and implemented in the C++ header and source files to be accessible by the Python config scripts run in the compiled simulators

SConscript: scons config file that adds compilation of the neurobranch and neuropath code
