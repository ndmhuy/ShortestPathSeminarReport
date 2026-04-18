#include "ShortestPath.h"
#include "../Heap/IndexedPriorityQueue.h"
#include "../Heap/IndexedDaryHeap.h"

#include <limits>
#include <queue>

#if defined(__has_include)
#if __has_include(<boost/heap/d_ary_heap.hpp>) && __has_include(<boost/heap/fibonacci_heap.hpp>)
#define SHORTESTPATH_HAS_BOOST_HEAP 1
#include <boost/heap/d_ary_heap.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#endif
#endif

static constexpr int INF = std::numeric_limits<int>::max() / 4;
static constexpr int NEG_INF = std::numeric_limits<int>::min() / 4;

bool ShortestPath::isBoostHeapAvailable() {
#ifdef SHORTESTPATH_HAS_BOOST_HEAP
    return true;
#else
    return false;
#endif
}

std::vector<int> ShortestPath::dagShortestPath(const IGraphStructure& graph, size_t startVertex) {
    std::vector<size_t> inDegree(graph.size(), 0);
    for (const auto& edge : graph.getEdges()) {
        inDegree[edge.to]++;
    }

    std::queue<size_t> zeroInDegreeQueue;
    for (size_t i = 0; i < graph.size(); ++i) {
        if (inDegree[i] == 0) {
            zeroInDegreeQueue.push(i);
        }
    }

    std::vector<size_t> topoOrder;
    topoOrder.reserve(graph.size());

    while (!zeroInDegreeQueue.empty()) {
        size_t currentVertex = zeroInDegreeQueue.front();
        zeroInDegreeQueue.pop();

        topoOrder.push_back(currentVertex);

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            size_t neighbor = edge.to;
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                zeroInDegreeQueue.push(neighbor);
            }
        }
    }

    return dagShortestPathWithTopoOrder(graph, startVertex, topoOrder);
}

std::vector<int> ShortestPath::dagShortestPathWithTopoOrder(
    const IGraphStructure& graph,
    size_t startVertex,
    const std::vector<size_t>& topoOrder
) {
    std::vector<int> distances(graph.size(), INF);
    distances[startVertex] = 0;

    for (const size_t currentVertex : topoOrder) {
        if (distances[currentVertex] == INF) {
            continue;
        }

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            const size_t neighbor = edge.to;
            const int newDist = distances[currentVertex] + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
            }
        }
    }

    return distances;
}

std::vector<int> ShortestPath::dijkstra(const IGraphStructure& graph, size_t startVertex) {
    std::vector<int> distances(graph.size(), INF);
    std::vector<bool> visited(graph.size(), false);
    distances[startVertex] = 0;

    using VertexDistPair = std::pair<int, size_t>;
    std::priority_queue<VertexDistPair, std::vector<VertexDistPair>, std::greater<VertexDistPair>> minHeap;
    minHeap.emplace(0, startVertex);

    while (!minHeap.empty()) {
        auto [currentDist, currentVertex] = minHeap.top();
        minHeap.pop();

        if (currentDist > distances[currentVertex] || visited[currentVertex]) {
            continue;
        }
        visited[currentVertex] = true;

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            size_t neighbor = edge.to;
            if (visited[neighbor]) {
                continue;
            }

            int newDist = currentDist + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                minHeap.emplace(newDist, neighbor);
            }
        }
    }

    return distances;
}

std::vector<int> ShortestPath::eagerDijkstra(const IGraphStructure& graph, size_t startVertex) {
    std::vector<int> distances(graph.size(), INF);
    std::vector<bool> visited(graph.size(), false);
    distances[startVertex] = 0;

    IndexedPriorityQueue minHeap(graph.size());
    minHeap.insert(startVertex, 0);

    while (!minHeap.isEmpty()) {
        size_t currentVertex = minHeap.popMin();
        if (visited[currentVertex]) {
            continue;
        }
        visited[currentVertex] = true;

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            size_t neighbor = edge.to;
            if (visited[neighbor]) {
                continue;
            }

            int newDist = distances[currentVertex] + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                minHeap.insert(neighbor, newDist);
            }
        }
    }

    return distances;
}

std::vector<int> ShortestPath::eagerDijkstraBoost(const IGraphStructure& graph, size_t startVertex) {
#ifdef SHORTESTPATH_HAS_BOOST_HEAP
    struct HeapNode {
        int distance;
        size_t vertex;
    };
    struct HeapNodeCompare {
        bool operator()(const HeapNode& lhs, const HeapNode& rhs) const {
            return lhs.distance > rhs.distance;
        }
    };

    std::vector<int> distances(graph.size(), INF);
    std::vector<bool> visited(graph.size(), false);
    distances[startVertex] = 0;

    using FibonacciHeap = boost::heap::fibonacci_heap<HeapNode, boost::heap::compare<HeapNodeCompare>>;
    FibonacciHeap minHeap;
    std::vector<typename FibonacciHeap::handle_type> handles(graph.size());
    std::vector<bool> inHeap(graph.size(), false);

    handles[startVertex] = minHeap.push({0, startVertex});
    inHeap[startVertex] = true;

    while (!minHeap.empty()) {
        const HeapNode current = minHeap.top();
        minHeap.pop();

        const size_t currentVertex = current.vertex;
        if (visited[currentVertex]) {
            continue;
        }
        visited[currentVertex] = true;
        inHeap[currentVertex] = false;

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            const size_t neighbor = edge.to;
            if (visited[neighbor]) {
                continue;
            }

            const int newDist = distances[currentVertex] + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;

                if (!inHeap[neighbor]) {
                    handles[neighbor] = minHeap.push({newDist, neighbor});
                    inHeap[neighbor] = true;
                } else {
                    (*handles[neighbor]).distance = newDist;
                    minHeap.decrease(handles[neighbor]);
                }
            }
        }
    }

    return distances;
#else
    return eagerDijkstra(graph, startVertex);
#endif
}

std::vector<int> ShortestPath::dijkstraWithDaryHeap(const IGraphStructure& graph, size_t startVertex) {
    std::vector<int> distances(graph.size(), INF);
    std::vector<bool> visited(graph.size(), false);
    distances[startVertex] = 0;

    size_t edgeCount = graph.getEdgeCount();
    size_t vertexCount = graph.size();
    size_t d = vertexCount > 0 ? (edgeCount / vertexCount) : 2;
    if (d < 2) {
        d = 2;
    }

    IndexedDaryHeap minHeap(graph.size(), d);
    minHeap.insert(startVertex, 0);

    while (!minHeap.isEmpty()) {
        size_t currentVertex = minHeap.popMin();
        if (visited[currentVertex]) {
            continue;
        }
        visited[currentVertex] = true;

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            size_t neighbor = edge.to;
            if (visited[neighbor]) {
                continue;
            }

            int newDist = distances[currentVertex] + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                minHeap.insert(neighbor, newDist);
            }
        }
    }

    return distances;
}

std::vector<int> ShortestPath::dijkstraWithBoostDaryHeap(const IGraphStructure& graph, size_t startVertex) {
#ifdef SHORTESTPATH_HAS_BOOST_HEAP
    struct HeapNode {
        int distance;
        size_t vertex;
    };
    struct HeapNodeCompare {
        bool operator()(const HeapNode& lhs, const HeapNode& rhs) const {
            return lhs.distance > rhs.distance;
        }
    };

    std::vector<int> distances(graph.size(), INF);
    std::vector<bool> visited(graph.size(), false);
    distances[startVertex] = 0;

    using DaryHeap = boost::heap::d_ary_heap<
        HeapNode,
        boost::heap::arity<4>,
        boost::heap::mutable_<true>,
        boost::heap::compare<HeapNodeCompare>
    >;

    DaryHeap minHeap;
    std::vector<typename DaryHeap::handle_type> handles(graph.size());
    std::vector<bool> inHeap(graph.size(), false);

    handles[startVertex] = minHeap.push({0, startVertex});
    inHeap[startVertex] = true;

    while (!minHeap.empty()) {
        const HeapNode current = minHeap.top();
        minHeap.pop();

        const size_t currentVertex = current.vertex;
        if (visited[currentVertex]) {
            continue;
        }
        visited[currentVertex] = true;
        inHeap[currentVertex] = false;

        for (const auto& edge : graph.getEdgesFromVertex(currentVertex)) {
            const size_t neighbor = edge.to;
            if (visited[neighbor]) {
                continue;
            }

            const int newDist = distances[currentVertex] + edge.weight;
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;

                if (!inHeap[neighbor]) {
                    handles[neighbor] = minHeap.push({newDist, neighbor});
                    inHeap[neighbor] = true;
                } else {
                    (*handles[neighbor]).distance = newDist;
                    minHeap.decrease(handles[neighbor]);
                }
            }
        }
    }

    return distances;
#else
    return dijkstraWithDaryHeap(graph, startVertex);
#endif
}

std::vector<int> ShortestPath::bellmanFord(const IGraphStructure& graph, size_t startVertex) {
    std::vector<int> distances(graph.size(), INF);
    if (graph.size() == 0) {
        return distances;
    }

    distances[startVertex] = 0;

    for (size_t i = 0; i + 1 < graph.size(); ++i) {
        bool changed = false;
        for (const auto& edge : graph.getEdges()) {
            if (distances[edge.from] != INF && distances[edge.from] + edge.weight < distances[edge.to]) {
                distances[edge.to] = distances[edge.from] + edge.weight;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
    }

    std::queue<size_t> negVertices;
    for (const auto& edge : graph.getEdges()) {
        if (distances[edge.from] != INF && distances[edge.from] + edge.weight < distances[edge.to]) {
            distances[edge.to] = NEG_INF;
            negVertices.push(edge.to);
        }
    }

    while (!negVertices.empty()) {
        size_t vertex = negVertices.front();
        negVertices.pop();
        for (const auto& edge : graph.getEdgesFromVertex(vertex)) {
            if (distances[edge.to] != NEG_INF) {
                distances[edge.to] = NEG_INF;
                negVertices.push(edge.to);
            }
        }
    }

    return distances;
}

std::vector<std::vector<int>> ShortestPath::floydWarshall(const IGraphStructure& graph) {
    std::vector<std::vector<int>> distances(graph.size(), std::vector<int>(graph.size(), INF));

    for (size_t i = 0; i < graph.size(); ++i) {
        distances[i][i] = 0;
    }

    for (const auto& edge : graph.getEdges()) {
        if (edge.weight < distances[edge.from][edge.to]) {
            distances[edge.from][edge.to] = edge.weight;
        }
    }

    for (size_t k = 0; k < graph.size(); ++k) {
        for (size_t i = 0; i < graph.size(); ++i) {
            if (distances[i][k] == INF) {
                continue;
            }
            for (size_t j = 0; j < graph.size(); ++j) {
                if (distances[k][j] == INF) {
                    continue;
                }
                int throughK = distances[i][k] + distances[k][j];
                if (throughK < distances[i][j]) {
                    distances[i][j] = throughK;
                }
            }
        }
    }

    return distances;
}
