// ReferralGraph.h
// Task 6: directed department graph with DFS and BFS traversals.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <array>
#include <iostream>
#include <string>
#include <vector>
class ReferralGraph {
    std::array<std::vector<int>, 6> edges;
    // DFS helper: mark a vertex before recursively visiting its unvisited neighbours.
    void visit(int vertex, std::array<bool, 6>& visited, std::vector<int>& order) const;
public:
    // Create exactly the directed referral edges specified by FA Task 6.
    ReferralGraph();
    // Depth-First Search: use recursive calls to explore one branch before the next.
    std::vector<int> dfs(int start) const;
    // Breadth-First Search: use a FIFO queue to explore reachable vertices level by level.
    std::vector<int> bfs(int start) const;
    // Display vertices and outgoing edges; referrals are not automatically bidirectional.
    void display(std::ostream& out = std::cout) const;
    // Translate vertex indices A-F into department names.
    static std::string name(int vertex);
};
