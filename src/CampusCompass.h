#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

struct Student {
    string name;
    int id;
    int residenceLocationId;
    vector<string> classes;
};

struct Edge {
    int to;
    int weight;
    bool closed = false;
};

class CampusCompass {
private:
    // Think about what member variables you need to initialize
    // perhaps some graph representation?
    unordered_map<int, string> locationNames;
    unordered_map<int, vector<Edge>> adjList;
    set<int> locationIds;
    unordered_map<int, string> studentList;

    // if doing EC
    unordered_map<string, int> classLocations;

    // Necessary Validation
    static bool isValidStudentId(const string& s) ;
    static bool isValidName(const string& s) ;
    static bool isValidClassCode(const string& s) ;
    bool locationExists(int id) const;

    // Helpers <printStudentZone>:
    pair<unordered_map<int, int>, unordered_map<int, int>> dijkstra(int src) const;

    vector<int> reconstructPath(int src, int dest, const unordered_map<int, int>& prev) const;

    int primMST(const set<int>& nodes, const unordered_map<int, int>& subgraph) const;

    // Commands:
    void insert(const string& fullLine);
    void remove(stringstream& ss);
    void dropClass(stringstream& ss);
    void replaceClass(stringstream& ss);
    void removeClass(stringstream& ss);
    void toggleEdgesClosure(stringstream& ss);
    void checkEdgeStatus(stringstream& ss);
    void isConnected(stringstream& ss);
    void printShortestEdges(stringstream& ss);
    void printStudentZone(stringstream& ss);

public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
};
