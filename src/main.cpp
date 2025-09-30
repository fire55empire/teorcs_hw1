#include "fsmachine.hpp"

int main (int argc, char** argv) {
    FinStateMachine FSM = FinStateMachine("data/test1.txt");
    std::cout << FSM.isDFA() << std::endl;
    return 0;
}
