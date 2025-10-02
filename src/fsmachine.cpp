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

    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // убираем остаток строки после m

    start.assign(n, 0);
    accept.assign(n, 0);
    startList.clear();
    acceptList.clear();

    transitions.assign(n + 1, std::vector <std::vector <int>>(m + 1));

    std::string line;
    int x;

    if (std::getline(fin, line)) {
        std::istringstream iss(line);
        while (iss >> x) {
            if (x >= 0 && x < n) {
                start[x] = 1;
            startList.push_back(x);
            }
        }
    }

    if (std::getline(fin, line)) {
        std::istringstream iss(line);
        while (iss >> x) {
            if (x >= 0 && x < n) {
                accept[x] = 1;
            acceptList.push_back(x);
            }
        }
    }

    int cond1, num, cond2;
    while (fin >> cond1 >> num >> cond2) {
        transitions[cond1][num].push_back(cond2);
    }

    std::cout << "start: ";
    for (int x : startList) std::cout << x << " ";
    std::cout << std::endl;

    std::cout << "accept: ";
    for (int x : acceptList) std::cout << x << " ";
    std::cout << std::endl;
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

bool FinStateMachine::accepts(const std::vector<int> &data) const {
    if (isDFA()) {
        int cur = startList[0];
        for (int num : data) {
            if (transitions[cur][num].size() != 1) return false;
            cur = transitions[cur][num][0];
        }
        return true;
    } else {
        std::vector<char> cur(n, 0), next(n, 0);

        if (!startList.empty()) {
            for (int s : startList) cur[s] = 1;
        } else {
            for (int i = 0; i < n; ++i) if (start[i]) cur[i] = 1;
        }

        if (data.empty()) {
            for (int i = 0; i < n; ++i) if (cur[i] && accept[i]) return true;
            return false;
        }

        for (int num : data) {
            if (num < 0 || num >= m) return false;

            std::fill(next.begin(), next.end(), 0);

            for (int s = 0; s < n; ++s) {
                if (!cur[s]) continue;
                const auto &targets = transitions[s][num];
                for (int dst : targets) {
                    next[dst] = 1;
                }
            }

            cur.swap(next);

            bool any = false;
            for (int i = 0; i < n; ++i) if (cur[i]) { any = true; break; }
            if (!any) return false;
        }

        for (int i = 0; i < n; ++i) if (cur[i] && accept[i]) return true;
        return false;
    }
}

FinStateMachine toDFA() const {
    int x;
}

void FinStateMachine::writeToFile(const std::string &filepath) const {
    return;
}