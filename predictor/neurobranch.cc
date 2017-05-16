/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor based on the one
 * implemented in the original neural predictor paper (i.e.
 * without integrating paths).
 ****************************************************************/

#include "cpu/pred/neurobranch.hh"

#include<iostream>
#include "base/bitfield.hh"
#include "base/intmath.hh"

NeuroBP::NeuroBP(const NeuroBPParams *params)
  : BPredUnit(params),
	globalPredictorSize(params->globalPredictorSize),
	globalHistory(params->numThreads, 0),
	globalHistoryBits(ceilLog2(params->globalPredictorSize))
{  
  if (!isPowerOf2(globalPredictorSize)) {
	fatal("Invalid global predictor size!\n");
  }
	
  // Set up historyRegisterMask
  historyRegisterMask = mask(globalHistoryBits);

  // number of hashed perceptrons, i.e. each
  // one act as a local predictor corresponding to local history
  perceptronCount = 20;

  // Perceptron theta threshold parameter empirically determined in the
  // fast neural branch predictor paper to be 1.93 * history + 14
  theta = 1.93 * globalPredictorSize + 14;
  
  // weights per neuron (historyRegister per neuron)
  weightsTable.assign(perceptronCount,
					  std::vector<unsigned>(globalPredictorSize + 1, 0));
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
  unsigned thread_history = globalHistory[tid];
  
  // the prediction is an indicator of the signed weighted sum
  int y_out = weightsTable[curPerceptron][0];
  for (int i = 1; i <= globalPredictorSize; i++) {
	if ((thread_history >> (i - 1)) & 1)
	  y_out += weightsTable[curPerceptron][i];
	else y_out -= weightsTable[curPerceptron][i];
  }
  
  bool prediction = (y_out >= 0);
  
  // Create BPHistory and pass it back to be recorded.
  BPHistory *history       = new BPHistory;
  history->globalHistory   = globalHistory[tid];
  history->globalPredTaken = prediction;
  bp_history = (void *)history;
  
  return prediction;
}

void
NeuroBP::uncondBranch(ThreadID tid, Addr pc, void * &bp_history)
{  
  // Create BPHistory and pass it back to be recorded.
  BPHistory *history       = new BPHistory;
  history->globalHistory   = globalHistory[tid];
  history->globalPredTaken = true;
  history->globalUsed      = true;
  bp_history = static_cast<void *>(history);
  updateGlobalHistTaken(tid);
}

void
NeuroBP::update(ThreadID tid, Addr branch_addr, bool taken,
				void *bp_history, bool squashed)
{
  assert(bp_history);
  
  int curPerceptron = branch_addr % perceptronCount; 
  unsigned thread_history = globalHistory[tid];
  
  // the prediction is an indicator of the signed weighted sum
  int y_out = weightsTable[curPerceptron][0];
  for (int i = 1; i <= globalPredictorSize; i++) {
	if ((thread_history >> (i - 1)) & 1)
	  y_out += weightsTable[curPerceptron][i];
	else y_out -= weightsTable[curPerceptron][i];
  }
  
  // If this is a misprediction, restore the speculatively
  // updated state (global history register and local history)
  // and update again.
  if (squashed || (abs(y_out) <= theta)) {	
	if (taken) weightsTable[curPerceptron][0] += 1;
	else       weightsTable[curPerceptron][0] -= 1;
	
	// Have to update the corresponding weights to negatively reinforce
	// the outcome of having predicted incorrectly
	for (int i = 1; i < globalPredictorSize; i++) {
	  if (((thread_history >> (i - 1)) & 1) == taken)
		weightsTable[curPerceptron][i]    += 1;
	  else weightsTable[curPerceptron][i] -= 1;
	}
  }
  
  // Global history restore and update
  globalHistory[tid] = (globalHistory[tid] << 1) | taken;
  globalHistory[tid] &= historyRegisterMask;
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
