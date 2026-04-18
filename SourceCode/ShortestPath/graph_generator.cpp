#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_set>
#include <string>
#include <chrono>
#include <filesystem>

// Helper to pack two 32-bit integers into a single 64-bit integer for blazing fast hashing
inline uint64_t pack_edge(uint32_t u, uint32_t v, bool directed) {
    if (!directed && u > v) {
        std::swap(u, v); // Normalize undirected edges so (u,v) == (v,u)
    }
    return (static_cast<uint64_t>(u) << 32) | v;
}

int main(int argc, char* argv[]) {
    // 1. Parse Command Line Arguments
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <vertices> <edges> <min_weight> <max_weight> <directed(1/0)> <output_file>\n";
        std::cerr << "Example: " << argv[0] << " 100000 500000 1 100 1 graph.txt\n";
        return 1;
    }

    uint32_t V = std::stoul(argv[1]);
    uint64_t E = std::stoull(argv[2]);
    int min_weight = std::stoi(argv[3]);
    int max_weight = std::stoi(argv[4]);
    bool is_directed = std::stoi(argv[5]) != 0;
    std::string filename = argv[6];

    // Safety check: Cannot have more edges than a complete graph allows
    uint64_t max_possible_edges = is_directed ? (static_cast<uint64_t>(V) * (V - 1)) : (static_cast<uint64_t>(V) * (V - 1) / 2);
    if (E > max_possible_edges) {
        std::cerr << "Error: Requested edges (" << E << ") exceeds max possible for " << V << " vertices.\n";
        return 1;
    }

    std::cout << "Generating " << (is_directed ? "directed" : "undirected") 
              << " graph with " << V << " vertices and " << E << " edges...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    // 2. Fast Random Number Generation Setup
    std::mt19937 rng(std::random_device{}()); // Use Mersenne Twister
    std::uniform_int_distribution<uint32_t> dist_v(0, V - 1);
    std::uniform_int_distribution<int> dist_w(min_weight, max_weight);

    // 3. Track existing edges to prevent duplicates
    std::unordered_set<uint64_t> existing_edges;
    existing_edges.reserve(E);

    // 4. Generate Edges
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing.\n";
        return 1;
    }

    // Write dataset-specific header metadata based on output filename stem.
    std::string dataset_name = std::filesystem::path(filename).stem().string();
    outfile << "# Directed " << dataset_name << " graph dataset\n";
    outfile << "# Nodes: " << V << " Edges: " << E << "\n";
    outfile << "# FromNodeId\tToNodeId\n";

    uint64_t edges_created = 0;
    while (edges_created < E) {
        uint32_t u = dist_v(rng);
        uint32_t v = dist_v(rng);

        // No self-loops
        if (u == v) continue;

        uint64_t packed = pack_edge(u, v, is_directed);

        // If edge doesn't exist, insert it and write to file
        if (existing_edges.insert(packed).second) {
            int weight = dist_w(rng);
            outfile << u << " " << v << " " << weight << "\n";
            edges_created++;

            if (edges_created % (E / 10 == 0 ? 1 : E / 10) == 0) {
                std::cout << (edges_created * 100) / E << "% complete...\n";
            }
        }
    }

    outfile.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    std::cout << "Graph successfully written to " << filename << " in " << elapsed.count() << " seconds.\n";

    return 0;
}