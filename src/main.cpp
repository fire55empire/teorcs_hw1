#include "fsmachine.hpp"

static std::vector<int> parseInputString(const std::string &s) {
    std::istringstream iss(s);
    std::vector<int> res;
    int x;
    while (iss >> x) res.push_back(x);
    return res;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Invalid number of arguments" << std::endl;
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "run") {
        const std::string filepath = argv[2];
        const std::string inputRaw = argv[3];

        std::vector<int> input = parseInputString(inputRaw);
        FinStateMachine fsm(filepath);
        bool accepted = fsm.accepts(input);
        std::cout << (accepted ? "true" : "false") << std::endl;
        return 0;
    } else {
        const std::string inFile = argv[2];
        const std::string outFile = argv[3];

        FinStateMachine fsm(inFile);
        if (fsm.isDFA()) {
            fsm.writeToFile(outFile);
        } else {
            FinStateMachine dfa = fsm.toDFA();
            dfa.writeToFile(outFile);
        }
        return 0;
    }
}
