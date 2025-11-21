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
    fin >> n >> m;

    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    start.assign(n, 0);
    accept.assign(n, 0);
    startList.clear();
    acceptList.clear();

    transitions.assign(n, std::vector <std::vector <int>>(m));

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
            if (num < 0 || num >= m) return false;
            if (transitions[cur][num].size() != 1) return false;
            cur = transitions[cur][num][0];
        }
        return accept[cur] != 0;
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
    std::vector<std::vector<int>> sets;

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
    dfa_trans.emplace_back(m);

    std::queue<int> q;
    q.push(0);

    while (!q.empty()) {
        int did = q.front(); q.pop();
        const std::vector<int> &nfa_set = sets[did];

        if ((int)dfa_trans.size() <= did) dfa_trans.resize(did + 1, std::vector<std::vector<int>>(m));

        for (int sym = 0; sym < m; ++sym) {
            std::vector<int> nextSet;
            for (int s : nfa_set) {
                if (s < 0 || s >= n) continue;
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
            } else ndid = it->second;

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

static std::tuple<std::vector<std::vector<int>>, int, std::vector<char>> build_complete_deterministic(const FinStateMachine &fsm) {
    if (!fsm.isDFA()) throw std::runtime_error("build_complete_deterministic: input must be DFA");

    int n0 = fsm.n;
    int m = fsm.m;
    bool need_dead = false;
    for (int i = 0; i < fsm.n; ++i)
        for (int sym = 0; sym < fsm.m; ++sym)
            if (fsm.transitions[i][sym].empty()) need_dead = true;

    int dead = -1;
    if (need_dead) {
        dead = n0;
        ++n0;
    }

    std::vector<std::vector<int>> trans(n0, std::vector<int>(m));

    for (int i = 0; i < fsm.n; ++i) {
        for (int sym = 0; sym < fsm.m; ++sym) {
            if (fsm.transitions[i][sym].size() == 1) trans[i][sym] = fsm.transitions[i][sym][0];
            else trans[i][sym] = (need_dead ? dead : -1);
        }
    }

    if (need_dead) for (int sym = 0; sym < m; ++sym) trans[dead][sym] = dead;

    std::vector<char> accept(n0, 0);
    for (int i = 0; i < fsm.n; ++i) accept[i] = fsm.accept[i];
    if (need_dead) accept[dead] = 0;

    int startstate = (fsm.startList.empty() ? 0 : fsm.startList[0]);

    return {trans, startstate, accept};
}

FinStateMachine FinStateMachine::minimize() const {
    if (!isDFA()) {
        FinStateMachine dfa = toDFA();
        return dfa.minimize();
    }

    auto [trans, startstate, acceptvec] = build_complete_deterministic(*this);
    int n0 = (int)trans.size();
    int m0 = m;

    std::vector<int> accept_states;
    std::vector<int> nonaccept_states;
    for (int i = 0; i < n0; ++i) if (acceptvec[i]) accept_states.push_back(i); else nonaccept_states.push_back(i);

    std::vector<std::vector<int>> P;
    if (!accept_states.empty()) P.push_back(accept_states);
    if (!nonaccept_states.empty()) P.push_back(nonaccept_states);

    std::queue<std::vector<int>> Wq;
    if (!accept_states.empty()) Wq.push(accept_states);
    if (!nonaccept_states.empty()) Wq.push(nonaccept_states);

    while (!Wq.empty()) {
        std::vector<int> A = Wq.front(); Wq.pop();
        std::vector<char> inA(n0, 0);
        for (int s : A) inA[s] = 1;

        for (int c = 0; c < m0; ++c) {
            std::vector<int> X;
            X.reserve(n0);
            for (int s = 0; s < n0; ++s) {
                int t = trans[s][c];
                if (t >= 0 && inA[t]) X.push_back(s);
            }

            if (X.empty()) continue;

            std::vector<std::vector<int>> newP;
            for (auto &Y : P) {
                std::vector<int> inter;
                std::vector<int> diff;
                for (int s : Y) if (std::binary_search(X.begin(), X.end(), s)) inter.push_back(s); else diff.push_back(s);

                std::sort(X.begin(), X.end());

                if (inter.empty() || diff.empty()) {
                    newP.push_back(Y);
                } else {
                    newP.push_back(inter);
                    newP.push_back(diff);

                    if (inter.size() <= diff.size()) Wq.push(inter); else Wq.push(diff);
                }
            }

            P.swap(newP);
        }
    }

    std::vector<int> which(n0, -1);
    for (int i = 0; i < (int)P.size(); ++i) for (int s : P[i]) which[s] = i;

    int newn = (int)P.size();
    FinStateMachine res(newn, m0);
    res.startList.clear();
    int newStart = which[startstate];
    res.startList.push_back(newStart);
    res.start.assign(newn, 0);
    if (newn > 0) res.start[newStart] = 1;

    res.accept.assign(newn, 0);
    res.acceptList.clear();
    for (int i = 0; i < newn; ++i) {
        for (int s : P[i]) if (acceptvec[s]) { res.accept[i] = 1; res.acceptList.push_back(i); break; }
    }

    res.transitions.assign(newn, std::vector<std::vector<int>>(m0));
    for (int i = 0; i < n0; ++i) {
        int bi = which[i];
        for (int c = 0; c < m0; ++c) {
            int j = trans[i][c];
            int bj = which[j];
            if (std::find(res.transitions[bi][c].begin(), res.transitions[bi][c].end(), bj) == res.transitions[bi][c].end())
                res.transitions[bi][c].push_back(bj);
        }
    }

    return res;
}

bool FinStateMachine::is_equivalent(const FinStateMachine &other) const {
    FinStateMachine A = *this;
    FinStateMachine B = other;

    if (!A.isDFA()) A = A.toDFA();
    if (!B.isDFA()) B = B.toDFA();

    auto [transA, startA, acceptA] = build_complete_deterministic(A);
    auto [transB, startB, acceptB] = build_complete_deterministic(B);

    if ((int)transA[0].size() != (int)transB[0].size()) return false;
    int m0 = (int)transA[0].size();

    using Pair = std::pair<int,int>;
    std::queue<Pair> q;
    std::set<Pair> vis;
    q.push({startA, startB});
    vis.insert({startA, startB});

    while (!q.empty()) {
        auto [u, v] = q.front(); q.pop();
        if (acceptA[u] != acceptB[v]) return false;
        for (int c = 0; c < m0; ++c) {
            int nu = transA[u][c];
            int nv = transB[v][c];
            Pair p = {nu, nv};
            if (!vis.count(p)) { vis.insert(p); q.push(p); }
        }
    }

    return true;
}

bool FinStateMachine::is_universal() const {
    FinStateMachine A = *this;
    if (!A.isDFA()) A = A.toDFA();

    auto [transA, startA, acceptA] = build_complete_deterministic(A);
    int n0 = (int)transA.size();

    std::vector<char> vis(n0, 0);
    std::queue<int> q;
    q.push(startA);
    vis[startA] = 1;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (!acceptA[u]) return false;
        for (int c = 0; c < (int)transA[u].size(); ++c) {
            int v = transA[u][c];
            if (!vis[v]) { vis[v] = 1; q.push(v); }
        }
    }

    return true;
}

void FinStateMachine::writeToFile(const std::string &filepath) const {
    std::ofstream fout(filepath);
    if (!fout.is_open()) throw std::runtime_error("cannot open output file: " + filepath);

    fout << n << "\n" << m << "\n";

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

struct NFABuilder {
    int n;
    int m;
    int epsilon_sym;
    int start_state;
    int end_state;
    std::vector<std::vector<std::vector<int>>> transitions;
    std::vector<char> is_accept;
    
    NFABuilder(int num_states, int alphabet_size) 
        : n(num_states), m(alphabet_size), epsilon_sym(alphabet_size),
          start_state(0), end_state(num_states - 1),
          transitions(n, std::vector<std::vector<int>>(m + 1)),
          is_accept(n, 0) {
    }
    
    void add_transition(int from, int symbol, int to) {
        if (from >= 0 && from < n && to >= 0 && to < n && symbol >= 0 && symbol <= m) {
            transitions[from][symbol].push_back(to);
        }
    }
    
    std::vector<int> epsilon_closure(int state) const {
        std::vector<int> result;
        std::vector<char> visited(n, 0);
        std::queue<int> q;
        
        q.push(state);
        visited[state] = 1;
        
        while (!q.empty()) {
            int s = q.front();
            q.pop();
            result.push_back(s);
            
            for (int next : transitions[s][epsilon_sym]) {
                if (!visited[next]) {
                    visited[next] = 1;
                    q.push(next);
                }
            }
        }
        
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
    
    std::vector<int> epsilon_closure(const std::vector<int>& states) const {
        std::vector<int> result;
        std::vector<char> visited(n, 0);
        std::queue<int> q;
        
        for (int s : states) {
            if (!visited[s]) {
                visited[s] = 1;
                q.push(s);
            }
        }
        
        while (!q.empty()) {
            int s = q.front();
            q.pop();
            result.push_back(s);
            
            for (int next : transitions[s][epsilon_sym]) {
                if (!visited[next]) {
                    visited[next] = 1;
                    q.push(next);
                }
            }
        }
        
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
};

static FinStateMachine remove_epsilon_transitions(const NFABuilder& nfa_builder) {
    std::vector<std::vector<int>> closures(nfa_builder.n);
    for (int i = 0; i < nfa_builder.n; ++i) {
        closures[i] = nfa_builder.epsilon_closure(i);
    }
    
    FinStateMachine result(nfa_builder.n, nfa_builder.m);
    
    std::vector<int> start_closure = nfa_builder.epsilon_closure(nfa_builder.start_state);
    result.startList = start_closure;
    for (int s : start_closure) {
        if (s >= 0 && s < result.n) {
            result.start[s] = 1;
        }
    }
    
    for (int i = 0; i < nfa_builder.n; ++i) {
        if (nfa_builder.is_accept[i]) {
            result.accept[i] = 1;
            result.acceptList.push_back(i);
        }
    }
    
    for (int from = 0; from < nfa_builder.n; ++from) {
        for (int symbol = 0; symbol < nfa_builder.m; ++symbol) {
            std::vector<int> targets;
            std::vector<char> added(nfa_builder.n, 0);
            
            for (int s : closures[from]) {
                for (int to : nfa_builder.transitions[s][symbol]) {
                    for (int t : closures[to]) {
                        if (!added[t]) {
                            added[t] = 1;
                            targets.push_back(t);
                        }
                    }
                }
            }
            
            std::sort(targets.begin(), targets.end());
            targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
            
            for (int t : targets) {
                result.transitions[from][symbol].push_back(t);
            }
        }
    }
    
    return result;
}

class RegexParser {
private:
    std::string regex;
    size_t pos;
    int state_counter;
    int alphabet_size;
    
    void skip_whitespace() {
        while (pos < regex.length() && std::isspace((unsigned char)regex[pos])) {
            ++pos;
        }
    }
    
    bool at_end() {
        skip_whitespace();
        return pos >= regex.length();
    }
    
    char peek() {
        skip_whitespace();
        if (pos >= regex.length()) return '\0';
        return regex[pos];
    }
    
    char consume() {
        skip_whitespace();
        if (pos >= regex.length()) return '\0';
        char c = regex[pos];
        ++pos;
        return c;
    }
    
    bool match(char c) {
        skip_whitespace();
        if (pos < regex.length() && regex[pos] == c) {
            ++pos;
            return true;
        }
        return false;
    }
    
    bool is_operator(char c) {
        return c == '|' || c == '*' || c == '+' || c == '?' || c == '(' || c == ')';
    }
    
    NFABuilder build_symbol(char c) {
        if (c < '0' || c > '9') {
            throw std::runtime_error("Invalid character in regex: expected digit 0-9");
        }
        int symbol = c - '0';
        
        NFABuilder nfa(2, alphabet_size);
        nfa.add_transition(0, symbol, 1);
        nfa.is_accept[1] = 1;
        return nfa;
    }
    
    NFABuilder build_epsilon() {
        NFABuilder nfa(2, alphabet_size);
        nfa.add_transition(0, nfa.epsilon_sym, 1);
        nfa.is_accept[1] = 1;
        return nfa;
    }
    
    NFABuilder build_alternative(NFABuilder left, NFABuilder right) {
        int n_left = left.n;
        int n_right = right.n;
        int new_n = 2 + n_left + n_right;
        
        NFABuilder result(new_n, alphabet_size);
        result.start_state = 0;
        result.end_state = new_n - 1;
        result.is_accept[result.end_state] = 1;
        
        for (int i = 0; i < n_left; ++i) {
            for (int sym = 0; sym <= left.m; ++sym) {
                for (int to : left.transitions[i][sym]) {
                    int new_from = 1 + i;
                    int new_to = 1 + to;
                    result.add_transition(new_from, sym, new_to);
                }
            }
            if (left.is_accept[i]) {
                result.add_transition(1 + i, result.epsilon_sym, result.end_state);
            }
        }
        
        int right_offset = 1 + n_left;
        for (int i = 0; i < n_right; ++i) {
            for (int sym = 0; sym <= right.m; ++sym) {
                for (int to : right.transitions[i][sym]) {
                    int new_from = right_offset + i;
                    int new_to = right_offset + to;
                    result.add_transition(new_from, sym, new_to);
                }
            }
            if (right.is_accept[i]) {
                result.add_transition(right_offset + i, result.epsilon_sym, result.end_state);
            }
        }
        
        int left_start = 1 + left.start_state;
        int right_start = right_offset + right.start_state;
        result.add_transition(0, result.epsilon_sym, left_start);
        result.add_transition(0, result.epsilon_sym, right_start);
        
        return result;
    }
    
    NFABuilder build_concatenation(NFABuilder left, NFABuilder right) {
        int n_left = left.n;
        int n_right = right.n;
        int new_n = n_left + n_right;
        
        NFABuilder result(new_n, alphabet_size);
        result.start_state = left.start_state;
        
        for (int i = 0; i < n_left; ++i) {
            for (int sym = 0; sym <= left.m; ++sym) {
                for (int to : left.transitions[i][sym]) {
                    result.add_transition(i, sym, to);
                }
            }
        }
        
        int right_offset = n_left;
        for (int i = 0; i < n_right; ++i) {
            for (int sym = 0; sym <= right.m; ++sym) {
                for (int to : right.transitions[i][sym]) {
                    int new_from = right_offset + i;
                    int new_to = right_offset + to;
                    result.add_transition(new_from, sym, new_to);
                }
            }
            if (right.is_accept[i]) {
                result.is_accept[right_offset + i] = 1;
            }
        }
        
        for (int i = 0; i < n_left; ++i) {
            if (left.is_accept[i]) {
                int right_start = right_offset + right.start_state;
                result.add_transition(i, result.epsilon_sym, right_start);
                result.is_accept[i] = 0;
            }
        }
        
        result.end_state = right_offset + right.end_state;
        return result;
    }
    
    NFABuilder build_kleene(NFABuilder inner) {
        int n_inner = inner.n;
        int new_n = 2 + n_inner;
        
        NFABuilder result(new_n, alphabet_size);
        result.start_state = 0;
        result.end_state = new_n - 1;
        result.is_accept[result.end_state] = 1;
        
        int inner_offset = 1;
        for (int i = 0; i < n_inner; ++i) {
            for (int sym = 0; sym <= inner.m; ++sym) {
                for (int to : inner.transitions[i][sym]) {
                    int new_from = inner_offset + i;
                    int new_to = inner_offset + to;
                    result.add_transition(new_from, sym, new_to);
                }
            }
            if (inner.is_accept[i]) {
                result.add_transition(inner_offset + i, result.epsilon_sym, inner_offset + inner.start_state);
                result.add_transition(inner_offset + i, result.epsilon_sym, result.end_state);
            }
        }
        
        int inner_start = inner_offset + inner.start_state;
        result.add_transition(0, result.epsilon_sym, inner_start);
        result.add_transition(0, result.epsilon_sym, result.end_state);
        
        return result;
    }
    
    NFABuilder build_plus(NFABuilder inner) {
        NFABuilder kleene = build_kleene(inner);
        return build_concatenation(inner, kleene);
    }
    
    NFABuilder build_question(NFABuilder inner) {
        NFABuilder epsilon = build_epsilon();
        return build_alternative(epsilon, inner);
    }
    
    NFABuilder parse_atom() {
        skip_whitespace();
        
        if (at_end()) {
            throw std::runtime_error("Unexpected end of regex in atom");
        }
        
        if (match('(')) {
            NFABuilder result = parse_expression();
            if (!match(')')) {
                throw std::runtime_error("Expected closing parenthesis");
            }
            return result;
        }
        
        char c = consume();
        if (c >= '0' && c <= '9') {
            return build_symbol(c);
        }
        
        throw std::runtime_error("Invalid character in regex atom: " + std::string(1, c));
    }
    
    NFABuilder parse_unary() {
        NFABuilder atom = parse_atom();
        
        while (!at_end()) {
            skip_whitespace();
            if (match('*')) {
                atom = build_kleene(atom);
            } else if (match('+')) {
                atom = build_plus(atom);
            } else if (match('?')) {
                atom = build_question(atom);
            } else {
                break;
            }
        }
        
        return atom;
    }
    
    NFABuilder parse_concat() {
        NFABuilder result = parse_unary();
        
        while (!at_end()) {
            char next = peek();
            if (next == '|' || next == ')' || next == '\0') {
                break;
            }
            if (next == '*' || next == '+' || next == '?') {
                break;
            }
            
            NFABuilder right = parse_unary();
            result = build_concatenation(result, right);
        }
        
        return result;
    }
    
    NFABuilder parse_expression() {
        NFABuilder result = parse_concat();
        
        while (match('|')) {
            NFABuilder right = parse_concat();
            result = build_alternative(result, right);
        }
        
        return result;
    }
    
public:
    RegexParser(const std::string& regex_str, int alphabet_sz = 10)
        : regex(regex_str), pos(0), state_counter(0), alphabet_size(alphabet_sz) {
    }
    
    NFABuilder parse() {
        int balance = 0;
        for (char c : regex) {
            if (c == '(') ++balance;
            else if (c == ')') --balance;
            if (balance < 0) {
                throw std::runtime_error("Unmatched closing parenthesis");
            }
        }
        if (balance != 0) {
            throw std::runtime_error("Unmatched opening parenthesis");
        }
        
        pos = 0;
        NFABuilder result = parse_expression();
        
        skip_whitespace();
        if (pos < regex.length()) {
            throw std::runtime_error("Unexpected characters at end of regex");
        }
        
        return result;
    }
};

FinStateMachine FinStateMachine::from_regex(const std::string &regex) {
    if (regex.empty()) {
        FinStateMachine result(1, 10);
        result.startList.push_back(0);
        result.start[0] = 1;
        result.acceptList.push_back(0);
        result.accept[0] = 1;
        return result;
    }
    
    RegexParser parser(regex, 10);
    NFABuilder nfa_builder = parser.parse();
    
    FinStateMachine result = remove_epsilon_transitions(nfa_builder);
    
    return result;
}