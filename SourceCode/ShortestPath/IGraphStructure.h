#ifndef IGRAPHSTRUCTURE_H
#define IGRAPHSTRUCTURE_H

#include "Edge.h"
#include <vector>

class IGraphStructure {
    protected:
    size_t vertexCount;
    size_t edgeCount;

    public:
    virtual ~IGraphStructure() = default;

    size_t size() const { return vertexCount; }
    void setEdgeCount(size_t count) { edgeCount = count; }
    size_t getEdgeCount() const { return edgeCount; }
    
    bool empty() const { return vertexCount == 0; }

    virtual void addVertex(size_t vertex) = 0;
    virtual void addEdge(size_t from, size_t to, int weight = 1) = 0;
    virtual void deleteEdge(size_t from, size_t to) = 0;
    virtual bool hasEdge(size_t from, size_t to) const = 0;

    virtual std::vector<size_t> getVertices() const = 0;
    virtual std::vector<Edge> getEdges() const = 0;

    virtual std::vector<size_t> getNeighbours(size_t vertex) const = 0;
    virtual std::vector<Edge> getEdgesFromVertex(size_t vertex) const = 0;
};

#endif // IGRAPHSTRUCTURE_H
