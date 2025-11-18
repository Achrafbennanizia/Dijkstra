// ============================================================================
// Dijkstra's Algorithm: Sequential vs Parallel Implementation
// ============================================================================
// This program implements both sequential and parallel versions of Dijkstra's
// shortest path algorithm using Intel TBB (Threading Building Blocks) library.

#include <iostream>
#include <tbb/global_control.h> // Control TBB global settings
#include <tbb/parallel_for.h>   // Parallel for loop from TBB
#include <tbb/combinable.h>     // Thread-local storage from TBB
#include <chrono>               // For timing measurements
#include <queue>                // Priority queue for Dijkstra
#include <limits>               // For std::numeric_limits
#include <fstream>              // File input
#include <sstream>              // String stream parsing
#include <string>               // String handling
#include <vector>               // Dynamic arrays
#include <algorithm>            // Standard algorithms (std::reverse)

// Use steady_clock for precise timing that is unaffected by system clock adjustments
using Clock = std::chrono::steady_clock;

// Data structure representing a directed edge in the graph
struct Edge
{
    int to;     // Target node index
    int weight; // Edge weight (cost/distance)
};

// Graph is represented as an adjacency list: vector of vectors of edges
// graph[u] contains all edges from node u to its neighbors
using Graph = std::vector<std::vector<Edge>>;

// Read graph from DIMACS format file (.gr file)
// DIMACS format:
//   c <comment>
//   p sp <num_nodes> <num_edges>
//   a <from> <to> <weight>
//
// Returns: true if file read successfully, false otherwise
bool read_dimacs_gr(const std::string &filename, Graph &graph, int &n_nodes)
{
    std::ifstream in(filename);
    if (!in)
    {
        std::cerr << "Could not open file " << filename << "\n";
        return false;
    }

    std::string line;
    int m_edges = 0;
    n_nodes = 0;

    // Parse file line by line
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        if (line[0] == 'c')
            continue; // Skip comment lines (marked with 'c')

        if (line[0] == 'p')
        {
            // Parse problem line: "p sp <n_nodes> <n_edges>"
            std::stringstream ss(line);
            char p;
            std::string type;
            ss >> p >> type >> n_nodes >> m_edges;
            // Initialize graph with n_nodes+1 empty adjacency lists (1-based indexing)
            graph.assign(n_nodes + 1, {});
        }
        else if (line[0] == 'a')
        {
            // Parse arc (edge) line: "a <from> <to> <weight>"
            std::stringstream ss(line);
            char a;
            int u, v, w;
            ss >> a >> u >> v >> w;
            // Add edge from u to v with weight w
            graph[u].push_back(Edge{v, w});
        }
    }
    return true;
}

// Result structure holding the output of Dijkstra's algorithm
struct DijkstraResult
{
    std::vector<long long> dist; // dist[i] = shortest distance from source to node i
    std::vector<int> parent;     // parent[i] = previous node in shortest path to i
};

// ============================================================================
// SEQUENTIAL DIJKSTRA'S ALGORITHM
// ============================================================================
// Standard single-threaded implementation using a min-heap priority queue.
// Time Complexity: O((V + E) * log V) where V = vertices, E = edges
//
// Algorithm:
// 1. Initialize distances to all nodes as infinity, except source = 0
// 2. Use priority queue to process nodes in order of increasing distance
// 3. For each node, relax its outgoing edges (update neighbor distances if shorter path found)
// 4. Track parent pointers to reconstruct the shortest path
//
DijkstraResult dijkstra_sequential(const Graph &graph, int source)
{
    // Use a large value for "infinity" (avoid overflow in comparisons)
    const long long INF = std::numeric_limits<long long>::max() / 4;
    int n = (int)graph.size() - 1; // Number of nodes (excluding index 0)

    // Initialize distance and parent arrays
    std::vector<long long> dist(n + 1, INF); // All distances start as infinity
    std::vector<int> parent(n + 1, -1);      // No parent initially

    // Priority queue: stores (distance, node) pairs
    // Elements are ordered by distance (min-heap with greater<> comparator)
    using PQItem = std::pair<long long, int>; // (distance, node_id)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    // Start from source: distance is 0
    dist[source] = 0;
    pq.push({0, source});

    // Process nodes in order of increasing distance
    while (!pq.empty())
    {
        auto [d, u] = pq.top(); // Extract minimum distance node
        pq.pop();

        // Skip if this is an outdated entry (we've already found a better path to u)
        if (d != dist[u])
            continue;

        // Relax all outgoing edges from u
        for (const auto &e : graph[u])
        {
            long long nd = d + e.weight; // New distance via node u
            // If we found a shorter path to neighbor e.to
            if (nd < dist[e.to])
            {
                dist[e.to] = nd;     // Update distance
                parent[e.to] = u;    // Record parent for path reconstruction
                pq.push({nd, e.to}); // Add to queue for processing
            }
        }
    }

    return {std::move(dist), std::move(parent)};
}

// ============================================================================
// PARALLEL DIJKSTRA'S ALGORITHM
// ============================================================================
// Parallel implementation using Intel TBB (Threading Building Blocks).
//
// Key insight: When processing a node u, we can parallelize the edge relaxation
// step because all outgoing edges from u are independent of each other initially.
//
// Strategy:
// 1. Main loop remains sequential (priority queue access must be serialized)
// 2. For each node u, use parallel_for to check all outgoing edges in parallel
// 3. Collect candidate updates in thread-local storage (combinable)
// 4. Apply all updates serially to maintain consistency
//
DijkstraResult dijkstra_parallel(const Graph &graph, int source)
{
    const long long INF = std::numeric_limits<long long>::max() / 4;
    int n = (int)graph.size() - 1;

    std::vector<long long> dist(n + 1, INF);
    std::vector<int> parent(n + 1, -1);

    using PQItem = std::pair<long long, int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0;
    pq.push({0, source});

    // Threshold: only parallelize if edge count is large enough to warrant overhead
    const std::size_t PARALLEL_THRESHOLD = 100;

    // Main loop: process nodes in order (must be serial due to priority queue)
    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();
        if (d != dist[u])
            continue;

        const auto &edges = graph[u];
        if (edges.empty())
            continue;

        // For small edge sets, process sequentially to avoid parallelization overhead
        if (edges.size() < PARALLEL_THRESHOLD)
        {
            for (const auto &e : edges)
            {
                long long nd = d + e.weight;
                if (nd < dist[e.to])
                {
                    dist[e.to] = nd;
                    parent[e.to] = u;
                    pq.push({nd, e.to});
                }
            }
            continue; // Skip parallel section
        }

        // For large edge sets, use parallel processing
        struct Update
        {
            int v;
            long long nd;
            int p;
        };

        // Use a simple vector instead of combinable for better cache locality
        std::vector<Update> all_updates;
        std::mutex update_mutex;

        // PARALLEL SECTION: Check all outgoing edges from u in parallel
        tbb::parallel_for(std::size_t(0), edges.size(), [&](std::size_t i)
                          {
            const auto& e = edges[i];
            long long nd = d + e.weight;

            if (nd < dist[e.to])
            {
                // Lock only when adding to shared vector (brief lock)
                {
                    std::lock_guard<std::mutex> lock(update_mutex);
                    all_updates.push_back(Update{e.to, nd, u});
                }
            } });

        // Apply all collected updates
        for (const auto &up : all_updates)
        {
            if (up.nd < dist[up.v])
            {
                dist[up.v] = up.nd;
                parent[up.v] = up.p;
                pq.push({up.nd, up.v});
            }
        }
    }

    return {std::move(dist), std::move(parent)};
}

// Calculate milliseconds elapsed between two time points
long long ms_between(Clock::time_point start, Clock::time_point end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================
// Compares sequential vs parallel Dijkstra implementation performance
//
// Usage: dijkstra <graph_file.gr> <target_node>
//   graph_file.gr : DIMACS format graph file
//   target_node   : destination node to compute shortest path to
//
int main(int argc, char **argv)
{
    // Validate command line arguments
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <graph.gr> <target_node>\n";
        return 1;
    }

    std::string filename = argv[1];
    int target = std::stoi(argv[2]);

    std::cout << "Hello, Dijkstra's World!\n";
    std::cout << "Reading graph data from " << filename << "...\n";

    // Load graph from file
    Graph graph;
    int n_nodes = 0;
    if (!read_dimacs_gr(filename, graph, n_nodes))
    {
        return 1; // File read error
    }
    std::cout << "ok (" << n_nodes << " nodes)\n";

    int source = 1; // Start from node 1

    // ========================================================================
    // Set up thread control BEFORE timing (to avoid overhead in sequential run)
    // ========================================================================
    int p = 4;
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, p);

    // ========================================================================
    // RUN 1: Sequential Dijkstra
    // ========================================================================
    auto t1 = Clock::now();
    auto res_seq = dijkstra_sequential(graph, source);
    auto t2 = Clock::now();
    auto ms_seq = ms_between(t1, t2);

    // ========================================================================
    // RUN 2: Parallel Dijkstra
    // ========================================================================

    auto t3 = Clock::now();
    auto res_par = dijkstra_parallel(graph, source);
    auto t4 = Clock::now();
    auto ms_par = ms_between(t3, t4);

    // ========================================================================
    // PERFORMANCE METRICS
    // ========================================================================
    std::cout << "Sequential time: " << ms_seq << " ms\n";
    std::cout << "Parallel (" << p << " threads) time: " << ms_par << " ms\n";

    // Speedup: how much faster is parallel vs sequential
    // Efficiency: speedup divided by number of threads (scales from 0 to 1)
    double speedup = (ms_par > 0) ? (double)ms_seq / (double)ms_par : 0.0;
    double efficiency = (ms_par > 0) ? speedup / p : 0.0;
    std::cout << "Speed-Up S_p = " << speedup << "\n";
    std::cout << "Efficiency E_p = " << efficiency << "\n";

    // ========================================================================
    // OUTPUT RESULTS
    // ========================================================================
    // Use parallel result (should match sequential result)
    auto dist = res_par.dist;
    auto parent = res_par.parent;

    std::cout << "Distance " << source << "-->" << target << ": " << dist[target] << "\n";
    std::cout << "Path: ";

    // Reconstruct shortest path by following parent pointers backward from target to source
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v])
    {
        path.push_back(v);
    }
    // Reverse to get path from source to target
    std::reverse(path.begin(), path.end());

    // Print the path
    for (int v : path)
        std::cout << v << " ";
    std::cout << "\n";

    return 0;
}
