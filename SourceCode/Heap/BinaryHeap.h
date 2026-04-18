#include <vector>

class BinaryHeap {
    private:
    int heapSize;
    int capacity;
    std::vector<int> heap;

    public:
    BinaryHeap(int cap = 16) : heapSize(0), capacity(cap) {
        heap.resize(capacity);
    }

    void insert(int value) {
        if (heapSize == capacity) {
            resize();
        }

        heap[heapSize] = value;
        ++heapSize;
        heapifyUp(heapSize - 1);
    }

    int pop() {
        if (heapSize == 0) return -1;

        int value = heap[0];
        heap[0] = heap[heapSize - 1];
        --heapSize;
        heapifyDown(0);
        return value;
    }

    int top() const {
        if (heapSize == 0) return;
        return heap[0];
    }

    private:
    void resize() {
        capacity *= 2;
        heap.resize(capacity);
    }

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index] > heap[parent]) {
                std::swap(heap[index], heap[parent]);
                index = parent;
            } else break;
        }
    }

    void heapifyDown(int index) {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heapSize && heap[left] > heap[largest]) {
            largest = left;
        }

        if (right < heapSize && heap[right] > heap[largest]) {
            largest = right;
        }

        if (largest != index) {
            std::swap(heap[index], heap[largest]);
            heapifyDown(largest);
        }
    }
};