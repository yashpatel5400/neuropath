/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor based on the one
 * implemented in the fast neural paths branch paper: header file.
 ****************************************************************/

#ifndef __CPU_PRED_NEUROPATH_PRED_HH__
#define __CPU_PRED_NEUROPATH_PRED_HH__

#include <vector>
#include <stdlib.h>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/NeuroPathBP.hh"

class NeuroPathBP : public BPredUnit
{
public:
  /**
   * Default branch predictor constructor.
   */
  NeuroPathBP(const NeuroPathBPParams *params);
  
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
  /**
   * Updates the global path tracking instance variable to include
   * newly encountered branch instruction
   * @param branch_addr Address object containing memory location obj
   */
  void inline updatePath(Addr branch_addr);

  /**
   * Updates the corresponding weight parameter w/ saturation factor
   * @param weight Current value of weight to be updated
   * @param inc Whether the weight is to be incremented or decremented
   */
  unsigned saturatedUpdate (unsigned weight, bool inc);
  
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
  
  /** Global history register, denoted G in this version to match the
   *  notation from the paper. Contains as much history as specified by
   *  globalHistoryBits. Actual number of bits used is determined by
   *  globalHistoryMask and choiceHistoryMask. */
  std::vector<unsigned> G;

  /** Speculative global history register, denoted SG in this version 
   *  to match notation from the paper. Contains prediction history for
   *  the same size as that of the true history global register, i.e.
   *  globalHistoryBits. Actual number of bits used is determined by
   *  globalHistoryMask and choiceHistoryMask. */
  std::vector<unsigned> SG;
  
  /** Running total computing the perceptron output steps
	  in the future (in reality). */
  std::vector<unsigned> R;
  
  /** Speculative running total computing the perceptron output steps
	  in the future (in reality). */
  std::vector<unsigned> SR;
  
  /** History of the path the CPU has travelled through the program trace,
      i.e. the previous h branch instruction addresses. These are used for
	  prediction, i.e. multiple inputs. */
  std::vector<unsigned> path;
  
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

  /** Saturated value of the maximum weight on a branch */
  unsigned max_weight;

  /** Saturated value of the minimum weight on a branch */
  unsigned min_weight;

  /** Perceptron weights for neural branch predictor */
  std::vector<std::vector<unsigned>> weightsTable;
};

#endif
