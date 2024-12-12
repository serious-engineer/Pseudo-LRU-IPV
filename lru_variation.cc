/**************************************************************************************** /
* Title: P-LRU IPV Implementation
* Author : Abin, Manya, Krishnaprasad
* Date : 11/22/2024
* Reference: LRU RP
***************************************************************************************/


#include "mem/cache/replacement_policies/lru_variation.hh"
#include "sim/core.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "params/LRU_Variation.hh"

namespace ReplacementPolicy {

    //constructor for class
    LRU_Variation::LRU_Variation(const LRU_VariationParams& p)
        : Base(p), set_associativity(p.numWays), blocknum(0), CacheBlockPos(nullptr)
    {
    }

    //constructor for structure
    LRU_Variation::LRU_VariationData::LRU_VariationData(const int position, std::shared_ptr<IPVtype> RecencyStack)
        : position(position), RecencyStack(RecencyStack) {}

    /*
    * Reset Function is called when a new block needs to be inserted into the Set. As per the hardcoded IPV,
    * The recency value of the newly inserted block is set to be insertionRecency which is 3 in this case.
    * Recencies of all the other blocks with recencies between insertionRecency(3) to 15(Set associativity-1) are incremented
    * by 1.
    */
    void LRU_Variation::reset(const std::shared_ptr<ReplacementData>& replacement_data) const {

        std::shared_ptr<LRU_VariationData> currentRC = std::static_pointer_cast<LRU_VariationData>(replacement_data);
        IPVtype* RStack = currentRC->RecencyStack.get();

        for (size_t i = 0; i < RStack->size(); ++i) {
            if ((RStack->at(i) >= insertionRecency) && (RStack->at(i) < (RStack->size()))) {
                RStack->at(i) = RStack->at(i) + 1; // recency of blocks with recencies between insertionRecency(3) and 15 (Rstacksize -1 or set_associativity -1) are incremented by 1.
            }
        }
        RStack->at(currentRC->position) = insertionRecency;  // Sets the Recency of new block as specified by insertion recency (which is 3 in this case)
    }

    /*
    * Touch is called when there’s a cache hit, meaning the requested data is already in the cache.
    * It updates the replacement data for the cache block to mark it as recently used.
    * Touch adjust the block’s position in a cache hit scenario according to the IPV Vector. So if say,
    * cache block with recency 5 is hit and now its recency has to be changed to 0 accroding to IPV, then cache
    * blocks with recencies 0-4 are incremented by 1 and recency of the block with cache hit is set to 0 according to IPV.
    */
    void LRU_Variation::touch(const std::shared_ptr<ReplacementData>& replacement_data) const {
        std::shared_ptr<LRU_VariationData> currentRC = std::static_pointer_cast<LRU_VariationData>(replacement_data);
        IPVtype* RStack = currentRC->RecencyStack.get();

        if (RStack->at(currentRC->position) == set_associativity) {
            return;
            //Doesn't update the recency values of a cache block with an invalid recency (i.e recency value == 16).
            //Acceptable recency values = 0-15 for set_associativity of 16.
        }

        //finds the new recency value of the block with cache hit as per IPV.
        int updatedRecency = IPV[RStack->at(currentRC->position)];

        //Stores the current recency value of the block that got hit.
        int currentRecency = RStack->at(currentRC->position);

        /*
        * Increments recency value of the blocks with recency values between new recency value and current recency value by 1.
        * If new value is 0 and current value is 3, then blocks with recency values 0,1,2 are incremented by 1.
        */
        for (size_t i = 0; i < RStack->size(); ++i) {
            if ((RStack->at(i) >= updatedRecency) && (RStack->at(i) < currentRecency)) {
                RStack->at(i) = RStack->at(i) + 1;
            }
        }
        // Update the Block with cache hit to its updated Recency value according to IPV.
        RStack->at(currentRC->position) = updatedRecency;
    }

    /*
    * By marking a block as invalid, invalidate essentially tells the replacement policy that this block should be prioritized for eviction,
    * regardless of its position in the recency order.
    * But in IPV always the last block is removed so we dont need to explicitly set any block to invalidate.
    */
    void LRU_Variation::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const {
        return;
    }

    /*
    * Here in getVictim() we get all the blocks in a set as candidates. The candidates are the blocks in a set out of which one block should be evicted.
    * Each candidate/block has the block metadata i.e. a struct of type ReplacementData which we type cast to LRU_VariationData.
    * First we make sure candidates is not empty.
    * Since the Recency stack is a shared pointer among the metadata of all the blocks, we read the recency stack of candidate[0].
    * We traverse through it to find which block has the max recency value.
    * Now this candidate block with max recency value is the victim and we return the block with max recency to the upper layers.
    */
    ReplaceableEntry* LRU_Variation::getVictim(const ReplacementCandidates& candidates) const {

        //Make sure candidates is not an empty vector.
        assert(candidates.size() > 0);

        //Picking up the Meta Data of the Block 0.
        ReplaceableEntry* victim = candidates[0];
        std::shared_ptr<LRU_VariationData> currentRC = std::static_pointer_cast<LRU_VariationData>(candidates[0]->replacementData);
        IPVtype* RStack = currentRC->RecencyStack.get();
        //Temp variables to used to figure out the max recency and index/block num at max recency.
        int max_recency = -1;
        int victim_index = -1;
        // Iterate over RStack and find the element with the maximum recency
        for (size_t i = 0; i < RStack->size(); ++i) {
            int value = (*RStack)[i];
            if (value > max_recency) {
                max_recency = value;
                victim_index = i;
            }
        }
        victim = candidates[victim_index];
        return victim;
    }

    /*
    * instantiateEntry creates and inits the replacement data for a cache block in a set-associative(16) cache.
    * Each cache block has its own metadata (LRU_VariationData) for the LRU IPV replacement policy.
    */
    std::shared_ptr<ReplacementData> LRU_Variation::instantiateEntry()
    {
        if (blocknum % set_associativity == 0) {
            //init the CacheBlockPos declared in hh file of type const IPVtype*
            CacheBlockPos = new IPVtype(set_associativity, set_associativity);
            //creating a new vector of size equal to set associativity with all values init to highest possible value (set associativity).
        }
        LRU_VariationData* BlockMetadata = new LRU_VariationData(blocknum % set_associativity, std::shared_ptr<IPVtype>(CacheBlockPos));
        ++blocknum;
        return std::shared_ptr<ReplacementData>(BlockMetadata);
    }
} // namespace ReplacementPolicy
