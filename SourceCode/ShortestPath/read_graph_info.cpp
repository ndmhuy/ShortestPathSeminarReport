#include "AdjacencyList.h"
#include "ShortestPath.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

enum class HeapBackend {
    Custom,
    Boost
};

struct GraphLoadStats {
    size_t declaredNodes = 0;
    size_t headerEdges = 0;
    size_t loadedEdges = 0;
    size_t maxNodeId = 0;
    size_t loadedVertices = 0;
};

struct BenchmarkResult {
    std::string name;
    double totalMs = 0.0;
    double averageMs = 0.0;
    size_t runs = 0;
    long long checksum = 0;
};

static constexpr int INF_VALUE = std::numeric_limits<int>::max() / 4;
static constexpr int NEG_INF_VALUE = std::numeric_limits<int>::min() / 4;

static void printProgress(const std::string& label, size_t processed, size_t total, size_t& lastShownPercent) {
    if (total == 0) {
        return;
    }

    size_t percent = (processed * 100) / total;
    if (percent > 100) {
        percent = 100;
    }

    if (percent >= lastShownPercent + 5 || percent == 100) {
        std::cout << "\r" << label << " " << std::setw(3) << percent << "%" << std::flush;
        lastShownPercent = percent;
        if (percent == 100) {
            std::cout << "\n";
        }
    }
}

static void parseHeaderLine(const std::string& line, GraphLoadStats& stats) {
    // Example header line: "# Nodes: 4847571 Edges: 68993773"
    if (line.find("Nodes:") == std::string::npos || line.find("Edges:") == std::string::npos) {
        return;
    }

    std::istringstream in(line);
    std::string token;
    while (in >> token) {
        if (token == "Nodes:") {
            in >> stats.declaredNodes;
        } else if (token == "Edges:") {
            in >> stats.headerEdges;
        }
    }
}

static bool scanGraphFile(const std::string& path, GraphLoadStats& stats) {
    std::ifstream input(path);
    if (!input.is_open()) {
        std::cerr << "Could not open file: " << path << "\n";
        return false;
    }

    size_t fileSize = 0;
    try {
        fileSize = static_cast<size_t>(std::filesystem::file_size(path));
    } catch (...) {
        fileSize = 0;
    }

    size_t processedBytes = 0;
    size_t lastShownPercent = 0;

    std::string line;
    while (std::getline(input, line)) {
        processedBytes += line.size() + 1;
        printProgress("Scan progress:", processedBytes, fileSize, lastShownPercent);

        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            parseHeaderLine(line, stats);
            continue;
        }

        std::istringstream edgeLine(line);
        size_t from = 0;
        size_t to = 0;
        if (!(edgeLine >> from >> to)) {
            continue;
        }

        if (from > stats.maxNodeId) {
            stats.maxNodeId = from;
        }
        if (to > stats.maxNodeId) {
            stats.maxNodeId = to;
        }
    }

    if (fileSize == 0) {
        std::cout << "Scan progress: 100%\n";
    }

    return true;
}

static bool loadDirectedGraphFromFile(const std::string& path, AdjacencyList& graph, GraphLoadStats& stats) {
    std::ifstream input(path);
    if (!input.is_open()) {
        std::cerr << "Could not open file: " << path << "\n";
        return false;
    }

    const size_t vertexCount = std::max(stats.declaredNodes, stats.maxNodeId + 1);
    for (size_t vertex = 0; vertex < vertexCount; ++vertex) {
        graph.addVertex(vertex);
    }
    stats.loadedVertices = graph.size();

    size_t fileSize = 0;
    try {
        fileSize = static_cast<size_t>(std::filesystem::file_size(path));
    } catch (...) {
        fileSize = 0;
    }

    size_t processedBytes = 0;
    size_t lastShownPercent = 0;

    std::string line;
    while (std::getline(input, line)) {
        processedBytes += line.size() + 1;
        printProgress("Load progress:", processedBytes, fileSize, lastShownPercent);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream edgeLine(line);
        size_t from = 0;
        size_t to = 0;
        int weight = 1;
        if (!(edgeLine >> from >> to)) {
            continue;
        }
        if (!(edgeLine >> weight)) {
            weight = 1;
        }

        graph.addEdge(from, to, weight);
        stats.loadedEdges++;
    }

    if (fileSize == 0) {
        std::cout << "Load progress: 100%\n";
    }

    graph.setEdgeCount(stats.loadedEdges);

    return true;
}

static BenchmarkResult benchmarkDijkstra(
    const std::string& name,
    const IGraphStructure& graph,
    size_t sourceRuns,
    std::vector<int> (*algorithm)(const IGraphStructure&, size_t)
) {
    BenchmarkResult result;
    result.name = name;
    result.runs = sourceRuns;

    long long checksum = 0;
    size_t lastShownPercent = 0;
    const auto begin = std::chrono::high_resolution_clock::now();
    for (size_t source = 0; source < sourceRuns; ++source) {
        const std::vector<int> distances = algorithm(graph, source);
        if (!distances.empty() && source < distances.size() && distances[source] == 0) {
            checksum += 1;
        }

        printProgress(name + " progress:", source + 1, sourceRuns, lastShownPercent);
    }
    const auto end = std::chrono::high_resolution_clock::now();

    result.checksum = checksum;
    result.totalMs = std::chrono::duration<double, std::milli>(end - begin).count();
    result.averageMs = sourceRuns == 0 ? 0.0 : (result.totalMs / static_cast<double>(sourceRuns));
    return result;
}

static std::optional<std::vector<size_t>> computeTopologicalOrderIfDag(const IGraphStructure& graph) {
    const size_t n = graph.size();
    std::vector<size_t> indegree(n, 0);
    for (const auto& edge : graph.getEdges()) {
        indegree[edge.to]++;
    }

    std::queue<size_t> q;
    for (size_t i = 0; i < n; ++i) {
        if (indegree[i] == 0) {
            q.push(i);
        }
    }

    std::vector<size_t> topoOrder;
    topoOrder.reserve(n);
    size_t visited = 0;
    while (!q.empty()) {
        const size_t u = q.front();
        q.pop();
        visited++;
        topoOrder.push_back(u);

        for (const auto& edge : graph.getEdgesFromVertex(u)) {
            if (--indegree[edge.to] == 0) {
                q.push(edge.to);
            }
        }
    }

    if (visited != n) {
        return std::nullopt;
    }

    return topoOrder;
}

static BenchmarkResult benchmarkDagWithPrecomputedTopologicalOrder(
    const IGraphStructure& graph,
    size_t sourceRuns,
    const std::vector<size_t>& topoOrder
) {
    BenchmarkResult result;
    result.name = "dagShortestPath (precomputed topo order)";
    result.runs = sourceRuns;

    long long checksum = 0;
    size_t lastShownPercent = 0;
    const auto begin = std::chrono::high_resolution_clock::now();
    for (size_t source = 0; source < sourceRuns; ++source) {
        const std::vector<int> distances = ShortestPath::dagShortestPathWithTopoOrder(graph, source, topoOrder);
        if (!distances.empty() && source < distances.size() && distances[source] == 0) {
            checksum += 1;
        }

        printProgress("dagShortestPath (precomputed topo order) progress:", source + 1, sourceRuns, lastShownPercent);
    }
    const auto end = std::chrono::high_resolution_clock::now();

    result.checksum = checksum;
    result.totalMs = std::chrono::duration<double, std::milli>(end - begin).count();
    result.averageMs = sourceRuns == 0 ? 0.0 : (result.totalMs / static_cast<double>(sourceRuns));
    return result;
}

static AdjacencyList buildLimitedGraph(const IGraphStructure& graph, size_t limitVertices) {
    AdjacencyList limited(true);
    for (size_t v = 0; v < limitVertices; ++v) {
        limited.addVertex(v);
    }

    size_t count = 0;
    for (const auto& edge : graph.getEdges()) {
        if (edge.from < limitVertices && edge.to < limitVertices) {
            limited.addEdge(edge.from, edge.to, edge.weight);
            count++;
        }
    }
    limited.setEdgeCount(count);
    return limited;
}

static BenchmarkResult benchmarkFloydWarshall(const IGraphStructure& graph, size_t floydVertices) {
    BenchmarkResult result;
    result.name = "floydWarshall (bounded vertices)";
    result.runs = 1;

    AdjacencyList limitedGraph = buildLimitedGraph(graph, floydVertices);

    const auto begin = std::chrono::high_resolution_clock::now();
    const std::vector<std::vector<int>> distances = ShortestPath::floydWarshall(limitedGraph);
    const auto end = std::chrono::high_resolution_clock::now();

    long long checksum = 0;
    for (size_t i = 0; i < distances.size(); ++i) {
        for (size_t j = 0; j < distances[i].size(); ++j) {
            const int d = distances[i][j];
            if (d != INF_VALUE && d != NEG_INF_VALUE) {
                checksum += static_cast<long long>((i + 1) * (j + 1));
            }
        }
    }

    result.checksum = checksum;
    result.totalMs = std::chrono::duration<double, std::milli>(end - begin).count();
    result.averageMs = result.totalMs;
    return result;
}

int main(int argc, char* argv[]) {
    // Usage:
    //   ./read_graph_info [inputPath] [sourceRuns] [floydVertexLimit] [skipOptions...]
    // Skip options (can pass multiple):
    //   --skip-bellman      Skip bellmanFord benchmark
    //   --skip-floyd        Skip floydWarshall benchmark
    //   --skip-heavy        Skip both bellmanFord and floydWarshall
    std::string inputPath = "soc-LiveJournal1.txt";
    size_t sourceVerticesToRun = 3;
    size_t floydVertexLimit = 200;
    bool skipBellman = false;
    bool skipFloyd = false;
    HeapBackend heapBackend = HeapBackend::Custom;

    if (argc > 1) {
        inputPath = argv[1];
    }
    if (argc > 2) {
        sourceVerticesToRun = static_cast<size_t>(std::stoull(argv[2]));
    }
    if (argc > 3) {
        floydVertexLimit = static_cast<size_t>(std::stoull(argv[3]));
    }
    for (int i = 4; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--skip-bellman") {
            skipBellman = true;
        } else if (arg == "--skip-floyd") {
            skipFloyd = true;
        } else if (arg == "--skip-heavy" || arg == "skip-heavy" || arg == "1") {
            skipBellman = true;
            skipFloyd = true;
        } else if (arg == "--heap-backend=boost") {
            heapBackend = HeapBackend::Boost;
        } else if (arg == "--heap-backend=custom") {
            heapBackend = HeapBackend::Custom;
        }
    }

    if (heapBackend == HeapBackend::Boost && !ShortestPath::isBoostHeapAvailable()) {
        std::cout << "Requested --heap-backend=boost, but Boost heap headers are unavailable. Falling back to custom heaps.\n";
        heapBackend = HeapBackend::Custom;
    }

    GraphLoadStats stats;
    if (!scanGraphFile(inputPath, stats)) {
        return 1;
    }

    AdjacencyList graph(true);
    if (!loadDirectedGraphFromFile(inputPath, graph, stats)) {
        return 1;
    }

    const size_t sourceRuns = std::min(sourceVerticesToRun, graph.size());
    if (sourceRuns == 0) {
        std::cerr << "No vertices available to run Dijkstra.\n";
        return 1;
    }

    std::cout << "Loaded graph from: " << inputPath << "\n";
    if (stats.declaredNodes > 0 || stats.headerEdges > 0) {
        std::cout << "Header nodes: " << stats.declaredNodes
                  << ", header edges: " << stats.headerEdges << "\n";
    }

    std::cout << "Loaded edges: " << stats.loadedEdges << "\n";
    std::cout << "Loaded vertices: " << stats.loadedVertices << "\n";
    std::cout << "Max node id seen: " << stats.maxNodeId << "\n";
    std::cout << "Benchmark source vertices: " << sourceRuns << "\n\n";
    std::cout << "Heap backend: " << (heapBackend == HeapBackend::Boost ? "boost" : "custom") << "\n\n";
    if (skipBellman) {
        std::cout << "bellmanFord benchmark disabled.\n";
    }
    if (skipFloyd) {
        std::cout << "floydWarshall benchmark disabled.\n";
    }
    if (skipBellman || skipFloyd) {
        std::cout << "\n";
    }

    std::vector<BenchmarkResult> results;
    results.push_back(benchmarkDijkstra("dijkstra (binary heap)", graph, sourceRuns, ShortestPath::dijkstra));
    if (heapBackend == HeapBackend::Boost) {
        results.push_back(benchmarkDijkstra("eagerDijkstra (boost fibonacci heap)", graph, sourceRuns, ShortestPath::eagerDijkstraBoost));
        results.push_back(benchmarkDijkstra("dijkstraWithDaryHeap (boost d_ary heap)", graph, sourceRuns, ShortestPath::dijkstraWithBoostDaryHeap));
    } else {
        results.push_back(benchmarkDijkstra("eagerDijkstra (indexed pq)", graph, sourceRuns, ShortestPath::eagerDijkstra));
        results.push_back(benchmarkDijkstra("dijkstraWithDaryHeap", graph, sourceRuns, ShortestPath::dijkstraWithDaryHeap));
    }
    if (!skipBellman) {
        results.push_back(benchmarkDijkstra("bellmanFord", graph, sourceRuns, ShortestPath::bellmanFord));
    }

    const std::optional<std::vector<size_t>> topoOrder = computeTopologicalOrderIfDag(graph);
    if (topoOrder.has_value()) {
        results.push_back(benchmarkDijkstra("dagShortestPath", graph, sourceRuns, ShortestPath::dagShortestPath));
        results.push_back(benchmarkDagWithPrecomputedTopologicalOrder(graph, sourceRuns, *topoOrder));
    } else {
        std::cout << "Skipping dagShortestPath: graph is not a DAG.\n\n";
    }

    const size_t boundedFloydVertices = std::min(floydVertexLimit, graph.size());
    if (!skipFloyd && boundedFloydVertices > 0) {
        std::cout << "Running floydWarshall on induced subgraph [0.." << (boundedFloydVertices - 1)
                  << "] to keep runtime practical.\n\n";
        results.push_back(benchmarkFloydWarshall(graph, boundedFloydVertices));
    }

    for (const auto& result : results) {
        std::cout << result.name << "\n";
        std::cout << "  Total time (ms): " << result.totalMs << "\n";
        std::cout << "  Average time per vertex (ms): " << result.averageMs << "\n";
        std::cout << "  Runs: " << result.runs << ", checksum: " << result.checksum << "\n\n";
    }

    return 0;
}
