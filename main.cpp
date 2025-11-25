#include <iostream>
#include <tbb/global_control.h>
#include <tbb/parallel_for.h>
#include <chrono>
#include <queue>
#include <limits>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>

using Clock = std::chrono::steady_clock;

struct Edge
{
    int to;
    int weight;
};

using Graph = std::vector<std::vector<Edge>>;

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

    while (std::getline(in, line))
    {
        if (line.empty() || line[0] == 'c')
            continue;

        if (line[0] == 'p')
        {
            std::stringstream ss(line);
            char p;
            std::string type;
            ss >> p >> type >> n_nodes >> m_edges;
            graph.assign(n_nodes + 1, {});
        }
        else if (line[0] == 'a')
        {
            std::stringstream ss(line);
            char a;
            int u, v, w;
            ss >> a >> u >> v >> w;
            graph[u].push_back({v, w});
        }
    }
    return true;
}

struct DijkstraResult
{
    std::vector<long long> dist;
    std::vector<int> parent;
};

DijkstraResult dijkstra_sequential(const Graph &graph, int source)
{
    const long long INF = std::numeric_limits<long long>::max() / 4;
    int n = (int)graph.size() - 1;

    std::vector<long long> dist(n + 1, INF);
    std::vector<int> parent(n + 1, -1);

    using PQItem = std::pair<long long, int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();

        if (d != dist[u])
            continue;

        for (const auto &e : graph[u])
        {
            long long new_dist = d + e.weight;
            if (new_dist < dist[e.to])
            {
                dist[e.to] = new_dist;
                parent[e.to] = u;
                pq.push({new_dist, e.to});
            }
        }
    }

    return {std::move(dist), std::move(parent)};
}

DijkstraResult dijkstra_parallel(const Graph &graph, int source)
{
    const long long INF = std::numeric_limits<long long>::max() / 4;
    const size_t THRESHOLD = 100;
    int n = (int)graph.size() - 1;

    std::vector<long long> dist(n + 1, INF);
    std::vector<int> parent(n + 1, -1);

    using PQItem = std::pair<long long, int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0;
    pq.push({0, source});

    struct Update
    {
        int node;
        long long dist;
        int prev;
    };

    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();

        if (d != dist[u])
            continue;

        const auto &edges = graph[u];
        if (edges.empty())
            continue;

        if (edges.size() < THRESHOLD)
        {
            for (const auto &e : edges)
            {
                long long new_dist = d + e.weight;
                if (new_dist < dist[e.to])
                {
                    dist[e.to] = new_dist;
                    parent[e.to] = u;
                    pq.push({new_dist, e.to});
                }
            }
        }
        else
        {
            std::vector<Update> updates;
            std::mutex mtx;

            tbb::parallel_for(size_t(0), edges.size(), [&](size_t i)
                              {
                const auto &e = edges[i];
                long long new_dist = d + e.weight;

                if (new_dist < dist[e.to]) {
                    std::lock_guard<std::mutex> lock(mtx);
                    updates.push_back({e.to, new_dist, u});
                } });

            for (const auto &upd : updates)
            {
                if (upd.dist < dist[upd.node])
                {
                    dist[upd.node] = upd.dist;
                    parent[upd.node] = upd.prev;
                    pq.push({upd.dist, upd.node});
                }
            }
        }
    }

    return {std::move(dist), std::move(parent)};
}

long long ms_between(Clock::time_point start, Clock::time_point end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <graph.gr> <target_node>\n";
        return 1;
    }

    std::string filename = argv[1];
    int target = std::stoi(argv[2]);

    std::cout << "Reading graph from " << filename << "...\n";

    Graph graph;
    int n_nodes = 0;
    if (!read_dimacs_gr(filename, graph, n_nodes))
    {
        return 1;
    }
    std::cout << "Loaded " << n_nodes << " nodes\n";

    int source = 1;
    int num_threads = 10;
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, num_threads);

    auto t1 = Clock::now();
    auto res_seq = dijkstra_sequential(graph, source);
    auto t2 = Clock::now();
    auto ms_seq = ms_between(t1, t2);

    auto t3 = Clock::now();
    auto res_par = dijkstra_parallel(graph, source);
    auto t4 = Clock::now();
    auto ms_par = ms_between(t3, t4);

    std::cout << "\nPerformance Results:\n";
    std::cout << "Sequential time: " << ms_seq << " ms\n";
    std::cout << "Parallel (" << num_threads << " threads) time: " << ms_par << " ms\n";

    double speedup = (ms_par > 0) ? (double)ms_seq / (double)ms_par : 0.0;
    double efficiency = (ms_par > 0) ? speedup / num_threads : 0.0;
    std::cout << "Speedup: " << speedup << "x\n";
    std::cout << "Efficiency: " << efficiency << "\n";

    std::cout << "\nShortest Path from " << source << " to " << target << ":\n";
    std::cout << "Distance: " << res_par.dist[target] << "\n";
    std::cout << "Path: ";

    std::vector<int> path;
    for (int v = target; v != -1; v = res_par.parent[v])
    {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());

    for (int v : path)
        std::cout << v << " ";
    std::cout << "\n";

    return 0;
}
