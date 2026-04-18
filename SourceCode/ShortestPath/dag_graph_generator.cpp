#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

// Pack two 32-bit integers into one 64-bit key for fast dedup.
inline uint64_t pack_edge(uint32_t u, uint32_t v) {
    return (static_cast<uint64_t>(u) << 32) | v;
}

int main(int argc, char* argv[]) {
    // Usage: <vertices> <edges> <min_weight> <max_weight> <output_file>
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0]
                  << " <vertices> <edges> <min_weight> <max_weight> <output_file>\n";
        std::cerr << "Example: " << argv[0] << " 100000 500000 1 10000 dataset5_dag_sssp.txt\n";
        return 1;
    }

    const uint32_t V = static_cast<uint32_t>(std::stoul(argv[1]));
    const uint64_t E = std::stoull(argv[2]);
    const int min_weight = std::stoi(argv[3]);
    const int max_weight = std::stoi(argv[4]);
    const std::string filename = argv[5];

    const uint64_t max_possible_edges = static_cast<uint64_t>(V) * (V - 1) / 2;
    if (E > max_possible_edges) {
        std::cerr << "Error: Requested edges (" << E
                  << ") exceeds max DAG edges for " << V << " vertices: "
                  << max_possible_edges << "\n";
        return 1;
    }

    std::cout << "Generating DAG with " << V << " vertices and " << E << " edges...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist_w(min_weight, max_weight);
    std::uniform_int_distribution<uint32_t> dist_pos(0, V - 1);

    // Random topological order.
    std::vector<uint32_t> topo(V);
    for (uint32_t i = 0; i < V; ++i) {
        topo[i] = i;
    }
    std::shuffle(topo.begin(), topo.end(), rng);

    // Map vertex -> position in topological order.
    std::vector<uint32_t> pos(V);
    for (uint32_t i = 0; i < V; ++i) {
        pos[topo[i]] = i;
    }

    std::unordered_set<uint64_t> existing_edges;
    existing_edges.reserve(static_cast<size_t>(E * 1.1));

    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing.\n";
        return 1;
    }

    const std::string dataset_name = std::filesystem::path(filename).stem().string();
    outfile << "# Directed " << dataset_name << " graph dataset\n";
    outfile << "# Nodes: " << V << " Edges: " << E << "\n";
    outfile << "# FromNodeId\tToNodeId\n";

    uint64_t edges_created = 0;
    while (edges_created < E) {
        uint32_t a = dist_pos(rng);
        uint32_t b = dist_pos(rng);
        if (a == b) {
            continue;
        }

        // Enforce topological direction: position(a) < position(b).
        if (a > b) {
            std::swap(a, b);
        }

        const uint32_t u = topo[a];
        const uint32_t v = topo[b];

        const uint64_t packed = pack_edge(u, v);
        if (!existing_edges.insert(packed).second) {
            continue;
        }

        const int weight = dist_w(rng);
        outfile << u << " " << v << " " << weight << "\n";
        ++edges_created;

        const uint64_t step = (E / 10 == 0) ? 1 : (E / 10);
        if (edges_created % step == 0) {
            std::cout << (edges_created * 100) / E << "% complete...\n";
        }
    }

    outfile.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "DAG successfully written to " << filename << " in " << elapsed.count()
              << " seconds.\n";

    return 0;
}
