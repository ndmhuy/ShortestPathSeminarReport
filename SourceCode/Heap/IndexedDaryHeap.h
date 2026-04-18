#ifndef INDEXEDDARYHEAP_H
#define INDEXEDDARYHEAP_H

#include <vector>
#include <limits>
#include <stdexcept>

class IndexedDaryHeap {
private:
    size_t d;                      // The branching factor (degree)
    std::vector<size_t> heap;      // heap[i] stores the vertex ID
    std::vector<int> priorities;   // priorities[v] stores the distance of vertex v
    std::vector<int> indexMap;     // indexMap[v] stores the heap array index of vertex v

    void swap(size_t i, size_t j) {
        std::swap(heap[i], heap[j]);
        indexMap[heap[i]] = i;
        indexMap[heap[j]] = j;
    }

    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / d;
            
            // Stop if the current node is greater than or equal to its parent
            if (priorities[heap[i]] >= priorities[heap[parent]]) {
                break;
            }
            
            swap(i, parent);
            i = parent; // Move up to the parent's index
        }
    }

    void siftDown(size_t i) {
        size_t size = heap.size();
        while (true) {
            size_t smallest = i;

            // Loop through all 'd' children of the current node
            for (size_t k = 1; k <= d; ++k) {
                size_t child = d * i + k;
                
                // If the child index is out of bounds, we've reached the bottom
                if (child >= size) {
                    break; 
                }
                
                // Find the absolute smallest among the parent and all its children
                if (priorities[heap[child]] < priorities[heap[smallest]]) {
                    smallest = child;
                }
            }

            // If the parent is already the smallest, the heap property is restored
            if (smallest == i) {
                break;
            }

            swap(i, smallest);
            i = smallest; // Move down to the smallest child's index
        }
    }

public:
    // Constructor requires both the max capacity (vertices) and the degree 'd'
    IndexedDaryHeap(size_t capacity, size_t degree) 
        : d(degree), 
          priorities(capacity, std::numeric_limits<int>::max()), 
          indexMap(capacity, -1) {
        
        if (d < 2) throw std::invalid_argument("Degree 'd' must be at least 2.");
        heap.reserve(capacity);
    }

    bool isEmpty() const {
        return heap.empty();
    }

    bool contains(size_t vertex) const {
        return vertex < indexMap.size() && indexMap[vertex] != -1;
    }

    void insert(size_t vertex, int priority) {
        if (contains(vertex)) {
            decreaseKey(vertex, priority);
            increaseKey(vertex, priority);
            return;
        }
        
        priorities[vertex] = priority;
        indexMap[vertex] = heap.size();
        heap.push_back(vertex);
        siftUp(heap.size() - 1);
    }

    size_t popMin() {
        if (isEmpty()) throw std::out_of_range("Priority queue is empty");

        size_t minVertex = heap[0];
        
        swap(0, heap.size() - 1);
        heap.pop_back();
        indexMap[minVertex] = -1; 

        if (!heap.empty()) {
            siftDown(0);
        }

        return minVertex;
    }

    void decreaseKey(size_t vertex, int newPriority) {
        if (!contains(vertex)) return;
        
        if (newPriority < priorities[vertex]) {
            priorities[vertex] = newPriority;
            siftUp(indexMap[vertex]); 
        }
    }

    void increaseKey(size_t vertex, int newPriority) {
        if (!contains(vertex)) return;
        
        if (newPriority > priorities[vertex]) {
            priorities[vertex] = newPriority;
            siftDown(indexMap[vertex]); 
        }
    }
};

#endif // INDEXEDDARYHEAP_H