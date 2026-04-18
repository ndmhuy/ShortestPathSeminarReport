#ifndef SHORTESTPATH_H
#define SHORTESTPATH_H

#include "IGraphStructure.h"
#include <vector>

class ShortestPath {
    public:
    static bool isBoostHeapAvailable();
    static std::vector<int> dagShortestPath(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> dagShortestPathWithTopoOrder(
        const IGraphStructure& graph,
        size_t startVertex,
        const std::vector<size_t>& topoOrder
    );
    static std::vector<int> dijkstra(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> eagerDijkstra(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> eagerDijkstraBoost(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> dijkstraWithDaryHeap(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> dijkstraWithBoostDaryHeap(const IGraphStructure& graph, size_t startVertex);
    static std::vector<int> bellmanFord(const IGraphStructure& graph, size_t startVertex);
    static std::vector<std::vector<int>> floydWarshall(const IGraphStructure& graph);
};

#endif // SHORTESTPATH_H