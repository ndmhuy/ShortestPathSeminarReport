#ifndef ADJACENCYMATRIX_H
#define ADJACENCYMATRIX_H

#include "IGraphStructure.h"

class AdjacencyMatrix : public IGraphStructure {
    private:
    bool isDirected;
    std::vector<std::vector<int>> matrix;

    void resizeMatrix(size_t newSize);
    bool isValidVertex(size_t vertex) const;

    public:
    AdjacencyMatrix(bool directed = false);

    void addVertex(size_t vertex) override;
    void addEdge(size_t from, size_t to, int weight = 1) override;
    void deleteEdge(size_t from, size_t to) override;
    bool hasEdge(size_t from, size_t to) const override;

    std::vector<size_t> getVertices() const override;
    std::vector<Edge> getEdges() const override;

    std::vector<size_t> getNeighbours(size_t vertex) const override;
    std::vector<Edge> getEdgesFromVertex(size_t vertex) const override;
};

#endif // ADJACENCYMATRIX_H