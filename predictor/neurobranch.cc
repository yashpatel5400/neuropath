/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor based on the one
 * implemented in the fast neural paths branch paper.
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

  // Set up the global history mask
  // this is equivalent to mask(log2(globalPredictorSize)
  globalHistoryMask = globalPredictorSize - 1;
	
  // Set up historyRegisterMask
  historyRegisterMask = mask(globalHistoryBits);

  //Check that predictors don't use more bits than they have available
  if (globalHistoryMask > historyRegisterMask)
	fatal("Global predictor too large for global history bits!\n");

  // number of hashed perceptrons, i.e. each
  // one act as a local predictor corresponding to local history
  perceptronCount = 10;

  // Perceptron theta threshold parameter empirically determined in the
  // fast neural branch predictor paper to be 1.93 * history + 14
  theta = 1.93 * globalPredictorSize + 14;
  
  // weights per neuron (historyRegister per neuron)
  weightsTable.assign(perceptronCount,
					  std::vector<unsigned>(globalPredictorSize, 0));
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
    std::cout << "BTB UPDATE" << std::endl;
	//Update Global History to Not Taken (clear LSB)
    globalHistory[tid] &= (historyRegisterMask & ~ULL(1));
}

bool
NeuroBP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
  std::cout << "LOOKING UP" << std::endl;
  // the current perceptron weights correspond to the ones
  // being hashed from the program counter and number of perceptrons
  int curPerceptron = branch_addr % perceptronCount; 

  int y_out = 0;
  unsigned thread_history = globalHistory[tid];

  // the prediction is an indicator of the signed weighted sum
  for (int i = 0; i < globalPredictorSize; i++) {
	if (((thread_history >> i) & 1) == 1)
	  y_out += weightsTable[curPerceptron][i];
	else y_out -= weightsTable[curPerceptron][i];
  }

  std::cout << "PREDICTED" << std::endl;
  
  bool prediction = (y_out > 0);
  
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
  std::cout << "UNCONDITIONAL" << std::endl;
  
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
  std::cout << "UDPATING" << std::endl;
  
  int curPerceptron = branch_addr % perceptronCount; 
  BPHistory *history = static_cast<BPHistory *>(bp_history);
  unsigned thread_history = globalHistory[tid];
  
  int y_out = 0;
  
  for (int i = 0; i < globalPredictorSize; i++) {
	if (((thread_history >> i) & 1) == 1)
	  y_out += weightsTable[curPerceptron][i];
	else y_out -= weightsTable[curPerceptron][i];
  }

  std::cout << "CALCULATE" << std::endl;
  
  // If this is a misprediction, restore the speculatively
  // updated state (global history register and local history)
  // and update again.
  if (squashed || abs(y_out) <= theta) {
	std::cout << "IS SQUASHED" << std::endl;
	
	if (squashed) {
	  // Global history restore and update
	  globalHistory[tid] = (history->globalHistory << 1) | taken;
	  globalHistory[tid] &= historyRegisterMask;
	}
	
	if (taken) weightsTable[curPerceptron][0] += 1;
	else       weightsTable[curPerceptron][0] -= 1;
	std::cout << "UDPATING SQUASHED" << std::endl;
	
	// Have to update the corresponding weights to negatively reinforce
	// the outcome of having predicted incorrectly
	for (int i = 1; i < globalPredictorSize; i++) {
	  if (((thread_history >> i) & 1) == taken)
		weightsTable[curPerceptron][i]    += 1;
	  else weightsTable[curPerceptron][i] -= 1;
	}

	std::cout << "FINISHED SQUASHED" << std::endl;
  }
  
  std::cout << "FREEING HISTORY" << std::endl;
  // We're done with this history, now delete it.
  delete history;
  std::cout << "FFUUUUCK" << std::endl;
}

void
NeuroBP::squash(ThreadID tid, void *bp_history)
{
  std::cout << "SQUASHx" << std::endl;
  BPHistory *history = static_cast<BPHistory *>(bp_history);

  // Restore global history to state prior to this branch.
  globalHistory[tid] = history->globalHistory;

  // Delete this BPHistory now that we're done with it.
  delete history;
}

unsigned
NeuroBP::getGHR(ThreadID tid, void *bp_history) const
{
  std::cout << "GHR" << std::endl;
  return static_cast<BPHistory *>(bp_history)->globalHistory;
}

NeuroBP*
NeuroBPParams::create()
{
  return new NeuroBP(this);
}
