// ReferralGraph.cpp
// Task 6: directed department graph with DFS and BFS traversals.
// DSA Final Assessment - Hospital Management System.

#include "ReferralGraph.h"
#include <iostream>
#include <queue>
#include <stdexcept>
using namespace std;

// Create exactly the directed referral edges specified by FA Task 6.
ReferralGraph::ReferralGraph() : edges{{{1,2},{3,4},{3},{5},{5},{}}} {}
// Translate vertex indices A-F into department names.
string ReferralGraph::name(int vertex) {
    static const array<string,6> names = {"Emergency", "Cardiology", "Radiology", "Surgery", "Neurology", "Pharmacy"};
    return names.at(vertex);
}
// DFS helper: mark a vertex before recursively visiting its unvisited neighbours.
void ReferralGraph::visit(int vertex, array<bool,6>& visited, vector<int>& order) const {
    // Mark before following edges to prevent cycles/repeated visits.
    visited[vertex] = true;
    order.push_back(vertex);
    for (int next : edges[vertex]) if (!visited[next]) visit(next, visited, order);
}
// Depth-First Search: use recursive calls to explore one branch before the next.
vector<int> ReferralGraph::dfs(int start) const {
    if (start < 0 || start >= 6) throw out_of_range("Invalid graph vertex");
    array<bool,6> visited{};
    vector<int> order;
    visit(start, visited, order);
    return order;
}
// Breadth-First Search: use a FIFO queue to explore reachable vertices level by level.
vector<int> ReferralGraph::bfs(int start) const {
    if (start < 0 || start >= 6) throw out_of_range("Invalid graph vertex");
    array<bool,6> visited{};
    vector<int> order;
    queue<int> pending;
    pending.push(start);
    visited[start] = true;
    // FIFO makes earlier-discovered vertices expand before the next level.
    while (!pending.empty()) {
        int current = pending.front(); pending.pop(); order.push_back(current);
        for (int next : edges[current]) if (!visited[next]) {
            // Mark on enqueue so a vertex reached through two edges is queued only once.
            visited[next] = true; pending.push(next);
        }
    }
    return order;
}
// Display vertices and outgoing edges; referrals are not automatically bidirectional.
void ReferralGraph::display(ostream& out) const {
    for (int i = 0; i < 6; ++i) {
        out << char('A'+i) << " - " << name(i) << " -> ";
        for (int next : edges[i]) out << char('A'+next) << ' ';
        out << '\n';
    }
}
