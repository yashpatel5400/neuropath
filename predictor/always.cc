/*****************************************************************
 * File: always.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Branch predictor that always returns True for the 
 * branching locations, i.e. a static branch predictor. Largely to
 * learn the ecosystem of gem5
 ****************************************************************/

#include "cpu/pred/always.hh"

#nclude "base/bitfield.hh"
#include "base/intmath.hh"

AlwaysBP::AlwaysBP(const AlwaysBPParams *params)
  : BPredUnit(params) { }

/**
 * Predicts whether or not the instruction is a taken branch, and the
 * target of the branch if it is taken. Always true by branch predictor
 * @param inst The branch instruction.
 * @param PC The predicted PC is passed back through this parameter.
 * @param tid The thread id.
 * @return Returns if the branch is taken or not.
 */
bool
AlwaysBP::predict(const StaticInstPtr &inst, const InstSeqNum &seqNum,
			 TheISA::PCState &pc, ThreadID tid) {
  return true;
}

AlwaysBP*
AlwaysBPParams::create() 
    return new AlwaysBP(this);
}
