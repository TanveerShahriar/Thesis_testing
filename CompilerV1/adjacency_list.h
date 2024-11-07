#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H

#include<bits/stdc++.h>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

class AdjacencyList {
private:
    ofstream* outlog;

    // Map to store node names and their corresponding index
    unordered_map<string, int> nodeIndexMap;
    
    // Adjacency list: stores connections between node indices
    vector<vector<int>> adjList;

public:
    AdjacencyList(ofstream* log) {  
        this->outlog = log;              
    }

    void addNode(const string& nodeName) {
        if (nodeIndexMap.find(nodeName) == nodeIndexMap.end()) {
            int newIndex = nodeIndexMap.size();
            nodeIndexMap[nodeName] = newIndex;

            adjList.push_back(vector<int>());
        }
    }

    void addEdge(const string& node1, const string& node2) {
        addNode(node1);
        addNode(node2);

        int index1 = nodeIndexMap[node1];
        int index2 = nodeIndexMap[node2];

        adjList[index1].push_back(index2);
    }

    vector<string> getAdjNodes(const string& nodeName) const {
        vector<string> adjacentNodes;

        if (nodeIndexMap.find(nodeName) != nodeIndexMap.end()) {
            int nodeIndex = nodeIndexMap.at(nodeName);
            for (int adjIndex : adjList[nodeIndex]) {
                for (const auto& pair : nodeIndexMap) {
                    if (pair.second == adjIndex) {
                        adjacentNodes.push_back(pair.first);
                    }
                }
            }
        }

        return adjacentNodes;
    }

    void displayAdjacencyList() const {
        for (const auto& pair : nodeIndexMap) {
            string nodeName = pair.first;
            int nodeIndex = pair.second;
            *outlog << nodeName << ": ";
            for (int adjIndex : adjList[nodeIndex]) {
                for (const auto& nodePair : nodeIndexMap) {
                    if (nodePair.second == adjIndex) {
                        *outlog << nodePair.first << " ";
                        break;
                    }
                }
            }
            *outlog << endl;
        }
    }

    void printJSON() {
        // Reverse the map to get index-to-name mapping
        unordered_map<int, string> indexToName;
        for (const auto& pair : nodeIndexMap) {
            indexToName[pair.second] = pair.first;
        }

        ofstream JSONFile;
        JSONFile.open("../Graph Visualizer/adjacency_list.json", ios::trunc);

        // Create a JSON object to store the adjacency list with node names
        json jsonAdjList;

        // Populate the JSON object
        for (int i = 0; i < adjList.size(); ++i) {
            string nodeName = indexToName[i]; // Get the node name
            vector<string> neighbors;

            // Convert each neighbor index to its corresponding name
            for (int neighborIndex : adjList[i]) {
                neighbors.push_back(indexToName[neighborIndex]);
            }

            // Assign the neighbors list to the JSON object with the node name as the key
            jsonAdjList[nodeName] = neighbors;
        }

        // json j = adjList;
        JSONFile << jsonAdjList;
        JSONFile.close();
    }
};

#endif
