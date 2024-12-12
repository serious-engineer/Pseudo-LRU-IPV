/**************************************************************************************** /
* Title: P-LRU IPV Implementation
* Author : Abin, Manya, Krishnaprasad
* Date : 11/22/2024
* Reference: LRU RP
***************************************************************************************/

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_HH__

#include "mem/cache/replacement_policies/base.hh"

struct LRU_VariationParams;
namespace ReplacementPolicy {

    class LRU_Variation : public Base {
    public:
        LRU_Variation(const LRU_VariationParams& p); //constructor for class.
        ~LRU_Variation() = default; //destructor for class.
        void reset(const std::shared_ptr<ReplacementData>& replacement_data) const override;
        void touch(const std::shared_ptr<ReplacementData>& replacement_data) const override;
        void invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const override;
        ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;
        std::shared_ptr<ReplacementData> instantiateEntry() override;

    protected:
        typedef std::vector<int> IPVtype; //typedef a vector of elements of type int for ease of use.
        //structure to host the metadata of each cache block.
        struct LRU_VariationData : public ReplacementData {
            mutable int position;
            std::shared_ptr<IPVtype> RecencyStack; //shared pointer common for all cache blocks in a set.
            LRU_VariationData(const int assoc, std::shared_ptr<IPVtype> RecencyStack);
        };

        // Insertion and Promotion Vector
        const IPVtype IPV = { 0, 0, 1, 0, 2, 0, 2, 2, 1, 0, 5, 1, 0, 0, 5, 11 };
        //Insertion at cache block of recency 3.
        const int insertionRecency = 3;

        const int set_associativity = 16;
        int blocknum;
        IPVtype* CacheBlockPos;
    };

} // namespace ReplacementPolicy

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_HH__
