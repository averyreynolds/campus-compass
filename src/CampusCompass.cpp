#include "CampusCompass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <algorithm>
#include <climits>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // edges.csv Format
    {
        ifstream f(edges_filepath);
        if (!f.is_open()) return false;

        string line;
        getline(f, line);
        while (getline(f, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string temp, name1, name2;
            int id1, id2, time;

            try {
                getline(ss, temp, ',');
                id1 = stoi(temp);
                getline(ss, temp, ',');
                id2 = stoi(temp);
                getline(ss, name1, ',');
                getline(ss, name2, ',');
                getline(ss, temp, ',');
                time = stoi(temp);
            } catch (...) {
                continue;
            }

            locationNames[id1] = name1;
            locationNames[id2] = name2;
            locationIds.insert(id1);
            locationIds.insert(id2);

            adjList[id1].push_back({id2, time, false});
            adjList[id2].push_back({id1, time, false});
        }
    }

    // classes.csv Format
    {
        ifstream f(classes_filepath);
        if (!f.is_open()) return false;

        string line;
        getline(f, line);

        while (getline(f, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string classCode, temp;
            int locationId;

            try {
                getline(ss, classCode, ',');
                getline(ss, temp, ',');
                locationId = stoi(temp);
            } catch (...) {
                continue;
            }
            classLocations[classCode] = locationId;
        }
    }
    return true;
}

bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    // hint: return a boolean for validation when testing. For example:
    if (command.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    stringstream ss(command);
    string input;
    ss >> input;

    if (input == "insert") insert(command);
    else if (input == "remove") remove(ss);
    else if (input == "dropClass") dropClass(ss);
    else if (input == "replaceClass") replaceClass(ss);
    else if (input == "removeClass") removeClass(ss);
    else if (input == "toggleEdgesClosure") toggleEdgesClosure(ss);
    else if (input == "checkEdgeStatus") checkEdgeStatus(ss);
    else if (input == "isConnected") isConnected(ss);
    else if (input == "printShortestEdges") printShortestEdges(ss);
    else if (input == "printStudentZone") printStudentZone(ss);
    else {
        cout << "unsuccessful" << endl;
        return false;
    }
    return true;
}

bool CampusCompass::isValidStudentId(const string &s) {
    if (s.length() != 8) return false;
    for (const char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool CampusCompass::isValidName(const string &s) {
    if (s.empty()) return false;
    for (const char c : s) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool CampusCompass::isValidClassCode(const string &s) {
    if (s.length() != 7) return false;
    for (int i = 0; i < 3; i++) {
        if (!isupper(s[i])) return false;
    }
    for (int i = 3; i < 7; i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

bool CampusCompass::locationExists(int id) const {
    if (locationIds.count(id) > 0) return true;
    return false;
}

pair<unordered_map<int, int>, unordered_map<int, int> > CampusCompass::dijkstra(int src) const {
    unordered_map<int, int> dist, prev;
    for (int id : locationIds) {
        dist[id] = INT_MAX;
        prev[id] = -1;
    }
    dist[src] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.emplace(0, src);

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;

        auto it = adjList.find(u);
        if (it == adjList.end()) continue;

        for (const Edge& e : it->second) {
            if (e.closed) continue;
            int v = e.to;
            int newDist = dist[u] + e.weight;
            if (newDist < dist[v]) {
                dist[v] = newDist;
                prev[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    return {dist, prev};
}

vector<int> CampusCompass::reconstructPath(int src, int dest, const unordered_map<int, int> &prev) const {
    vector<int> path;
    int curr = dest;
    while (curr != -1) {
        path.push_back(curr);
        auto it = prev.find(curr);
        if (it == prev.end()) break;
        curr = it->second;
    }
    reverse(path.begin(), path.end());
    if (path.empty() || path[0] != src) return {};
    return path;
}

int CampusCompass::primMST(const set<int> &nodes, const unordered_map<int, vector<Edge>>& subgraph) const {
    if (nodes.empty()) return 0;

    unordered_map<int, int> key;
    unordered_map<int, bool> included;
    for (int n : nodes) {
        key[n] = INT_MAX;
        included[n] = false;
    }

    int start = *nodes.begin();
    key[start];

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.emplace(0, start);

    int total = 0;
    while (!pq.empty()) {
        auto [w, u] = pq.top();
        pq.pop();

        if (included[u]) continue;
        included[u] = true;
        total += w;

        auto it = subgraph.find(u);
        if (it == subgraph.end()) continue;

        for (const Edge& e : it->second) {
            int v = e.to;
            if (nodes.count(v) && !included[v] && e.weight < key[v]) {
                key[v] = e.weight;
                pq.push({e.weight, v});
            }
        }

    }
    return total;
}

// Commands

void CampusCompass::insert(const string &fullLine) {
    size_t quote1 = fullLine.find('"');
    size_t quote2 = (quote1 == string::npos) ? string::npos : fullLine.find('"', quote1 + 1);
    if (quote1 == string::npos || quote2 == string::npos) {
        cout << "unsuccessful" << endl;
        return;
    }
    string name = fullLine.substr(quote1 + 1, quote2 - quote1 - 1);

    if (!isValidName(name)) {
        cout << "unsuccessful" << endl;
        return;
    }

    stringstream rest(fullLine.substr(quote2 + 1));
    string idStr;
    int residenceId, N;

    if (!(rest >> idStr >> residenceId >> N)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }

    int studentId = stoi(idStr);

    if (students.count(studentId)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (N < 1 || N > 6) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!locationExists(residenceId)) {
        cout << "unsuccessful" << endl;
        return;
    }

    vector<string> codes;
    for (int i = 0; i < N; i++) {
        string code;
        if (!(rest >> code)) {
            cout << "unsuccessful" << endl;
            return;
        }
        if (!isValidClassCode(code)) {
            cout << "unsuccessful" << endl;
            return;
        }
        if (!classLocations.count(code)) {
            cout << "unsuccessful" << endl;
            return;
        }
        if (find(codes.begin(), codes.end(), code) != codes.end()) {
            cout << "unsuccessful" << endl;
            return;
        }
        codes.push_back(code);
    }

    Student s;
    s.name               = name;
    s.id                 = studentId;
    s.residenceLocationId = residenceId;
    s.classes            = codes;
    students[studentId] = s;

    cout << "successful" << endl;
}

void CampusCompass::remove(stringstream &ss) {
    string idStr;
    if (!(ss >> idStr) || !isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }
    int id = stoi(idStr);
    if (!students.count(id)) {
        cout << "unsuccessful" << endl;
        return;
    }
    students.erase(id);
    cout << "successful" << endl;
}

void CampusCompass::dropClass(stringstream& ss) {
    string idStr, code;
    if (!(ss >> idStr >> code)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }
    int id = stoi(idStr);
    if (!studentList.count(id)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!classLocations.count(code)) {
        cout << "unsuccessful" << endl;
        return;
    }
    Student& s = students[id];
    auto it = find(s.classes.begin(), s.classes.end(), code);
    if (it == s.classes.end()) {
        cout << "unsuccessful" << endl;
        return;
    }
    s.classes.erase(it);
    cout << "successful" << endl;
    if (s.classes.empty()) students.erase(id);
}

void CampusCompass::replaceClass(stringstream& ss) {
    string idStr, code1, code2;
    if (!(ss >> idStr >> code1 >> code2)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!isValidClassCode(code1) || !isValidClassCode(code2)) {
        cout << "unsuccessful" << endl;
        return;
    }
    int id = stoi(idStr);
    if (!students.count(id)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!classLocations.count(code2)) {
        cout << "unsuccessful" << endl;
        return;
    }
    Student& s = students[id];
    auto it1 = find(s.classes.begin(), s.classes.end(), code1);
    if (it1 == s.classes.end()) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (find(s.classes.begin(), s.classes.end(), code2) != s.classes.end()) {
        cout << "unsuccessful" << endl;
        return;
    }
    *it1 = code2;
    cout << "successful" << endl;
}

void CampusCompass::removeClass(stringstream& ss) {
    string code;
    if (!(ss >> code)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!isValidClassCode(code)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!classLocations.count(code)) {
        cout << "unsuccessful" << endl;
        return;
    }

    int count = 0;
    vector<int> toErase;

    for (auto& [id, s] : students) {
        auto it = find(s.classes.begin(), s.classes.end(), code);
        if (it != s.classes.end()) {
            s.classes.erase(it);
            count++;
            if (s.classes.empty()) toErase.push_back(id);
        }
    }

    if (count == 0) {
        cout << "unsuccessful" << endl;
        return;
    }
    for (int id : toErase) students.erase(id);
    cout << count << endl;
}

void CampusCompass::toggleEdgesClosure(stringstream& ss) {
    int N;
    if (!(ss >> N)) {
        cout << "unsuccessful" << endl;
        return;
    }
    for (int i = 0; i < N; i++) {
        int a, b;
        if (!(ss >> a >> b)) {
            cout << "unsuccessful" << endl;
            return;
        }
        for (Edge& e : adjList[a])
            if (e.to == b) { e.closed = !e.closed; break; }
        for (Edge& e : adjList[b])
            if (e.to == a) { e.closed = !e.closed; break; }
    }
    cout << "successful" << endl;
}

void CampusCompass::checkEdgeStatus(stringstream& ss) {
    int a, b;
    if (!(ss >> a >> b)) {
        cout << "unsuccessful" << endl;
        return;
    }
    auto it = adjList.find(a);
    if (it != adjList.end()) {
        for (const Edge& e : it->second) {
            if (e.to == b) {
                cout << (e.closed ? "closed" : "open") << endl;
                return;
            }
        }
    }
    cout << "DNE" << endl;
}

void CampusCompass::isConnected(stringstream& ss) {
    int a, b;
    if (!(ss >> a >> b)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (!locationExists(a) || !locationExists(b)) {
        cout << "unsuccessful" << endl;
        return;
    }
    if (a == b) {
        cout << "successful" << endl;
        return;
    }

    // Run BFS
    unordered_map<int,bool> visited;
    queue<int> q;
    q.push(a);
    visited[a] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        auto it = adjList.find(u);
        if (it == adjList.end()) continue;
        for (const Edge& e : it->second) {
            if (e.closed) continue;
            if (e.to == b) {
                cout << "successful" << endl;
                return;
            }
            if (!visited[e.to]) {
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }
    cout << "unsuccessful" << endl;
}

void CampusCompass::printShortestEdges(stringstream& ss) {
    string idStr;
    if (!(ss >> idStr) || !isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }
    int id = stoi(idStr);
    if (!students.count(id)) {
        cout << "unsuccessful" << endl;
        return;
    }

    const Student& s = students[id];
    auto [dist, prev] = dijkstra(s.residenceLocationId);

    cout << "Time For Shortest Edges: " << s.name << endl;

    vector<string> sorted = s.classes;
    sort(sorted.begin(), sorted.end());

    for (const string& code : sorted) {
        int locID = classLocations.at(code);
        int d = (dist.count(locID) && dist.at(locID) != INT_MAX) ? dist.at(locID) : -1;
        cout << code << ": " << d << endl;
    }
}

void CampusCompass::printStudentZone(stringstream& ss) {
    string idStr;
    if (!(ss >> idStr) || !isValidStudentId(idStr)) {
        cout << "unsuccessful" << endl;
        return;
    }
    int id = stoi(idStr);
    if (!students.count(id)) {
        cout << "unsuccessful" << endl;
        return;
    }

    const Student& s = students[id];
    auto [dist, prev] = dijkstra(s.residenceLocationId);

    // Step 1: Node collection from shortest paths
    set<int> nodeSet;
    nodeSet.insert(s.residenceLocationId);

    for (const string& code : s.classes) {
        int locID = classLocations.at(code);
        if (!dist.count(locID) || dist.at(locID) == INT_MAX) continue;
        vector<int> path = reconstructPath(s.residenceLocationId, locID, prev);
        for (int node : path) nodeSet.insert(node);
    }

    // Step 2: building the subgraph
    unordered_map<int, vector<Edge>> subgraph;
    for (int u : nodeSet) {
        auto it = adjList.find(u);
        if (it == adjList.end()) continue;
        for (const Edge& e : it->second) {
            if (!e.closed && nodeSet.count(e.to)) {
                subgraph[u].push_back(e);
            }
        }
    }

    // Step 3: MST of sub-graph using Prim's
    int cost = primMST(nodeSet, subgraph);

    cout << "Student Zone Cost For " << s.name << ": " << cost << endl;
}