/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor based on the one
 * implemented in the original neural predictor paper (i.e.
 * without integrating paths): header file
 ****************************************************************/

#ifndef __CPU_PRED_NEUROBRANCH_PRED_HH__
#define __CPU_PRED_NEUROBRANCH_PRED_HH__

#include <vector>
#include <stdlib.h>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/NeuroBP.hh"

class NeuroBP : public BPredUnit
{
public:
  /**
   * Default branch predictor constructor.
   */
  NeuroBP(const NeuroBPParams *params);
  
  /**
   * Looks up the given address in the branch predictor and returns
   * a true/false value as to whether it is taken.  Also creates a
   * BPHistory object to store any state it will need on squash/update.
   * @param branch_addr The address of the branch to look up.
   * @param bp_history Pointer that will be set to the BPHistory object.
   * @return Whether or not the branch is taken.
   */
  bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);

  /**
   * Records that there was an unconditional branch, and modifies
   * the bp history to point to an object that has the previous
   * global history stored in it.
   * @param bp_history Pointer that will be set to the BPHistory object.
   */
  void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
  
  /**
   * Updates the branch predictor to Not Taken if a BTB entry is
   * invalid or not found.
   * @param branch_addr The address of the branch to look up.
   * @param bp_history Pointer to any bp history state.
   * @return Whether or not the branch is taken.
   */
  
  void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
  /**
   * Updates the branch predictor with the actual result of a branch.
   * @param branch_addr The address of the branch to update.
   * @param taken Whether or not the branch was taken.
   * @param bp_history Pointer to the BPHistory object that was created
   * when the branch was predicted.
   * @param squashed is set when this function is called during a squash
   * operation.
   */
  void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
			  bool squashed);

  /**
   * Restores the global branch history on a squash.
   * @param bp_history Pointer to the BPHistory object that has the
   * previous global branch history in it.
   */
  void squash(ThreadID tid, void *bp_history);

  unsigned getGHR(ThreadID tid, void *bp_history) const;

private:
  /** Updates global history as taken. */
  inline void updateGlobalHistTaken(ThreadID tid);

  /** Updates global history as not taken. */
  inline void updateGlobalHistNotTaken(ThreadID tid);

  /**
   * The branch history information that is created upon predicting
   * a branch.  It will be passed back upon updating and squashing,
   * when the BP can use this information to update/restore its
   * state properly.
   */
  struct BPHistory {
	unsigned globalHistory;
	bool globalPredTaken;
	bool globalUsed;
  };

  /** Number of entries in the global predictor. */
  unsigned globalPredictorSize;

  /** Global history register - used for only the outcomes of 
   *  branches as they are executed. Contains as much history as specified by
   *  globalHistoryBits. Actual number of bits used is determined by
   *  globalHistoryMask and choiceHistoryMask. */
  std::vector<unsigned> globalHistory;
  
  /** Number of bits for the global history. Determines maximum number of
	  entries in global and choice predictor tables. */
  unsigned globalHistoryBits;

  /** Mask to apply to globalHistory to access global history table.
   *  Based on globalPredictorSize.*/
  unsigned globalHistoryMask;

  /** Mask to control how much history is stored. All of it might not be
   *  used. */
  unsigned historyRegisterMask;

  /** Perceptron weights for neural branch predictor */
  unsigned perceptronCount;

  /** Perceptron theta threshold parameter empirically estimated in the
   fast neural branch predictor paper to be 1.93 * history + 14 */
  unsigned theta;
  
  /** Perceptron weights for neural branch predictor */
  std::vector<std::vector<unsigned>> weightsTable;
};

#endif
