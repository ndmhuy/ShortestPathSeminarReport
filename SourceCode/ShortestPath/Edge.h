#ifndef EDGE_H
#define EDGE_H

#include <cstddef>

struct Edge {
    size_t from;
    size_t to;
    int weight;

    Edge(size_t f, size_t t, int w = 1) : from(f), to(t), weight(w) {}
};

#endif // EDGE_H