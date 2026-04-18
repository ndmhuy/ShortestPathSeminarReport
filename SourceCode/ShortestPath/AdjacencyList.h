#ifndef ADJACENCYLIST_H
#define ADJACENCYLIST_H

#include "IGraphStructure.h"
#include <unordered_map>
#include <list>

class AdjacencyList : public IGraphStructure {
    private:
    bool isDirected;

    struct GraphNode {
        size_t vertex;
        int weight;

        GraphNode(size_t v, int w = 1) : vertex(v), weight(w) {}
    };

    std::unordered_map<size_t, std::list<GraphNode>> adjacencyList;

    
    public:
    AdjacencyList(bool directed = false);

    void addVertex(size_t vertex) override;
    void addEdge(size_t from, size_t to, int weight = 1) override;
    void deleteEdge(size_t from, size_t to) override;
    bool hasEdge(size_t from, size_t to) const override;
    
    std::vector<size_t> getVertices() const override;
    std::vector<Edge> getEdges() const override;

    std::vector<size_t> getNeighbours(size_t vertex) const override;
    std::vector<Edge> getEdgesFromVertex(size_t vertex) const override;
};

#endif // ADJACENCYLIST_H