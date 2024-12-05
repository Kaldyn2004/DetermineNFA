#pragma once
#include "../stdafx.h"
using namespace std;

class Determiner
{
public:
    void ReadNFA(const string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string line;
        std::vector<std::string> headers;

        if (std::getline(file, line))
        {
            std::istringstream headerStream(line);
            std::string cell;
            while (std::getline(headerStream, cell, ';'))
            {
                headers.push_back(cell);
            }
        }

        if (std::getline(file, line))
        {
            std::istringstream stateStream(line);
            std::string cell;
            int i = 0;

            while (std::getline(stateStream, cell, ';'))
            {
                if (!cell.empty())
                {
                    nfa.states.push_back(cell);
                    if (headers[i] == "F")
                    {
                        nfa.finalStates.push_back(cell);
                    }
                }
                i++;
            }
        }

        while (std::getline(file, line)) {
            std::istringstream rowStream(line);
            std::string cell;
            string symbol;
            if (std::getline(rowStream, cell, ';')) {
                if (!cell.empty())
                {
                    nfa.alphabet.push_back(cell);
                    symbol = cell;
                }
            }

            int i = 0;
            while (std::getline(rowStream, cell, ';'))
            {
                if (!cell.empty())
                {
                    std::istringstream destStream(cell);
                    std::string destination;
                    while (std::getline(destStream, destination, ',')) {
                        nfa.transitions[nfa.states[i]][symbol].push_back(destination);
                    }
                }
                i++;
            }
        }

        if (!nfa.states.empty()) {
            nfa.startState = *nfa.states.begin();
        }
    }

    void ConvertToDFA() {
        СomputeAllEpsilonClosures();
        BuildDFATransitionTable();
        PrintDFATransitionTable();
    }

    void СomputeAllEpsilonClosures() {
        std::cout << nfa.startState << std::endl;
        for (const auto& state : nfa.states) {
            std::cout << state << std::endl;
            std::vector<std::string> closure = computeEpsilonClosure(state, nfa.transitions);
            nfa.epsilonClosures.emplace_back(state, closure);
        }
    }

    void WriteDFAToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        for (const auto& [state, transitions] : dfa.transitions) {
            dfa.final_states.contains(state)
            ? file << ";F"
            : file << ";";
        }
        file << endl;
        std::map<string, string> renamedState;
        int i;
        for (const auto& [state, transitions] : dfa.transitions) {
            renamedState[state] = "q" + to_string(i);
           file << ";" << renamedState[state];
            i++;
        }
        file << endl;

        for (const auto& symbol : dfa.alphabet) {
            file << symbol;

            for (const auto& [state, transitions] : dfa.transitions) {
                if (transitions.count(symbol)) {
                    file << ";" << renamedState[transitions.at(symbol)];
                } else
                {
                    file << ";";
                }
            }
            file << endl;
        }

        for (const auto& [state, transitions] : dfa.transitions) {
            std::cout << state;
            for (const auto& symbol : dfa.alphabet) {
                if (transitions.count(symbol)) {
                    std::cout << "\t" << transitions.at(symbol);
                } else {
                    std::cout << "\t-"; // Для отсутствующих переходов
                }
            }
            std::cout << "\n";
        }

        // Вывод финальных состояний
        std::cout << "\nFinal States:\n";
        for (const auto& state : dfa.final_states) {
            std::cout << state << "\n";
        }

        // Стартовое состояние
        std::cout << "\nStart State:\n" << dfa.start_state << "\n";


        std::cout << "DFA has been written to file: " << filename << "\n";
    }


    static std::vector<std::string> computeEpsilonClosure(
            const std::string& state,
            const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>& transitions
    ) {
        std::unordered_set<std::string> closure;
        std::vector<std::string> stack = {state};

        while (!stack.empty()) {
            std::string current = stack.back();
            stack.pop_back();

            if (closure.find(current) == closure.end()) {
                closure.insert(current);

                auto it = transitions.find(current);
                if (it != transitions.end()) {
                    auto epsilonTransitions = it->second.find("ε");
                    if (epsilonTransitions != it->second.end()) {
                        for (const auto& next : epsilonTransitions->second) {
                            stack.push_back(next);
                        }
                    }
                }
            }
        }

        return std::vector<std::string>(closure.begin(), closure.end());
    }

private:
    struct NFA
    {
        std::vector<std::string> states;
        std::vector<std::string> alphabet;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> transitions;
        std::string startState;
        std::vector<std::string> finalStates;
        std::vector<std::pair<std::string, std::vector<std::string>>> epsilonClosures;
    };

    struct DFA
    {
        set<string> states;
        std::vector<std::string> alphabet;
        map<string, map<string, string>> transitions;
        set<string> final_states;
        string start_state;
    };

    NFA nfa;
    DFA dfa;

    set<string> EpsilonClosure(const string &state)
    {
        set<string> closure = {state};
        queue<string> to_process;
        to_process.push(state);

        while (!to_process.empty())
        {
            string current = to_process.front();
            to_process.pop();
            if (nfa.transitions.count(current) && nfa.transitions.at(current).count("ε"))
            {
                for (const auto &next_state : nfa.transitions.at(current).at("ε"))
                {
                    if (!closure.count(next_state)) {
                        closure.insert(next_state);
                        to_process.push(next_state);
                    }
                }
            }
        }
        return closure;
    }

    void BuildDFATransitionTable() {
        // Очередь для обработки состояний
        std::queue<std::set<std::string>> stateQueue;

        // Начальное состояние ДКА — ε-замыкание начального состояния НКА
        auto startClosure = EpsilonClosure(nfa.startState);
        stateQueue.push(startClosure);

        // Множество всех уникальных состояний (для предотвращения дублирования)
        std::set<std::set<std::string>> processedStates;
        processedStates.insert(startClosure);

        // Устанавливаем начальное состояние ДКА
        std::string startStateName = StateSetToString(startClosure);
        dfa.start_state = startStateName;

        // Алфавит ДКА (без "ε")
        std::vector<std::string> dfaAlphabet(nfa.alphabet.begin(), nfa.alphabet.end());
        auto it = std::find(dfaAlphabet.begin(),dfaAlphabet.end(), "ε");
        if(it != dfaAlphabet.end()) //если найден
            dfaAlphabet.erase(it);
        dfa.alphabet = dfaAlphabet;

        // Проверяем финальность начального состояния
        for (const auto& state : startClosure) {
            if (std::find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
                dfa.final_states.insert(startStateName);
                break;
            }
        }

        // Обработка всех состояний
        while (!stateQueue.empty()) {
            auto currentStates = stateQueue.front();
            stateQueue.pop();

            std::string currentStateName = StateSetToString(currentStates);

            for (const auto& symbol : dfaAlphabet) {
                std::set<std::string> reachable;

                // Ищем состояния, достижимые по текущему символу
                for (const auto& state : currentStates) {
                    if (nfa.transitions.count(state) && nfa.transitions.at(state).count(symbol)) {
                        for (const auto& nextState : nfa.transitions.at(state).at(symbol)) {
                            auto closure = EpsilonClosure(nextState);
                            reachable.insert(closure.begin(), closure.end());
                        }
                    }
                }

                if (!reachable.empty()) {
                    std::string reachableStateName = StateSetToString(reachable);

                    // Добавляем переход
                    dfa.transitions[currentStateName][symbol] = reachableStateName;

                    // Если это новое множество состояний, добавляем его в очередь
                    if (processedStates.find(reachable) == processedStates.end()) {
                        stateQueue.push(reachable);
                        processedStates.insert(reachable);

                        // Проверяем, является ли новое состояние финальным
                        for (const auto& state : reachable) {
                            if (std::find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
                                dfa.final_states.insert(reachableStateName);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

// Вспомогательный метод для преобразования множества состояний в строку
    std::string StateSetToString(const std::set<std::string>& states) {
        std::vector<std::string> sortedStates(states.begin(), states.end());
        return std::accumulate(std::next(sortedStates.begin()), sortedStates.end(),
                                     sortedStates[0], [](std::string a, std::string b) { return a + "," + b; });
    }

    void PrintDFATransitionTable() const {
        std::cout << "DFA Transition Table:\n";

        // Выводим заголовок
        std::cout << "State";
        for (const auto& symbol : dfa.alphabet) {
            std::cout << "\t" << symbol;
        }
        std::cout << "\n";

        // Выводим каждую строку таблицы переходов
        for (const auto& [state, transitions] : dfa.transitions) {
            std::cout << state;
            for (const auto& symbol : dfa.alphabet) {
                if (transitions.count(symbol)) {
                    std::cout << "\t" << transitions.at(symbol);
                } else {
                    std::cout << "\t-"; // Для отсутствующих переходов
                }
            }
            std::cout << "\n";
        }

        // Вывод финальных состояний
        std::cout << "\nFinal States:\n";
        for (const auto& state : dfa.final_states) {
            std::cout << state << "\n";
        }

        // Стартовое состояние
        std::cout << "\nStart State:\n" << dfa.start_state << "\n";
    }
};

