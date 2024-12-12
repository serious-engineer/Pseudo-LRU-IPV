#ifndef __CPU_PRED_CUSTOM_PRED_HH__
#define __CPU_PRED_CUSTOM_PRED_HH__

#include "base/sat_counter.hh"
#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "params/CustomBP.hh"


class CustomBP : public BPredUnit
{
public:
    CustomBP(const CustomBPParams& params);
    void uncondBranch(ThreadID tid, Addr pc, void*& bpHistory);
    void squash(ThreadID tid, void* bpHistory);
    bool lookup(ThreadID tid, Addr branchAddr, void*& bpHistory);
    void btbUpdate(ThreadID tid, Addr branchAddr, void*& bpHistory);
    void update(ThreadID tid, Addr branchAddr, bool taken, void* bpHistory,
        bool squashed, const StaticInstPtr& inst, Addr corrTarget);

private:
    void updateGH(ThreadID tid, bool taken);

    struct BPHistory {
        unsigned globalHistoryReg;
        unsigned globalPrediction;
    };

    std::vector<unsigned> globalHistory;

    unsigned globalHistoryBits;
    unsigned PredictorSize;
    unsigned PHTCtrBits;
    unsigned branchMask;
    unsigned globalHistoryMask;
    unsigned PHTThreshold;

    std::vector<SatCounter8> PHTCtr;    // PHT counters
};

#endif //__CPU_PRED_CUSTOM_PRED_HH__