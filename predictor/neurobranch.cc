/*****************************************************************
 * File: neurobranch.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Always true static predictor for gem5 simulator
 * environment header.
 ****************************************************************/

#include "cpu/pred/neuro.hh"

#include "base/intmath.hh"
#include "base/misc.hh"
#include "base/trace.hh"

NeuroBP::NeuroBP(const NeuroBPParams *params)
    : BPredUnit(params)
{ }

NeuroBP*
NeuroBPParams::create()
{
    return new NeuroBP(this);
}
