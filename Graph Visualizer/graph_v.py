import json
import networkx as nx
import matplotlib.pyplot as plt

# Load the adjacency list from JSON
with open('adjacency_list.json', 'r') as file:
    adjacency_list = json.load(file)

# Create a directed graph (use nx.Graph() for an undirected graph)
G = nx.DiGraph()

# Add edges based on the adjacency list
for node, neighbors in adjacency_list.items():
    for neighbor in neighbors:
        G.add_edge(node, neighbor)

# Draw the graph
plt.figure(figsize=(8, 6))
nx.draw(G, with_labels=True, node_color="skyblue", font_weight="bold", node_size=2000, edge_color="gray")
plt.show()
