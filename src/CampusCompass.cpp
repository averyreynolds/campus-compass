#include "CampusCompass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

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
