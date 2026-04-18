#include "AdjacencyList.h"

AdjacencyList::AdjacencyList(bool directed) : isDirected(directed) { vertexCount = 0; }

void AdjacencyList::addVertex(size_t vertex) {
    if (adjacencyList.find(vertex) == adjacencyList.end()) {
        adjacencyList[vertex] = std::list<GraphNode>();
        vertexCount++;
    }
}

void AdjacencyList::addEdge(size_t from, size_t to, int weight) {
    adjacencyList[from];
    adjacencyList[to];
    
    if (!hasEdge(from, to)) {
        adjacencyList[from].emplace_back(to, weight);
        if (!isDirected) {
            adjacencyList[to].emplace_back(from, weight);
        }
    } else { // Update weight if edge already exists
        auto &fromList = adjacencyList[from];
        for (auto &node : fromList) {
            if (node.vertex == to) {
                node.weight = weight;
                break;
            }
        }
        if (!isDirected) {
            auto &toList = adjacencyList[to];
            for (auto &node : toList) {
                if (node.vertex == from) {
                    node.weight = weight;
                    break;
                }
            }
        }
    }
    vertexCount = adjacencyList.size();
}

void AdjacencyList::deleteEdge(size_t from, size_t to) {
    if (hasEdge(from, to)) {
        adjacencyList[from].remove_if([to](const GraphNode& node) { return node.vertex == to;});
        if (!isDirected) {
            adjacencyList[to].remove_if([from](const GraphNode& node) { return node.vertex == from;});
        }
    }
}

bool AdjacencyList::hasEdge(size_t from, size_t to) const {
    auto it = adjacencyList.find(from);
    if (it != adjacencyList.end()) {
        for (const auto& node : it->second) {
            if (node.vertex == to) {
                return true;
            }
        }
    }
    return false;
}

std::vector<size_t> AdjacencyList::getVertices() const {
    std::vector<size_t> vertices;
    for (const auto& pair : adjacencyList) {
        vertices.push_back(pair.first);
    }
    return vertices;
}

// may return only one side in case of undirected graph if renderer needs
std::vector<Edge> AdjacencyList::getEdges() const {
    std::vector<Edge> edges;
    for (const auto& pair : adjacencyList) {
        size_t from = pair.first;
        for (const auto& node : pair.second) {
            edges.emplace_back(from, node.vertex, node.weight);
        }
    }
    return edges;
}

std::vector<size_t> AdjacencyList::getNeighbours(size_t vertex) const {
    std::vector<size_t> neighbours;
    auto it = adjacencyList.find(vertex);
    if (it != adjacencyList.end()) {
        for (const auto& node : it->second) {
            neighbours.push_back(node.vertex);
        }
    }
    return neighbours;
}

std::vector<Edge> AdjacencyList::getEdgesFromVertex(size_t vertex) const {
    std::vector<Edge> edges;
    auto it = adjacencyList.find(vertex);
    if (it != adjacencyList.end()) {
        for (const auto& node : it->second) {
            edges.emplace_back(vertex, node.vertex, node.weight);
        }
    }
    return edges;
}