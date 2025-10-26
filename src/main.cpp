#include "fsmachine.hpp"
#include <iostream>
#include <cctype>

static std::vector<int> parse_input(const std::string &s) {
    std::vector<int> res;
    std::istringstream iss(s);
    int x;
    iss.clear(); iss.str(s);
    while (iss >> x) res.push_back(x);
    if (!res.empty()) return res;
    for (char c : s) if (std::isdigit((unsigned char)c)) res.push_back(c - '0');
    return res;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  ./FSMachine run <file> <input>\n";
        std::cout << "  ./FSMachine refactor <in> <out>    # convert NFA->DFA\n";
        std::cout << "  ./FSMachine minimize <in> <out>    # minimize DFA (input may be NFA)\n";
        std::cout << "  ./FSMachine equal <file1> <file2>  # print true|false\n";
        std::cout << "  ./FSMachine universal <file>       # print true|false if language == Sigma*\n";
        return 0;
    }

    std::string cmd = argv[1];

    try {
        if (cmd == "run") {
            if (argc < 4) throw std::runtime_error("run requires <file> <input>");
            std::string file = argv[2];
            std::string input;
            for (int i = 3; i < argc; ++i) { if (i > 3) input += ' '; input += argv[i]; }
            FinStateMachine f(file);
            auto data = parse_input(input);
            std::cout << (f.accepts(data) ? "true" : "false") << std::endl;
            return 0;
        }
        if (cmd == "refactor") {
            if (argc != 4) throw std::runtime_error("refactor requires <in> <out>");
            FinStateMachine f(argv[2]);
            FinStateMachine dfa = f.toDFA();
            dfa.writeToFile(argv[3]);
            std::cout << "OK" << std::endl;
            return 0;
        }
        if (cmd == "minimize") {
            if (argc != 4) throw std::runtime_error("minimize requires <in> <out>");
            FinStateMachine f(argv[2]);
            FinStateMachine dfa = f.minimize();
            dfa.writeToFile(argv[3]);
            std::cout << "OK" << std::endl;
            return 0;
        }
        if (cmd == "equal") {
            if (argc != 4) throw std::runtime_error("equal requires <file1> <file2>");
            FinStateMachine a(argv[2]);
            FinStateMachine b(argv[3]);
            std::cout << (a.is_equivalent(b) ? "true" : "false") << std::endl;
            return 0;
        }

        if (cmd == "universal") {
            if (argc != 3) throw std::runtime_error("universal requires <file>");
            FinStateMachine a(argv[2]);
            std::cout << (a.is_universal() ? "true" : "false") << std::endl;
            return 0;
        }

        throw std::runtime_error("unknown command");
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
