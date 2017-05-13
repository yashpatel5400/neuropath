/*****************************************************************
 * File: always.hh
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Branch predictor that always returns True for the 
 * branching locations, i.e. a static branch predictor header
 ****************************************************************/
#ifndef __CPU_PRED_ALWAYS_PRED_HH__
#define __CPU_PRED_ALWAYS_PRED_HH__

#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/BiModeBP.hh"

/*****************************************************************
 * Implements an always true branch predictor, i.e. simplest static
 * branch predictor for integration into gem5 environment 
 ****************************************************************/
class AlwaysBP : public BPredUnit {
public:
  AlwaysBP(const AlwaysBPParams *params);
  bool predict(const StaticInstPtr &inst, const InstSeqNum &seqNum,
			   TheISA::PCState &pc, ThreadID tid);
private:
}

#endif
