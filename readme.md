# Production-Grade Byte Pair Encoding (BPE) Tokenizer Infrastructure

## High-Performance Tokenization Runtime & Training System in Modern C++

A systems-engineering focused implementation of Byte Pair Encoding (BPE) tokenizer infrastructure designed around production-oriented runtime constraints, memory locality, scalable merge execution, serialization-safe vocabulary representation, and cache-aware tokenizer lookup systems.

This repository explores tokenizer infrastructure as a low-level systems problem rather than a purely NLP-oriented preprocessing step.

The implementation draws architectural inspiration from tokenizer systems commonly used in modern large language model ecosystems, including GPT-family and Claude-family inference pipelines.

---

# Table of Contents

- [High-level Overview](#high-level-overview)
- [Why Tokenizers Matter in Modern LLM Systems](#why-tokenizers-matter-in-modern-llm-systems)
- [How BPE Works](#how-bpe-works)
- [Architecture Overview](#architecture-overview)
- [Core Systems Design Decisions](#core-systems-design-decisions)
- [Memory Optimization Strategies](#memory-optimization-strategies)
- [Radix Tree / Trie Design](#radix-tree--trie-design)
- [Merge Engine Design](#merge-engine-design)
- [Serialization System](#serialization-system)
- [Training Pipeline](#training-pipeline)
- [Runtime Token Lookup](#runtime-token-lookup)
- [Cache Locality & Performance Considerations](#cache-locality--performance-considerations)
- [Scalability Considerations](#scalability-considerations)
- [Example Merge Walkthrough](#example-merge-walkthrough)
- [Example Tokenization Flow](#example-tokenization-flow)
- [Data Structures Used](#data-structures-used)
- [Time Complexity Analysis](#time-complexity-analysis)
- [Production-Oriented Design Choices](#production-oriented-design-choices)
- [Current Limitations](#current-limitations)
- [Future Improvements](#future-improvements)
- [Build Instructions](#build-instructions)
- [Example Usage](#example-usage)
- [Benchmark / Performance Section](#benchmark--performance-section)
- [Repository Structure](#repository-structure)
- [Architecture Diagram Placeholder](#architecture-diagram-placeholder)
- [References to Modern LLM Tokenization Concepts](#references-to-modern-llm-tokenization-concepts)
- [Conclusion](#conclusion)

---

# High-level Overview

This project implements a production-oriented Byte Pair Encoding tokenizer runtime and training engine in modern C++.

Unlike educational BPE implementations that primarily focus on merge correctness, this system explores tokenizer infrastructure from the perspective of:

- runtime efficiency
- memory locality
- scalable merge execution
- allocator-aware systems design
- serialization-safe vocabulary representation
- low-level data structure engineering
- inference-time lookup performance
- incremental state updates
- cache-aware token processing

The implementation includes:

- Custom memory pool allocator
- Intrusive doubly linked token streams
- Incremental pair-frequency maintenance
- Lazy priority queue invalidation
- Patricia/Radix vocabulary tree
- Prefix-compressed token storage
- Binary serialization/deserialization
- O(1) merge rewiring semantics
- Index-based references instead of raw pointers
- Runtime stale-state validation
- Cache-aware contiguous memory layouts

The project treats tokenization as infrastructure.

---

# Why Tokenizers Matter in Modern LLM Systems

In modern large language model systems, tokenization is not merely a preprocessing stage.

Tokenizer infrastructure directly affects:

- inference latency
- training throughput
- memory bandwidth consumption
- context window efficiency
- vocabulary scalability
- streaming decode performance
- batching efficiency
- distributed preprocessing cost
- serialization footprint
- tokenizer startup latency

Large-scale inference systems may tokenize billions of text fragments per day.

At that scale:

- cache misses matter
- allocator fragmentation matters
- pointer chasing matters
- serialization overhead matters
- memory locality matters
- trie traversal performance matters
- branch prediction matters

This repository explores those systems-level considerations explicitly.

---

# How BPE Works

Byte Pair Encoding iteratively merges the most frequently occurring adjacent token pairs.

The algorithm begins with a base vocabulary consisting of individual byte-level symbols.

Example:

```text
Input:
"lower"

Initial byte tokens:
l o w e r
```

Frequent adjacent pairs are identified:

```text
(l, o)
(o, w)
(w, e)
(e, r)
```

The most frequent pair globally is merged:

```text
(lo)
```

Subsequent iterations continue:

```text
(lo, w)
(low)
(low, er)
(lower)
```

Over time, frequently co-occurring sequences become compact subword units.

This implementation optimizes:

- pair tracking
- merge execution
- token stream rewiring
- vocabulary lookup
- runtime token reconstruction
- pair-frequency maintenance

rather than merely implementing merge correctness.

---

# Architecture Overview

```text
                    ┌──────────────────────┐
                    │     Input Corpus     │
                    └──────────┬───────────┘
                               │
                               ▼
               ┌────────────────────────────┐
               │ Token Sequence Construction│
               │  Intrusive DLL Allocation  │
               └──────────┬─────────────────┘
                          │
                          ▼
            ┌───────────────────────────────┐
            │ Pair Frequency Aggregation    │
            │ Incremental Pair Tracking     │
            └──────────┬────────────────────┘
                       │
                       ▼
             ┌─────────────────────────────┐
             │ Max Heap / Priority Queue   │
             │ Lazy Invalidation Strategy  │
             └──────────┬──────────────────┘
                        │
                        ▼
           ┌────────────────────────────────┐
           │ Incremental Merge Engine       │
           │ O(1) DLL Rewiring              │
           └──────────┬─────────────────────┘
                      │
                      ▼
         ┌──────────────────────────────────┐
         │ Prefix-Compressed Radix Tree     │
         │ Runtime Vocabulary Lookup        │
         └──────────┬───────────────────────┘
                    │
                    ▼
         ┌──────────────────────────────────┐
         │ Binary Serialization Layer       │
         │ Runtime Vocabulary Artifact      │
         └──────────────────────────────────┘
```

---

# Core Systems Design Decisions

## 1. Index-Based References Instead of Raw Pointers

The implementation avoids raw pointer ownership across tokenizer structures.

Instead:

```cpp
uint_32 PrevNodeIndex;
uint_32 NextNodeIndex;
```

This improves:

- serialization portability
- allocator flexibility
- memory compaction safety
- deterministic relocation
- debugging simplicity
- pointer invalidation safety

This also enables contiguous pool-backed storage.

---

## 2. Intrusive Token Stream Representation

Each token sequence is represented as an intrusive doubly linked structure.

```text
[token] <-> [token] <-> [token]
```

This enables:

- O(1) merge rewiring
- local neighborhood updates
- efficient adjacent pair maintenance
- incremental frequency updates

Unlike vector-shifting approaches, merges do not require large-scale memory movement.

---

## 3. Lazy Heap Invalidation

The priority queue intentionally allows stale entries.

Instead of eagerly repairing heap state after every frequency update:

```text
Heap Top
   ↓
Validate Against Canonical Frequency Map
   ↓
Discard If Stale
```

This dramatically reduces heap maintenance overhead.

This pattern is commonly used in high-performance scheduling and graph systems.

---

# Memory Optimization Strategies

## Custom Memory Pool Allocator

The tokenizer uses contiguous pool allocation:

```cpp
template <typename T>
struct MemoryPool {
    vector<T> pool;
};
```

Key properties:

- contiguous allocation
- stable indices
- improved spatial locality
- allocator amortization
- reduced fragmentation
- deterministic memory growth

### Benefits

| Strategy | Benefit |
|---|---|
| Pool allocation | Fewer heap allocations |
| Stable indices | Safer rewiring |
| Contiguous storage | Better cache locality |
| Reserve-based growth | Reduced reallocations |
| No ownership graph | Simpler serialization |

---

## Cache-Friendly Token Streams

Token nodes are compact:

```cpp
struct TokenNode {
    uint_32 TokenId;
    uint_32 Count;
    uint_32 PrevNodeIndex;
    uint_32 NextNodeIndex;
};
```

The structure is intentionally flat and cache-friendly.

This minimizes:

- pointer chasing
- allocator metadata overhead
- cache-line inefficiency

---

# Radix Tree / Trie Design

Vocabulary lookup uses a Patricia/Radix compressed trie.

```text
(root)
  ├── th
  │    └── e
  ├── to
  └── tok
       └── en
```

## Why Radix Compression?

Traditional tries waste memory.

Compressed radix nodes reduce:

- node count
- branch depth
- traversal overhead
- memory fragmentation

Each node stores:

```cpp
string NodeString;
uint_32 ChildIdx[256];
```

This implementation performs:

- partial-match splitting
- prefix compression
- branch compaction
- runtime token reconstruction

---

## Split-Based Node Compression

During insertion:

```text
Existing Node:
"token"

Insert:
"topology"
```

The node splits:

```text
"to"
 ├── "ken"
 └── "pology"
```

This significantly reduces trie depth.

---

# Merge Engine Design

The merge engine is the core runtime component.

## Merge Lifecycle

```text
1. Extract Highest Frequency Pair
2. Validate Heap Entry
3. Create New Token
4. Rewire DLL Nodes
5. Update Neighbor Pair Frequencies
6. Push Incremental Updates
7. Continue Training
```

---

## O(1) Merge Rewiring

Adjacent token merges operate through intrusive pointer rewiring.

Before:

```text
[A] <-> [B] <-> [C]
```

After merging A+B:

```text
[AB] <-> [C]
```

No vector shifting.

No sequence copying.

No allocator churn.

Only local pointer/index updates.

---

## Incremental Pair Frequency Maintenance

Instead of recomputing pair statistics globally after every merge:

Only affected neighboring pairs are updated.

```text
Prev + Left  → invalidated
Left + Right → merged
Right + Next → invalidated
Prev + Merged → inserted
Merged + Next → inserted
```

This dramatically improves scalability.

---

# Serialization System

The tokenizer supports binary serialization for runtime deployment.

Implemented via:

- contiguous binary buffers
- explicit byte writes
- deterministic structure layout
- runtime deserialization

Example serializer pattern:

```cpp
writeBytes(&node.TokenID, sizeof(node.TokenID));
```

---

## Serialized Components

The runtime artifact stores:

- vocabulary size
- token byte sequences
- radix tree nodes
- child index tables
- token metadata

This enables:

- offline training
- runtime loading
- deployment portability
- inference-time startup reduction

---

## Serialization Safety

The implementation avoids:

- raw pointer persistence
- allocator-specific layouts
- non-portable ownership graphs

This makes runtime restoration deterministic.

---

# Training Pipeline

```text
Corpus
  ↓
Unique String Aggregation
  ↓
DLL Token Stream Construction
  ↓
Pair Frequency Aggregation
  ↓
Priority Queue Construction
  ↓
Incremental Merge Iterations
  ↓
Vocabulary Expansion
  ↓
Trie Construction
  ↓
Binary Serialization
```

---

# Runtime Token Lookup

Runtime token lookup operates through the radix vocabulary tree.

Lookup process:

```text
Input Bytes
   ↓
Traverse Prefix Node
   ↓
Compressed Match Validation
   ↓
Child Transition
   ↓
Longest Match Resolution
```

The lookup system prioritizes:

- prefix compression
- low traversal depth
- deterministic branching
- contiguous node access

---

# Cache Locality & Performance Considerations

Modern tokenizer performance is heavily memory-bound.

This implementation explicitly considers:

## Spatial Locality

Pool-backed vectors keep nodes contiguous.

This improves:

- prefetch efficiency
- cache line utilization
- branch locality

---

## Reduced Allocator Pressure

Frequent heap allocation is avoided.

Benefits:

- lower fragmentation
- improved deterministic behavior
- reduced allocator synchronization cost

---

## Stable Memory Topology

Index-based references avoid pointer invalidation.

This improves:

- relocation safety
- serialization simplicity
- debugging clarity

---

## Reduced Reprocessing

Incremental updates avoid full rescans.

This significantly lowers:

- corpus traversal overhead
- unnecessary frequency recomputation
- runtime bandwidth consumption

---

# Scalability Considerations

The implementation is designed with scaling behavior in mind.

## Current Design Goals

- scalable merge iteration
- efficient pair-frequency maintenance
- low-overhead token rewiring
- bounded heap operations
- contiguous runtime memory

---

## Future Scale Targets

Potential extensions include:

- SIMD token scanning
- multi-threaded pair aggregation
- lock-free merge scheduling
- mmap-based vocabulary loading
- streaming tokenization pipelines
- NUMA-aware memory placement
- compressed trie node layouts
- batched runtime inference tokenization

---

# Example Merge Walkthrough

Input:

```text
low lower lowest
```

Initial representation:

```text
l o w
l o w e r
l o w e s t
```

Frequent pair:

```text
(l,o)
```

After merge:

```text
lo w
lo w e r
lo w e s t
```

Next merge:

```text
(lo,w)
```

Result:

```text
low
low e r
low e s t
```

This process continues incrementally.

---

# Example Tokenization Flow

```text
Input String
      ↓
UTF-8 Byte Stream
      ↓
Greedy Prefix Matching
      ↓
Radix Tree Traversal
      ↓
Longest Token Match
      ↓
Token ID Emission
```

---

# Data Structures Used

| Structure | Purpose |
|---|---|
| MemoryPool<T> | Contiguous allocation |
| TokenNode | Intrusive token stream |
| TokenPairData | Pair occurrence tracking |
| Priority Queue | Highest-frequency pair scheduling |
| Radix Tree | Vocabulary lookup |
| Token DAG | Merge lineage representation |
| Binary Buffer | Serialization artifact |

---

# Time Complexity Analysis

| Operation | Complexity |
|---|---|
| DLL Merge Rewiring | O(1) |
| Pair Frequency Update | O(1) amortized |
| Heap Push | O(log N) |
| Heap Pop | O(log N) |
| Trie Lookup | O(k) |
| Token Serialization | O(V) |
| Vocabulary Insertion | O(k) |

Where:

- `N` = number of pair entries
- `V` = vocabulary size
- `k` = token byte length

---

# Production-Oriented Design Choices

## Deterministic Runtime Layouts

The tokenizer avoids:

- recursive ownership graphs
- unstable pointer structures
- runtime allocator dependency

---

## Serialization-Aware Architecture

All runtime structures are designed to survive binary persistence.

---

## Infrastructure-Centric Engineering

The implementation emphasizes:

- memory behavior
- runtime efficiency
- deployment practicality
- cache-aware layouts
- scalable state updates

---

## Runtime Safety Through Validation

Stale heap entries are validated lazily.

This avoids expensive eager synchronization.

---

# Current Limitations

Current implementation limitations include:

- single-threaded merge execution
- no SIMD acceleration
- UTF-8 handling remains byte-oriented
- no memory-mapped runtime vocabulary loading
- radix nodes still use dense child arrays
- serialization lacks version headers/checksums
- no concurrent training pipeline

---

# Future Improvements

Potential future directions:

## Runtime

- SIMD trie traversal
- branchless lookup paths
- cache-line aligned node packing
- compact child transition encoding

---

## Training

- parallel pair aggregation
- distributed corpus ingestion
- lock-free heap scheduling
- shard-based merge execution

---

## Serialization

- memory-mapped vocab artifacts
- compressed runtime blobs
- zero-copy deserialization
- endian-safe serialization layers

---

## Inference Integration

- streaming tokenizer APIs
- speculative tokenization
- GPU-assisted preprocessing
- batched tokenizer pipelines

---

# Build Instructions

## Requirements

- C++17 or newer
- GCC / Clang / MSVC

---

## Build

```bash
g++ -O3 -std=c++17 main.cpp -o tokenizer
```

Recommended flags:

```bash
-O3
-march=native
-flto
```

---

# Example Usage

```cpp
uint_32 targetVocabSize = 1000;
uint_32 memoryPoolSize = 10000;

BytePairTokenizer tokenizer(
    targetVocabSize,
    memoryPoolSize
);

bool success = tokenizer.train(corpus);
```

---

# Benchmark / Performance Section

> Placeholder for empirical benchmark data.

Suggested future benchmark dimensions:

| Benchmark | Metric |
|---|---|
| Merge throughput | merges/sec |
| Tokenization throughput | MB/sec |
| Cache misses | LLC miss ratio |
| Allocation count | allocations/sec |
| Serialization latency | ms |
| Vocabulary load time | ms |

---

# Repository Structure

```text
.
├── main.cpp
├── bpeTokenizer.cpp
├── MemoryPool.cpp
├── Models.cpp
├── Serializer.cpp
├── saveutility.cpp
├── learnedVocab.bin
└── README.md
```

---

# Architecture Diagram Placeholder

```text
                   ┌─────────────────────┐
                   │     Text Corpus     │
                   └──────────┬──────────┘
                              │
                              ▼
             ┌────────────────────────────┐
             │ Intrusive Token Sequences  │
             └──────────┬─────────────────┘
                        │
                        ▼
             ┌────────────────────────────┐
             │ Pair Frequency Aggregator  │
             └──────────┬─────────────────┘
                        │
                        ▼
             ┌────────────────────────────┐
             │ Incremental Merge Engine   │
             └──────────┬─────────────────┘
                        │
                        ▼
             ┌────────────────────────────┐
             │ Prefix Radix Vocabulary    │
             └──────────┬─────────────────┘
                        │
                        ▼
             ┌────────────────────────────┐
             │ Serialized Runtime Artifact│
             └────────────────────────────┘
```

---

# References to Modern LLM Tokenization Concepts

This project explores infrastructure concepts commonly associated with:

- GPT-family tokenizer systems
- Claude-family tokenizer runtimes
- large-scale inference preprocessing
- vocabulary serialization pipelines
- runtime subword lookup systems
- cache-aware tokenizer execution
- scalable merge scheduling
- deployment-oriented tokenizer artifacts

The implementation intentionally focuses on low-level engineering concerns rather than NLP abstractions.

---

# Source-Level Architecture Notes

The current implementation includes:

- intrusive token node structures 
- pool-based allocator infrastructure 
- binary serialization/deserialization systems 
- disk-backed runtime artifact persistence 
- incremental merge execution engine 
- radix-tree based vocabulary insertion and lookup 

---

# Conclusion

This repository is an exploration of tokenizer infrastructure through the lens of systems engineering.

The implementation prioritizes:

- runtime efficiency
- memory locality
- scalable merge execution
- deterministic serialization
- allocator-aware architecture
- production-oriented tokenizer design

Rather than treating tokenization as a simple preprocessing utility, the project approaches it as a foundational runtime subsystem in modern LLM infrastructure.

The result is a low-level tokenizer implementation focused on engineering tradeoffs that become increasingly important at production scale.

