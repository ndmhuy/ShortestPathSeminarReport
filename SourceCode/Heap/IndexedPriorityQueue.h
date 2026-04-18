#ifndef INDEXEDPRIORITYQUEUE_H
#define INDEXEDPRIORITYQUEUE_H

#include <vector>
#include <limits>
#include <stdexcept>

class IndexedPriorityQueue {
private:
    // heap[i] stores the vertex ID located at heap position i.
    std::vector<size_t> heap; 
    
    // priorities[v] stores the current priority (distance) of vertex v.
    std::vector<int> priorities; 
    
    // indexMap[v] stores the heap position i of vertex v. -1 means not in heap.
    std::vector<int> indexMap; 

    // Helper: Swap two elements in the heap and update their inverse map
    void swap(size_t i, size_t j) {
        std::swap(heap[i], heap[j]);
        indexMap[heap[i]] = i;
        indexMap[heap[j]] = j;
    }

    // Helper: Move an element UP the tree to restore Min-Heap property
    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            // If the current element is greater than or equal to its parent, stop.
            if (priorities[heap[i]] >= priorities[heap[parent]]) {
                break;
            }
            swap(i, parent);
            i = parent; // Move up
        }
    }

    // Helper: Move an element DOWN the tree to restore Min-Heap property
    void siftDown(size_t i) {
        size_t size = heap.size();
        while (true) {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            size_t smallest = i;

            // Find the smallest among node, left child, and right child
            if (left < size && priorities[heap[left]] < priorities[heap[smallest]]) {
                smallest = left;
            }
            if (right < size && priorities[heap[right]] < priorities[heap[smallest]]) {
                smallest = right;
            }
            // If the node is smaller than both children, we are done
            if (smallest == i) {
                break;
            }
            swap(i, smallest);
            i = smallest; // Move down
        }
    }

public:
    // Initialize with the total number of vertices in your graph
    IndexedPriorityQueue(size_t capacity) 
        : priorities(capacity, std::numeric_limits<int>::max()), 
          indexMap(capacity, -1) {
        heap.reserve(capacity);
    }

    bool isEmpty() const {
        return heap.empty();
    }

    bool contains(size_t vertex) const {
        // A vertex is in the queue if its index is within bounds and not -1
        return vertex < indexMap.size() && indexMap[vertex] != -1;
    }

    // Insert a new vertex or update it if it already exists
    void insert(size_t vertex, int priority) {
        if (contains(vertex)) {
            decreaseKey(vertex, priority);
            increaseKey(vertex, priority);
            return;
        }
        
        // Add new vertex to the bottom of the heap
        priorities[vertex] = priority;
        indexMap[vertex] = heap.size();
        heap.push_back(vertex);
        
        // Bubble it up to its correct position
        siftUp(heap.size() - 1);
    }

    // Extract the vertex with the minimum priority (shortest distance)
    size_t popMin() {
        if (isEmpty()) throw std::out_of_range("Priority queue is empty");

        size_t minVertex = heap[0];
        
        // Swap the root with the last element
        swap(0, heap.size() - 1);
        
        // Pop the last element off the array
        heap.pop_back();
        indexMap[minVertex] = -1; // Mark as removed from the queue

        // Sink the new root down to its correct position
        if (!heap.empty()) {
            siftDown(0);
        }

        return minVertex;
    }

    // For Dijkstra, we only ever find shorter paths, so we only need decreaseKey
    void decreaseKey(size_t vertex, int newPriority) {
        if (!contains(vertex)) return;
        
        // Only update if the new priority is actually smaller
        if (newPriority < priorities[vertex]) {
            priorities[vertex] = newPriority;
            // Since the distance got smaller, the vertex floats UP the Min-Heap
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

#endif // INDEXEDPRIORITYQUEUE_H