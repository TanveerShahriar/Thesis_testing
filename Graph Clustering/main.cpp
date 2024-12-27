#include <igraph/igraph.h>
#include <iostream>

int main() {
    igraph_t graph;
    igraph_ring(&graph, 10, IGRAPH_UNDIRECTED, 0, 1); // Create a ring graph with 10 vertices

    std::cout << "Graph vertices: " << (int)igraph_vcount(&graph) << "\n";
    std::cout << "Graph edges: " << (int)igraph_ecount(&graph) << "\n";

    igraph_write_graph_edgelist(&graph, stdout); // Print the edges of the graph to stdout
    igraph_destroy(&graph); // Destroy the graph to free memory

    return 0;
}
