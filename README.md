# README for Pseudo-LRU Cache Replacement Policy Implementation in gem5

## Project Overview
This project involves implementing a **Pseudo-Least Recently Used (Pseudo-LRU)** cache replacement policy in the **gem5 simulator** as part of coursework or personal research in **computer architecture**. The implementation is focused on enhancing the cache management system in gem5 by introducing an efficient Pseudo-LRU policy that approximates LRU while reducing the complexity associated with it.

---

## Features
- **Pseudo-LRU Algorithm**: Implements a tree-based approach to track cache line usage and select eviction candidates.
- **Integration with gem5**: The policy is integrated into gem5's cache replacement framework, replacing or complementing the existing policies.
- **Customizable Configuration**: Allows users to configure the cache replacement policy at runtime for simulation.
- **Performance Evaluation**: Provides tools to measure and compare the performance of Pseudo-LRU with other replacement policies.

---

## Prerequisites
1. **gem5 Simulator**:
   - Clone the gem5 repository:  
     ```bash
     git clone https://gem5.googlesource.com/public/gem5
     ```
   - Build gem5 with the required architecture (e.g., `X86`):  
     ```bash
     scons build/X86/gem5.opt -j<num_threads>
     ```
2. **Programming Language**:  
   - Implementation is in **C++** (gem5 source code is predominantly C++ and Python).
3. **Development Environment**:
   - Linux-based OS (Ubuntu recommended).
   - `scons` for building gem5.
4. **Knowledge Requirements**:
   - Familiarity with gem5 simulator structure.
   - Basic understanding of cache replacement policies and computer architecture concepts.

---

## Implementation Details
### 1. Code Structure
The implementation involves modifying and adding files in the following directories within the gem5 source code:
- **`src/mem/cache/replacement_policies/`**:
  - Add a new file `pseudo_lru_rp.cc` and `pseudo_lru_rp.hh` for the implementation.
- **`src/mem/cache/replacement_policies/SConscript`**:
  - Include the new replacement policy files for compilation.
- **`src/mem/cache/base_cache.hh`**:
  - Modify to support the integration of Pseudo-LRU as a configurable policy.

### 2. Pseudo-LRU Algorithm
- **Tree-based Approach**: Maintains a binary tree structure for each cache set to track usage.
  - Internal nodes track which "half" of the cache was used least recently.
  - Leaves correspond to cache lines.
  - Update the tree on every cache access and use the tree for eviction decisions.

### 3. Configuration
To enable the Pseudo-LRU policy, modify the cache configuration in the gem5 simulation script:
```python
cache = Cache(
    size='32kB',
    assoc=4,
    replacement_policy='PseudoLRU'
)
```

---

## How to Run
1. **Build gem5**: Rebuild gem5 after integrating the Pseudo-LRU code:
   ```bash
   scons build/X86/gem5.opt -j<num_threads>
   ```
2. **Simulation Script**:
   - Use or create a simulation script (`.py`) to configure the memory hierarchy and enable the new replacement policy.
3. **Run Simulation**:
   ```bash
   ./build/X86/gem5.opt configs/example/<your_script>.py
   ```

---

## Performance Evaluation
1. Compare the performance of Pseudo-LRU with other policies like LRU, FIFO, and Random by analyzing metrics such as:
   - Cache miss rate.
   - Execution time.
   - Energy consumption.
2. Use gem5 statistics (`stats.txt`) for analysis.

---

## Challenges and Considerations
- **Accuracy vs. Complexity**: Pseudo-LRU is a trade-off between the precision of true LRU and the simplicity of Random/FIFO.
- **Scalability**: Evaluate how the policy scales with increasing associativity and cache size.
- **Integration Testing**: Ensure compatibility with gem5's modular design and other cache features.

---

## Future Work
- Extend the implementation to support adaptive Pseudo-LRU policies.
- Integrate with hierarchical cache systems (e.g., L1, L2, L3).
- Evaluate performance on different workloads and architectures.

---

## Author
**Krishnaprasad Sreekumar Nair**  
- Email: ksreekum@asu.edu  
- LinkedIn: [Krishnaprasad Sreekumar Nair](https://www.linkedin.com/in/krishnaprasad-s)  

---

## License
This project is for academic and research purposes. Please credit appropriately if reused or extended.

---

Let me know if you'd like to include specific simulation results or additional implementation details!