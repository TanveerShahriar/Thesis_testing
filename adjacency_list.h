#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H

#include<bits/stdc++.h>
using namespace std;

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
};

#endif
