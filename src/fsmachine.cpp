#include "fsmachine.hpp"

FinStateMachine::FinStateMachine(int n_, int m_) {
    n = n_;
    m = m_;

    start.assign(n, 0);
    accept.assign(n, 0);
    startList.clear();
    acceptList.clear();

    transitions.assign(n, std::vector <std::vector <int>>(m));
}

FinStateMachine::FinStateMachine(const std::string &filepath) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) throw std::runtime_error("cannot open file: " + filepath);
    //файл считается полностью корректным
    fin >> n >> m;

    start.assign(n, 0);
    accept.assign(n, 0);
    startList.clear();
    acceptList.clear();

    transitions.assign(n, std::vector <std::vector <int>>(m));

    int s;
    while (fin.peek() != '\n' && fin >> s) {
        start[s] = 1;
        startList.push_back(s);
    }

    int a;
    while (fin.peek() != '\n' && fin >> a) {
        accept[a] = 1;
        acceptList.push_back(a);
    }

    int cond1, num, cond2;
    while (fin >> cond1 >> num >> cond2) {
        transitions[cond1][num].push_back(cond2);
    }
}

std::vector <int> FinStateMachine::parseInputString(const std::string &s) {
    std::vector <int> data;
    for (int i = 0; i < s.size(); ++i) data.push_back(s[i] - '0');
    return data;
}

bool FinStateMachine::isDFA() const noexcept {
    if (startList.size() != 1) return false;
    for (int cond1 = 0; cond1 < n; ++cond1)
        for (int cond2 = 0; cond2 < m; ++cond2)
            if (transitions[cond1][cond2].size() > 1) return false;
    return true;
}

bool FinStateMachine::accepts(const std::vector<int> &input) const {
    std::vector <int> data = parseInputString(input);

    if (isDFA()) {
        int cur = startList[0];
        for (int num : data) {
            if (transitions[cur][num] != 1) return false;
            cur = transitions[cur][num];
        }
        return true;
    } else {

    }
}

void FinStateMachine::writeToFile(const std::string &filepath) const {
    return;
}