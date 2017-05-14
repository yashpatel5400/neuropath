/*****************************************************************
 * File: neurobranch.hh
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Always true static predictor for gem5 simulator
 * environment header.
 ****************************************************************/

#ifndef __CPU_PRED_NEUROBRANCH_PRED_HH__
#define __CPU_PRED_NEUROBRANCH_PRED_HH__

#include <vector>

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

    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history) { };

    /**
     * Looks up the given address in the branch predictor and returns
     * a true/false value as to whether it is taken.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer to any bp history state.
     * @return Whether or not the branch is taken.
     */
    bool lookup(ThreadID tid, Addr branch_addr,
				void * &bp_history) { return true; };

    /**
     * Updates the branch predictor to Not Taken if a BTB entry is
     * invalid or not found.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer to any bp history state.
     * @return Whether or not the branch is taken.
     */
    void btbUpdate(ThreadID tid, Addr branch_addr,
				   void * &bp_history) { };

    /**
     * Updates the branch predictor with the actual result of a branch.
     * @param branch_addr The address of the branch to update.
     * @param taken Whether or not the branch was taken.
     */
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed) { };

    void squash(ThreadID tid, void *bp_history)
    { assert(bp_history == NULL); }
};

#endif
