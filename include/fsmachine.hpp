#ifndef fsmachine_hpp
#define fsmachine_hpp

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cctype>
#include <iostream>


struct FinStateMachine{
    int n;
    int m;

    std::vector <char> start;
    std::vector <char> accept;

    std::vector<int> startList;
    std::vector<int> acceptList;

    std::vector <std::vector <std::vector <int>>> transitions;

    FinStateMachine(int n_, int m_);

    explicit FinStateMachine(const std::string &filepath);

    std::vector <int> parseInputString(const std::string &s);

    bool isDFA() const noexcept;

    bool accepts(const std::vector<int> &input) const;

    FinStateMachine toDFA() const;

    void writeToFile(const std::string &filepath) const;
};

#endif