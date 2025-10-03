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

FinStateMachine FinStateMachine::toDFA() const {
   	std::map<std::vector<int>, int> idx;
    std::vector<std::vector<int>> sets; // reverse map: dfa id -> subset of NFA states

    auto normalize = [](std::vector<int> v) {
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
        return v;
    };

    std::vector<int> startSet = startList;
    startSet = normalize(startSet);

    idx[startSet] = 0;
    sets.push_back(startSet);

    std::vector<std::vector<std::vector<int>>> dfa_trans;
    dfa_trans.emplace_back(m); // for state 0

    std::queue<int> q;
    q.push(0);

    while (!q.empty()) {
        int did = q.front(); q.pop();
        const std::vector<int> &nfa_set = sets[did];

        if ((int)dfa_trans.size() <= did) dfa_trans.resize(did + 1, std::vector<std::vector<int>>(m));

        for (int sym = 0; sym < m; ++sym) {
            std::vector<int> nextSet;
            for (int s : nfa_set) {
                if (s < 0 || s >= n) continue; // defensive
                const auto &dsts = transitions[s][sym];
                for (int d : dsts) nextSet.push_back(d);
            }
            nextSet = normalize(nextSet);

            auto it = idx.find(nextSet);
            int ndid;
            if (it == idx.end()) {
                ndid = (int)sets.size();
                idx[nextSet] = ndid;
                sets.push_back(nextSet);
                dfa_trans.emplace_back(m);
                q.push(ndid);
            } else {
                ndid = it->second;
            }

            if (!nextSet.empty()) dfa_trans[did][sym].push_back(ndid);
        }
    }

    FinStateMachine dfa((int)sets.size(), m);

    dfa.startList.clear();
    dfa.startList.push_back(0);
    dfa.start.assign(dfa.n, 0);
    if (dfa.n > 0) dfa.start[0] = 1;

    dfa.accept.assign(dfa.n, 0);
    dfa.acceptList.clear();
    for (int i = 0; i < dfa.n; ++i) {
        for (int s : sets[i]) {
            if (s >= 0 && s < n && accept[s]) {
                dfa.accept[i] = 1;
                dfa.acceptList.push_back(i);
                break;
            }
        }
    }

    for (int i = 0; i < dfa.n; ++i) {
        for (int sym = 0; sym < m; ++sym) {
            const auto &vec = dfa_trans[i][sym];
            for (int dst : vec) {
                dfa.transitions[i][sym].push_back(dst);
            }
        }
    }

    return dfa;
}

void FinStateMachine::writeToFile(const std::string &filepath) const {
    std::ofstream fout(filepath);
    if (!fout.is_open()) throw std::runtime_error("cannot open output file: " + filepath);

    fout << n << " " << m << "\n";

    for (size_t i = 0; i < startList.size(); ++i) {
        if (i) fout << ' ';
        fout << startList[i];
    }
    fout << "\n";

    for (size_t i = 0; i < acceptList.size(); ++i) {
        if (i) fout << ' ';
        fout << acceptList[i];
    }
    fout << "\n";

    for (int src = 0; src < n; ++src) {
        for (int sym = 0; sym < m; ++sym) {
            for (int dst : transitions[src][sym]) {
                fout << src << " " << sym << " " << dst << "\n";
            }
        }
    }

    fout.close();
}