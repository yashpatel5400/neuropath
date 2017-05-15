/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor based on the one
 * implemented in the fast neural paths branch paper.
 ****************************************************************/

#include "cpu/pred/neurobranch.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

NeuroBP::NeuroBP(const NeuroBPParams *params)
  : BPredUnit(params),
	globalPredictorSize(params->globalPredictorSize),
	globalCtrBits(params->globalCtrBits),
	globalHistory(params->numThreads, 0),
	globalHistoryBits(
					  ceilLog2(params->globalPredictorSize) >
					  ceilLog2(params->globalPredictorSize) :)
{  
  if (!isPowerOf2(globalPredictorSize)) {
	fatal("Invalid global predictor size!\n");
  }

  //Setup the array of counters for the global predictor
  globalCtrs.resize(globalPredictorSize);

  for (int i = 0; i < globalPredictorSize; ++i)
	globalCtrs[i].setBits(globalCtrBits);

  // Set up the global history mask
  // this is equivalent to mask(log2(globalPredictorSize)
  globalHistoryMask = globalPredictorSize - 1;
	
  //Set up historyRegisterMask
  historyRegisterMask = mask(globalHistoryBits);

  //Check that predictors don't use more bits than they have available
  if (globalHistoryMask > historyRegisterMask) {
	fatal("Global predictor too large for global history bits!\n");
  }

  // number of hashed perceptrons, i.e. each
  // one act as a local predictor corresponding to local history
  perceptronCount = 10;
    
  // weights per neuron (historyRegister per neuron)
  unsigned **weightsTable = new unsigned[perceptronCount][globalPredictorSize];             
  for (int i = 0; i < perceptronCount; i++) {
	for (int j = 0; j < globalPredictorSize; j++)
	  // initialize each of the perceptrons to have to bias and/or
	  // weights towards any of the branching statements (empty neurons)
	  weightsTable[i][j] = 0;
  }
}

inline
void
NeuroBP::updateGlobalHistTaken(ThreadID tid)
{
  globalHistory[tid] = (globalHistory[tid] << 1) | 1;
  globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

inline
void
NeuroBP::updateGlobalHistNotTaken(ThreadID tid)
{
  globalHistory[tid] = (globalHistory[tid] << 1);
  globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

void
NeuroBP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    //Update Global History to Not Taken (clear LSB)
    globalHistory[tid] &= (historyRegisterMask & ~ULL(1));
}

bool
NeuroBP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
  // the current perceptron weights correspond to the ones
  // being hashed from the program counter and number of perceptrons
  int curPerceptron = branch_addr % perceptronCount; 
  int weightSumPrediction = 0;
  
  // the prediction is an indicator of the signed weighted sum
  for (int i = 0; i < globalPredictorSize; i++) {
	weightSumPrediction += weightsTable[curPerceptron][i] *
	  globalHistory[tid][i];
  }

  bool prediction = bool(weightSumPrediction > 0);
  
  // Create BPHistory and pass it back to be recorded.
  BPHistory *history = new BPHistory;
  history->globalHistory = globalHistory[tid];
  history->globalPredTaken = prediction;
  bp_history = (void *)history;
  
  if (prediction) updateGlobalHistTaken(tid);
  else updateGlobalHistNotTaken(tid);
  return prediction;
}

void
NeuroBP::uncondBranch(ThreadID tid, Addr pc, void * &bp_history)
{
  // Create BPHistory and pass it back to be recorded.
  BPHistory *history = new BPHistory;
  history->globalHistory = globalHistory[tid];
  history->globalPredTaken = true;
  history->globalUsed = true;
  bp_history = static_cast<void *>(history);

  updateGlobalHistTaken(tid);
}

void
NeuroBP::update(ThreadID tid, Addr branch_addr, bool taken,
				void *bp_history, bool squashed)
{
  assert(bp_history);

  int curPerceptron = branch_addr % perceptronCount; 
  BPHistory *history = static_cast<BPHistory *>(bp_history);

  // Have to update the corresponding weights to negatively reinforce
  // the outcome of having predicted incorrectly
  for (int i = 1; i < globalPredictorSize; i++) {
	if (weightsTable[curPerceptron][i] == globalHistory[tid][i])
	  weightsTable[curPerceptron][i] += 1;
	else weightsTable[curPerceptron][i] -= 1;
  }
	
  // If this is a misprediction, restore the speculatively
  // updated state (global history register and local history)
  // and update again.
  if (squashed) {
	// Global history restore and update
	globalHistory[tid] = (history->globalHistory << 1) | taken;
	globalHistory[tid] &= historyRegisterMask;

	weightsTable[curPerceptron][0] -= 1;
	return;
  }

  // Update the counters with the proper
  // resolution of the branch. Histories are updated
  // speculatively, restored upon squash() calls, and
  // recomputed upon update(squash = true) calls,
  // so they do not need to be updated.
  unsigned global_predictor_idx =
	history->globalHistory & globalHistoryMask;
  if (taken) {
	weightsTable[curPerceptron][0] += 1;
	globalCtrs[global_predictor_idx].increment();
  } else {
	weightsTable[curPerceptron][0] -= 1;
	globalCtrs[global_predictor_idx].decrement();
  }

  // We're done with this history, now delete it.
  delete history;
}

void
NeuroBP::squash(ThreadID tid, void *bp_history)
{
  BPHistory *history = static_cast<BPHistory *>(bp_history);

  // Restore global history to state prior to this branch.
  globalHistory[tid] = history->globalHistory;

  // Delete this BPHistory now that we're done with it.
  delete history;
}

unsigned
NeuroBP::getGHR(ThreadID tid, void *bp_history) const
{
  return static_cast<BPHistory *>(bp_history)->globalHistory;
}

NeuroBP*
NeuroBPParams::create()
{
  return new NeuroBP(this);
}
