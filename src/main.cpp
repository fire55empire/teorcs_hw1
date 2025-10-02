#include "fsmachine.hpp"

static std::vector<int> parseInputString(const std::string &s) {
    std::istringstream iss(s);
    std::vector<int> res;
    int x;
    while (iss >> x) res.push_back(x);
    return res;
}

int main (int argc, char** argv) {

    const std::string filepath = argv[1];
    const std::string inputStr = argv[2];

    std::vector <int> data = parseInputString(inputStr);

    FinStateMachine FSM = FinStateMachine(filepath);
    std::cout << FSM.isDFA() << std::endl;
    return 0;
}
