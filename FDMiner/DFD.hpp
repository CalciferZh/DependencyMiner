#pragma once
#include <string>
#include <vector>
#include <stack>
#include <unordered_set>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>

#define PROPOGATE
#define RANDOM

typedef int NodeIndex;
typedef int ColIndex;
const std::vector<ColIndex> BITMAP({ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 });

struct Node {
    //std::vector<int> LHS;
    bool isVisited = false;
    bool isDep = false;
    bool isMinDep = false;
    bool isCandidateMinDep = true;
    bool isNonDep = false;
    bool isMaxNonDep = false;
    bool isCandidateMaxNonDep = true;
};

class DFD {
public:
    DFD(std::string path, int size = -1, int ncol = -1) : size(size), ncol(ncol) {
        if (size == -1) {
            //calculate
        }
        if (ncol == -1) {
            //calculate
        }
        tabu_for_unique_cols = 0;
        for (int i = 0; i < ncol; ++i) {
            data.push_back(std::vector<std::string>());
            data.at(i).reserve(size);
        }
        std::ifstream in(path);
        std::string buffer;
        if (!in.is_open()) {
            std::cout << "read file fail. Please check the path and retry/n";
        }
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < ncol - 1; ++j) {
                std::getline(in, buffer, ',');
                data.at(j).push_back(buffer);
            }
            std::getline(in, buffer);
            data.at(ncol - 1).push_back(buffer);
        }
        NodeSet.resize(BITMAP.at(ncol));
    }

    void output() {
        for (auto fd : FD) {
            std::string result;
            for (auto x : fd) {
                result += std::to_string(x);
                result += " ";
            }
            result.replace(result.rfind(' '), 1, "");
            result.replace(result.rfind(' '), 1, " -> ");
            std::cout << result << std::endl;
        }
    }

    void extraction() {
        for (int i = 0; i < ncol; ++i) {
            if (std::unordered_set<std::string>(data.at(i).begin(), data.at(i).end()).size() == data.at(i).size()) {
                for (int j = 0; j < ncol; ++j) {
                    if (j != i) {
                        FD.push_back(std::vector<int>({ i + 1, j + 1 }));
                    }
                }
                tabu_for_unique_cols |= BITMAP.at(i);
            } else {
                non_unique_cols.push_back(i);
            }
        }
        for (int i : non_unique_cols) {
            current_rhs = i;
            findLHSs();
            for (NodeIndex j : minDeps) {
                auto fd = getColIndexVector(j);
                for (auto& x : fd) x += 1;
                fd.push_back(i + 1);
                FD.push_back(fd);
            }
            std::fill(NodeSet.begin(), NodeSet.end(), Node());
            minDeps.clear();
            maxNonDeps.clear();
        }
        std::sort(FD.begin(), FD.end(), [](const std::vector<int>& lhs, const std::vector<int>& rhs) -> bool {
            auto iter1 = lhs.begin();
            auto iter2 = rhs.begin();
            while (iter1 != lhs.end() && iter2 != rhs.end()) {
                if (*iter1 < *iter2) return true;
                if (*iter1 > *iter2) return false;
                ++iter1;
                ++iter2;
            }
            return(iter2 != rhs.end()); // lhs shorter return true
        });
    }

    std::vector<std::vector<int>>& getFD() {
        return FD;
    }

private:
    std::vector<std::vector<std::string>> data;
    int size;
    int ncol;
    int tabu_for_unique_cols;
    ColIndex current_rhs;
    std::vector<std::vector<ColIndex>> FD;
    std::vector<ColIndex> non_unique_cols;
    std::vector<Node> NodeSet;
    std::vector<NodeIndex> minDeps;
    std::vector<NodeIndex> maxNonDeps;
    std::stack<NodeIndex> trace;

    int getRandom(std::set<int>& S) {
        int idx = rand() % S.size(); // not equal prob but acceptable
        auto iter = S.begin();
        while (idx--) {
            ++iter;
        }
        return *iter;
    }

    void subset(int n, std::set<int>& S, int depth = -1) { // optimization: just use 1 layer propogate now
        if (depth == -1) {
            depth = 32;
        }
        if (depth == 0 || n == 0)
            return;
        int nn = n;
        while (nn != 0) {
            int firstOne = nn & ~(nn - 1);
            if ((n & ~firstOne) == 0) break;
            nn = nn & ~firstOne;
            S.insert(n & ~firstOne);
            subset(n & ~firstOne, S, depth - 1);
        }
    }

    void superset(int n, std::set<int>& S, int tabu, int mask, int depth = -1) { // optimization: just use 1 layer propogate now
        if (depth == -1) {
            depth = 32;
        }
        if (depth == 0 || (n | tabu) == mask) return;
        int nn = mask & ~n;
        while (nn != 0) {
            int firstOne = nn & ~(nn - 1);
            nn = nn & ~firstOne;
            if ((firstOne & tabu) == 0) {
                S.insert(n | firstOne);
                superset(n | firstOne, S, tabu, mask, depth - 1);
            }
        }
    }

    std::vector<ColIndex> getColIndexVector(NodeIndex nodeID) {
        std::vector<ColIndex> V;
        for (ColIndex i = 0; i < BITMAP.size(); ++i) {
            if (nodeID & BITMAP.at(i)) {
                V.push_back(i);
            }
        }
        return V;
    }

    void minimize(std::set<NodeIndex>& newSeeds, std::set<NodeIndex>& seeds) {
        seeds.clear();
        while (!newSeeds.empty()) {
            auto x = *newSeeds.begin();
            newSeeds.erase(newSeeds.begin());
            bool reserve_it = true;
            for (auto iter = newSeeds.begin(); iter != newSeeds.end();) {
                if ((*iter & x) == x) {
                    newSeeds.erase(iter++);
                } else if ((*iter & x) == *iter) {
                    reserve_it = false;
                    break;
                } else {
                    ++iter;
                }
            }
            if (reserve_it) {
                seeds.insert(x);
            }
        }
        newSeeds.clear();
    }

    void findLHSs() {
        if (non_unique_cols.size() < 2) return;
        std::set<NodeIndex> seeds;
        //NodeIndex firstseed = non_unique_cols.at(0) == current_rhs ? non_unique_cols.at(1) : non_unique_cols.at(0); // should random?
        //seeds.insert(BITMAP.at(firstseed));
        //NodeIndex firstseed = ((1 << ncol) - 1) & ~(tabu_for_unique_cols | BITMAP.at(current_rhs));
        //seeds.insert(firstseed);
        for (auto col : non_unique_cols) {
            if (col == current_rhs) continue;
            seeds.insert(BITMAP.at(col));
            while (!seeds.empty()) {
#ifdef RANDOM
                int nodeID = getRandom(seeds);
#else
                int nodeID = *seeds.begin();
#endif
                while (nodeID != -1) {
                    auto& node = NodeSet.at(nodeID);
                    if (node.isVisited) {
                        if (node.isCandidateMinDep || node.isCandidateMaxNonDep) {
                            if (node.isDep) {
                                if (node.isMinDep) {
                                    node.isCandidateMinDep = false;
                                    minDeps.push_back(nodeID);
                                }
                            } else {
                                if (node.isMaxNonDep) {
                                    node.isCandidateMaxNonDep = false;
                                    maxNonDeps.push_back(nodeID);
                                }
                            }
                            //updateDependencyType(node);
                        }
                    } else {
                        if (!inferCategory(nodeID)) {
                            computePartition(nodeID);
                        }
                    }
                    nodeID = pickNextNode(nodeID);
                }
                seeds = generateNextSeeds();
                //auto tmp = generateNextSeeds();
                //seeds.insert(tmp.begin(), tmp.end());
            }
        }
    }

    NodeIndex pickNextNode(NodeIndex nodeID) {
        auto& node = NodeSet.at(nodeID);
        if (node.isCandidateMinDep) {
            std::set<NodeIndex> S;
            subset(nodeID, S, 1); // Not sure whether I understand it correctly
            for (auto iter = S.begin(); iter != S.end();) {
                if (NodeSet.at(*iter).isVisited) {
                    if (NodeSet.at(*iter).isNonDep) {
                        S.erase(iter++);
                    } else {
                        node.isCandidateMinDep = false;
                        S.clear();
                        break;
                    }
                } else {
                    ++iter;
                }
            }
            if (S.empty()) {
                if (node.isCandidateMinDep) {
                    node.isMinDep = true;
                    node.isCandidateMinDep = false;
                    minDeps.push_back(nodeID);
                }
            } else {
#ifdef RANDOM
                NodeIndex nextNode = getRandom(S);
#else
                NodeIndex nextNode = *S.begin(); // should random
#endif
                trace.push(nodeID);
                return nextNode;
            }
        } else if (node.isCandidateMaxNonDep) {
            std::set<NodeIndex> S;
            superset(nodeID, S, tabu_for_unique_cols | BITMAP.at(current_rhs), (1 << ncol) - 1, 1); // Not sure whether I understand it correctly
            for (auto iter = S.begin(); iter != S.end();) {
                if (NodeSet.at(*iter).isVisited) {
                    if (NodeSet.at(*iter).isDep) {
                        S.erase(iter++);
                    } else {
                        node.isCandidateMaxNonDep = false;
                        break;
                        S.clear();
                    }
                } else {
                    ++iter;
                }
            }
            if (S.empty()) {
                if (node.isCandidateMaxNonDep) {
                    node.isMaxNonDep = true;
                    node.isCandidateMaxNonDep = false;
                    maxNonDeps.push_back(nodeID);
                }
            } else {
#ifdef RANDOM
                NodeIndex nextNode = getRandom(S);
#else
                NodeIndex nextNode = *S.begin();
#endif
                trace.push(nodeID);
                return nextNode;
            }
        }
        if (trace.empty())
            return -1;
        NodeIndex idx = trace.top();
        trace.pop();
        return idx;
    }

    std::set<NodeIndex> generateNextSeeds() {
        std::set<NodeIndex> seeds;
        std::set<NodeIndex> newSeeds;
        for (auto maxNonDep : maxNonDeps) {
            NodeIndex complement = ((1 << ncol) - 1) & (~maxNonDep) & ~(tabu_for_unique_cols | BITMAP.at(current_rhs));
            if (seeds.empty()) {
                for (ColIndex i = 0; i < ncol; ++i) {
                    if (complement & BITMAP.at(i)) {
                        seeds.insert(BITMAP.at(i));
                    }
                }
            } else {
                for (auto dep : seeds) {
                    for (ColIndex i = 0; i < ncol; ++i) {
                        if (complement & BITMAP.at(i)) {
                            newSeeds.insert(dep | BITMAP.at(i));
                        }
                    }
                }
                minimize(newSeeds, seeds);
            }
        }
        for (auto dep : minDeps)
            seeds.erase(dep);
        return seeds;
    }

    bool checkFD(NodeIndex nodeID) {
        std::map<std::string, std::string> dict;
        auto lhs = getColIndexVector(nodeID);
        bool isFD = true;
        for (int i = 0; i < size; ++i) {
            std::string l;
            for (auto j : lhs) {
                l += data.at(j).at(i);
            }
            auto iter = dict.find(l);
            if (iter == dict.end()) {
                dict.insert(std::pair<std::string, std::string>(l, data.at(current_rhs).at(i)));
            } else {
                if (data.at(current_rhs).at(i) != iter->second) {
                    isFD = false;
                    break;
                }
            }
        }
#ifdef PROPOGATE
        if (isFD) { // could do more?
            std::set<NodeIndex> S;
            superset(nodeID, S, BITMAP.at(current_rhs) | tabu_for_unique_cols, (1 << ncol) - 1, 1);
            for (int s : S) {
                auto& node = NodeSet.at(s);
                node.isVisited = true;
                node.isDep = true;
                node.isCandidateMinDep = false;
                node.isCandidateMaxNonDep = false;
            }
        } else {
            std::set<NodeIndex> S;
            subset(nodeID, S, 1);
            for (int s : S) {
                auto& node = NodeSet.at(s);
                node.isVisited = true;
                node.isNonDep = true;
                node.isCandidateMinDep = false;
                node.isCandidateMaxNonDep = false;
            }
        }
#endif
        return isFD;
    }

    bool inferCategory(NodeIndex nodeID) {
        std::set<NodeIndex> S;
        auto& node = NodeSet.at(nodeID);
        subset(nodeID, S, 1);
        for (auto s : S) {
            if (NodeSet.at(s).isVisited && NodeSet.at(s).isDep) {
                node.isVisited = true;
                node.isDep = true;
                node.isCandidateMinDep = false;
                node.isCandidateMaxNonDep = false;
                return true;
            }
        }
        S.clear();
        superset(nodeID, S, tabu_for_unique_cols | BITMAP.at(current_rhs), (1 << ncol) - 1, 1);
        for (auto s : S) {
            if (NodeSet.at(s).isVisited && NodeSet.at(s).isNonDep) {
                node.isVisited = true;
                node.isNonDep = true;
                node.isCandidateMaxNonDep = false;
                node.isCandidateMinDep = false;
                return true;
            }
        }
        return false;
    }

    void computePartition(NodeIndex nodeID) {
        auto& node = NodeSet.at(nodeID);
        node.isVisited = true;
        bool check_result = checkFD(nodeID);
        node.isDep = check_result;
        node.isCandidateMinDep = check_result;
        node.isNonDep = !check_result;
        node.isCandidateMaxNonDep = !check_result;
    }
};
