# Dijkstra's Algorithm: Sequential vs Parallel Implementation

A high-performance C++ implementation comparing sequential and parallel versions of Dijkstra's shortest path algorithm using Intel TBB.

## 🎯 Project Highlights

- **Sequential & Parallel Algorithms**: Single-threaded vs multi-threaded implementations
- **Adaptive Parallelization**: Automatic threshold detection (100+ edges per node)
- **Performance Metrics**: Speedup and efficiency calculations
- **DIMACS Graph Format**: Standard format support for graph representation
- **Graph Generation**: Python utility to generate test graphs of various sizes
- **Path Reconstruction**: Efficient path recovery from target to source

## 📊 Features

✅ Sequential Dijkstra using min-heap priority queue  
✅ Parallel Dijkstra with Intel TBB combinable  
✅ Adaptive threshold-based parallelization  
✅ DIMACS format graph parsing  
✅ Performance benchmarking  
✅ Path visualization  
✅ Comprehensive Python graph generator

## 🚀 Quick Start

```bash
# Generate test graphs
python3 generate_graph.py --all

# Build project
cd cmake-build-debug && cmake .. && ninja

# Run shortest path
./dijkstra massive_graph.gr 2500
```

## 📈 Performance

On 5000-node, 50,000-edge graph:

- Sequential: 4ms
- Parallel (10 threads): 4ms
- Adaptive threshold prevents overhead on sparse edge distributions

## 📚 Documentation

See [README.md](README.md) for complete documentation, usage examples, and algorithm details.

## 🛠️ Tech Stack

- **Language**: C++17
- **Threading**: Intel TBB (Threading Building Blocks)
- **Build**: CMake + Ninja
- **Format**: DIMACS graph format
- **Utilities**: Python 3 for graph generation
