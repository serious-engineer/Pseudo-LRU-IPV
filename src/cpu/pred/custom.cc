/**************************************************************************************** /
* Title: 6 bit OR Implementation of a Custom Branch Predictor
* Author : Abin, Manya, Krishnaprasad
* Date : 10/28/2024
* Reference: Bimode Branch Predictor Implementation
***************************************************************************************/
#include "cpu/pred/custom.hh"
#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/Fetch.hh"

CustomBP::CustomBP(const CustomBPParams& params)
    : BPredUnit(params),
    globalHistory(params.numThreads, 0),
    globalHistoryBits(params.globalHistoryBits),
    PredictorSize(params.PredictorSize),
    PHTCtrBits(params.PHTCtrBits),
    PHTCtr(PredictorSize, SatCounter8(PHTCtrBits))
{
    if (static_cast<int>(PredictorSize) < static_cast<int>(1 << globalHistoryBits))
        fatal("Predictor Size is too small for the specified global History bits. It must be atleast 2^globalHistoryBits. \n");

    globalHistoryMask = mask(globalHistoryBits);  // Creates a mask with last GlobalHistoryBits bits as 1
    branchMask = mask(globalHistoryBits);
    PHTThreshold = (ULL(1) << (PHTCtrBits - 1)) - 1; //Finds threshold value as 2^(n-1) - 1. If ctr value is less than or equal to threshold, then NOT TAKEN.
}

/*
 * In case of an unconditional branch, the global history of the thread is updated to
 * show the latest branch as Taken since there are no conditions for branching.
*/
void CustomBP::uncondBranch(ThreadID tid, Addr pc, void*& bpHistory)
{
    BPHistory* history = new BPHistory;
    history->globalHistoryReg = globalHistory[tid] & globalHistoryMask; //assign the global history of the thread to the newly created location
    history->globalPrediction = true;

    bpHistory = static_cast<void*>(history); // cast the pointer containing address of the history which has
                                             // the Global history of the thread to the passed down pointer.

    updateGH(tid, true); // Updates global history of the thread as Taken.
}

/*
 * If a branch is predicted as taken by the branch predictor, then while trying to
 * continue execution, if the BTB do not contain the target address for that branch,
 * the execution is stalled. Here, the global History will be updated to show the recent
 * branch prediction as not taken. The function btbUpdate will set LSB of the global
 * history register as 0.
*/
void CustomBP::btbUpdate(ThreadID tid, Addr branchAddr, void*& bpHistory)
{
    globalHistory[tid] &= (globalHistoryMask & (~ULL(1)));  //  0x000000000000003F & 0xFFFFFFFFFFFFFFFE = 0x000000000000003E. This is ANDed with Global history so that LSB becomes 0.
}

/*
 * When a conditional branch instruction is encountered, lookup function is called to
 * get a prediction based on thread ID, branch address and the Global History. Here
 * the branch address is ORed with the global history corresponding to the thread to
 * get the index of the Pattern History table which stores saturating counter specific
 * to the pattern we get after OR operation of branch address and global history. The
 * value of the counter is then compared with the PHTThreshold (1 in case of 2 bit Ctr)
 * and then a prediction is made based on that. This value then returned to the function
 * call.
*/
bool CustomBP::lookup(ThreadID tid, Addr branchAddr, void*& bpHistory)
{
    unsigned shiftedBranchAddr;
    unsigned PHTIdx;
    unsigned patternCtr;
    bool Prediction;
    shiftedBranchAddr = (branchAddr >> instShiftAmt) & branchMask; // Shifts out the Last instShiftAmt bits from the branch address
    PHTIdx = (shiftedBranchAddr | globalHistory[tid]) & globalHistoryMask; // finds Pattern history table index by OR-ing Global History
                                                                           // and BranchAddress after accounting for shiting
    patternCtr = PHTCtr[PHTIdx]; // Gets the counter value corresponding to the pattern.
    Prediction = patternCtr > PHTThreshold; // Ctr value is compared with PHTThreshold (1 in case of 2 bit Ctr).
                                            // Prediction will be assigned True(Taken) if Ctr value is higher than threshold.

    //Creates a new pointer for storing global history and assigns it with global history of the thread.
    BPHistory* history = new BPHistory;
    history->globalHistoryReg = globalHistory[tid];
    history->globalPrediction = Prediction;
    bpHistory = static_cast<void*>(history); //The newly created pointer is casted on to the passed down pointer.

    updateGH(tid, Prediction); // Updates the global history of the thread with the prediction.
    return Prediction;
}

/*
 * The update function is called while executing instructions of the predicted branch after the
 * branch condition is evaluated. There are two scenarios when this is called: One is where it
 * correctly predicted the branch and other where it has mispredicted the same. If there has been
 * a misprediction, then squashing occurs where it invalidates the speculatively executed instructions
 * and restarts execution of the correct branch. In this case, global history of the thread (which
 * contains the prediction as the recent history) will need to be restored to the older known good
 * value stored in the memory allocated during lookup and then update global history of the thread
 * with the correct branch path (Taken or Not taken).
 * If the branch has been predicted correctly then, corresponding pattern counter needs
 * to be updated accordingly.
*/

void CustomBP::update(ThreadID tid, Addr branchAddr, bool taken, void* bpHistory,
    bool squashed, const StaticInstPtr& inst, Addr corrTarget)
{
    unsigned shiftedBranchAddr;
    unsigned PHTIdx;

    BPHistory* history = static_cast<BPHistory*>(bpHistory); //Casts the passed down pointer containing Global History stored in the lookup stage to a pointer of BPHistory type

    if (squashed)
    {
        globalHistory[tid] = history->globalHistoryReg; //Using the history without predicted value to prepare it for updating it with correct branching result.
        updateGH(tid, taken); // Global history is updated with the correct result of branching.
        return;
    }
    shiftedBranchAddr = (branchAddr >> instShiftAmt) & branchMask;
    PHTIdx = (shiftedBranchAddr | history->globalHistoryReg) & globalHistoryMask; //Uses global history stored in lookup stage to calculate Index so that correct counter are updated
    taken ? PHTCtr[PHTIdx]++ : PHTCtr[PHTIdx]--; // Increments the corresponding counter if the branch was taken or decrement it if it is not taken.

    delete history; // Deletes the history pointer and unallocates the memory as it is no longer needed.
}

/*
 * When a branch is mispredicted, Global history is restored with previously stored
 * history when the lookup function was called for that branch address. The history
 * pointer is deleted after it is used for restoring the global history for the thread
 * in order to avoid issues of memory leaks.
*/

void CustomBP::squash(ThreadID tid, void* bpHistory)
{
    BPHistory* history = static_cast<BPHistory*>(bpHistory);
    globalHistory[tid] = history->globalHistoryReg;     // Global history(which contains prediction) of the thread is restored with history stored in lookup stage (do no contain prediction).

    delete history;
}

/*
 * Function to update Global History Register for a thread. Accepts Thread ID and a boolean variable
 * as arguments which will then append zero to LSB if boolean variable is False and 1 to LSB as 1 if
 * boolean variable is True. Global history Mask (as per the global history bits provided) is applied
 * before the function ends its execution.
*/
inline void CustomBP::updateGH(ThreadID tid, bool taken)
{
    globalHistory[tid] = taken ? globalHistory[tid] << 1 | 1 : globalHistory[tid] << 1;
    globalHistory[tid] = globalHistory[tid] & globalHistoryMask;
    return;
}