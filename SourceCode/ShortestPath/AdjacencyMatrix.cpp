#include "AdjacencyMatrix.h"
#include <algorithm>

AdjacencyMatrix::AdjacencyMatrix(bool directed) : isDirected(directed) { vertexCount = 0; }

void AdjacencyMatrix::addVertex(size_t vertex) {
    if (vertex >= vertexCount) {
        resizeMatrix(vertex + 1);
    }
}

void AdjacencyMatrix::resizeMatrix(size_t newSize) {
    matrix.resize(newSize);
    for (auto& row : matrix) {
        row.resize(newSize, 0);
    }
}

bool AdjacencyMatrix::isValidVertex(size_t vertex) const {
    return vertex < vertexCount;
}

void AdjacencyMatrix::addEdge(size_t from, size_t to, int weight) {
    if (from >= vertexCount || to >= vertexCount) {
        resizeMatrix(std::max(from, to) + 1);
        vertexCount = matrix.size();
    }

    matrix[from][to] = weight;
    if (!isDirected) {
        matrix[to][from] = weight;
    }
}

void AdjacencyMatrix::deleteEdge(size_t from, size_t to) {
    if (isValidVertex(from) && isValidVertex(to)) {
        matrix[from][to] = 0;
        if (!isDirected) {
            matrix[to][from] = 0;
        }
    }
}

bool AdjacencyMatrix::hasEdge(size_t from, size_t to) const {
    if (isValidVertex(from) && isValidVertex(to)) {
        return matrix[from][to] != 0;
    }
    return false;
}

std::vector<size_t> AdjacencyMatrix::getVertices() const {
    std::vector<size_t> vertices;
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices.push_back(i);
    }
    return vertices;
}

std::vector<Edge> AdjacencyMatrix::getEdges() const {
    std::vector<Edge> edges;
    for (size_t i = 0; i < vertexCount; ++i) {
        for (size_t j = 0; j < vertexCount; ++j) {
            if (matrix[i][j] != 0) {
                edges.emplace_back(i, j, matrix[i][j]);
            }
        }
    }
    return edges;
}

std::vector<size_t> AdjacencyMatrix::getNeighbours(size_t vertex) const {
    std::vector<size_t> neighbours;
    if (isValidVertex(vertex)) {
        for (size_t j = 0; j < vertexCount; ++j) {
            if (matrix[vertex][j] != 0) {
                neighbours.push_back(j);
            }
        }
    }
    return neighbours;
}

std::vector<Edge> AdjacencyMatrix::getEdgesFromVertex(size_t vertex) const {
    std::vector<Edge> edges;
    if (isValidVertex(vertex)) {
        for (size_t j = 0; j < vertexCount; ++j) {
            if (matrix[vertex][j] != 0) {
                edges.emplace_back(vertex, j, matrix[vertex][j]);
            }
        }
    }
    return edges;
}