#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

class Determiner {
public:
    void ReadNFA(const string &filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        string line;
        vector<string> headers;

        // Читаем заголовок
        if (getline(file, line)) {
            istringstream headerStream(line);
            string cell;
            while (getline(headerStream, cell, ';')) {
                headers.push_back(cell);
            }
        }

        // Читаем состояния
        if (getline(file, line)) {
            istringstream stateStream(line);
            string cell;
            int i = 0;

            while (getline(stateStream, cell, ';')) {
                if (!cell.empty()) {
                    nfa.states.push_back(cell);
                    if (headers[i] == "F") {
                        nfa.finalStates.push_back(cell);
                    }
                }
                i++;
            }
        }

        // Читаем переходы
        while (getline(file, line)) {
            istringstream rowStream(line);
            string cell;
            string symbol;
            if (getline(rowStream, cell, ';')) {
                if (!cell.empty()) {
                    nfa.alphabet.push_back(cell);
                    symbol = cell;
                }
            }

            int i = 0;
            while (getline(rowStream, cell, ';')) {
                if (!cell.empty()) {
                    istringstream destStream(cell);
                    string destination;
                    while (getline(destStream, destination, ',')) {
                        nfa.transitions[nfa.states[i]][symbol].push_back(destination);
                    }
                }
                i++;
            }
        }

        if (!nfa.states.empty()) {
            nfa.startState = nfa.states[0];
        }
    }

    void ConvertToDFA() {
        BuildDFATransitionTable();
        PrintDFATransitionTable();
    }

    void WriteDFAToFile(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        // Переименовываем состояния для удобства
        map<string, string> renamedState;
        int i = 0;
        for (const auto& [state, _] : dfa.transitions) {
            renamedState[state] = "q" + to_string(i++);
        }

        file << ";";
        if (dfa.final_states.contains(dfa.start_state)) {
            file << "F";
        }
        for (const auto& [state, _] : dfa.transitions) {
            if (state != dfa.start_state) {
                file << (dfa.final_states.contains(state) ? ";F" : ";");
            }
        }
        file << endl;

        file << ";";
        file << renamedState[dfa.start_state];
        for (const auto& [state, _] : dfa.transitions) {
            if (state != dfa.start_state) {
                file << ";" << renamedState[state];
            }
        }
        file << endl;

        for (const auto& symbol : dfa.alphabet) {
            file << symbol;
            for (const auto& [state, transitions] : dfa.transitions) {
                if (transitions.count(symbol)) {
                    file << ";" << renamedState[transitions.at(symbol)];
                } else {
                    file << ";";
                }
            }
            file << endl;
        }
    }


private:
    struct NFA {
        vector<string> states;
        vector<string> alphabet;
        unordered_map<string, unordered_map<string, vector<string>>> transitions;
        string startState;
        vector<string> finalStates;
    };

    struct DFA {
        set<string> states;
        vector<string> alphabet;
        map<string, map<string, string>> transitions;
        set<string> final_states;
        string start_state;
    };

    NFA nfa;
    DFA dfa;

    set<string> EpsilonClosure(const string &state) {
        set<string> closure = {state};
        queue<string> to_process;
        to_process.push(state);

        while (!to_process.empty()) {
            string current = to_process.front();
            to_process.pop();
            if (nfa.transitions.count(current) && nfa.transitions.at(current).count("ε")) {
                for (const auto &next_state : nfa.transitions.at(current).at("ε")) {
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
        queue<set<string>> stateQueue;

        // Начальное состояние ДКА — ε-замыкание начального состояния НКА
        auto startClosure = EpsilonClosure(nfa.startState);
        stateQueue.push(startClosure);

        // Множество всех уникальных состояний (для предотвращения дублирования)
        map<set<string>, string> processedStates;
        processedStates[startClosure] = "X0"; // Назначаем имя начальному состоянию

        // Устанавливаем начальное состояние ДКА
        dfa.start_state = "X0";

        // Алфавит ДКА (без "ε")
        vector<string> dfaAlphabet(nfa.alphabet.begin(), nfa.alphabet.end());
        auto it = find(dfaAlphabet.begin(), dfaAlphabet.end(), "ε");
        if (it != dfaAlphabet.end()) dfaAlphabet.erase(it);
        dfa.alphabet = dfaAlphabet;

        // Проверяем финальность начального состояния
        for (const auto& state : startClosure) {
            if (find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
                dfa.final_states.insert("X0");
                break;
            }
        }

        int stateCounter = 1;

        // Обработка всех состояний
        while (!stateQueue.empty()) {
            auto currentStates = stateQueue.front();
            stateQueue.pop();

            string currentStateName = processedStates[currentStates];

            for (const auto& symbol : dfaAlphabet) {
                set<string> reachable;

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
                    // Если новое множество состояний, назначаем ему имя
                    if (processedStates.find(reachable) == processedStates.end()) {
                        string newStateName = "X" + to_string(stateCounter++);
                        processedStates[reachable] = newStateName;

                        // Добавляем в очередь для дальнейшей обработки
                        stateQueue.push(reachable);

                        // Проверяем, является ли новое состояние финальным
                        for (const auto& state : reachable) {
                            if (find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
                                dfa.final_states.insert(newStateName);
                                break;
                            }
                        }
                    }

                    // Добавляем переход
                    dfa.transitions[currentStateName][symbol] = processedStates[reachable];
                } else {
                    // Если перехода нет, оставляем пустой
                    dfa.transitions[currentStateName][symbol] = "-";
                }
            }
        }
    }


    string StateSetToString(const set<string>& states) {
        vector<string> sortedStates(states.begin(), states.end());
        return accumulate(next(sortedStates.begin()), sortedStates.end(), sortedStates[0],
                          [](string a, string b) { return a + "," + b; });
    }

    void PrintDFATransitionTable() const {
        cout << "DFA Transition Table:\n";
        cout << "State";
        for (const auto& symbol : dfa.alphabet) {
            cout << "\t" << symbol;
        }
        cout << "\n";

        for (const auto& [state, transitions] : dfa.transitions) {
            cout << state;
            for (const auto& symbol : dfa.alphabet) {
                if (transitions.count(symbol)) {
                    cout << "\t" << transitions.at(symbol);
                } else {
                    cout << "\t-";
                }
            }
            cout << "\n";
        }

        cout << "\nFinal States:\n";
        for (const auto& state : dfa.final_states) {
            cout << state << "\n";
        }

        cout << "\nStart State:\n" << dfa.start_state << "\n";
    }
};
