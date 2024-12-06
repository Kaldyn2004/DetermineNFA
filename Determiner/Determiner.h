#pragma once
#include "../stdafx.h"
using namespace std;

class Determiner
{
public:
    void ReadNFA(const string &filename)
    {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        string line;
        vector<string> headers;

        if (getline(file, line))
        {
            istringstream headerStream(line);
            string cell;
            while (getline(headerStream, cell, ';'))
            {
                headers.push_back(cell);
            }
        }

        if (getline(file, line))
        {
            istringstream stateStream(line);
            string cell;
            int i = 0;

            while (getline(stateStream, cell, ';'))
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

        while (getline(file, line)) {
            istringstream rowStream(line);
            string cell;
            string symbol;
            if (getline(rowStream, cell, ';')) {
                if (!cell.empty())
                {
                    nfa.alphabet.push_back(cell);
                    symbol = cell;
                }
            }

            int i = 0;
            while (getline(rowStream, cell, ';'))
            {
                if (!cell.empty())
                {
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
            nfa.startState = *nfa.states.begin();
        }
    }

    void ConvertToDFA() {
        СomputeAllEpsilonClosures();
        BuildDFATransitionTable();
        PrintDFATransitionTable();
    }

    void СomputeAllEpsilonClosures() {
        cout << nfa.startState << endl;
        for (const auto& state : nfa.states) {
            cout << state << endl;
            vector<string> closure = computeEpsilonClosure(state, nfa.transitions);
            nfa.epsilonClosures.emplace_back(state, closure);
        }
    }

    void WriteDFAToFile(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        dfa.final_states.contains(dfa.start_state)
        ? file << ";F"
        : file << ";";

        for (const auto& [state, transitions] : dfa.transitions) {
            if (state != dfa.start_state)
            {
                dfa.final_states.contains(state)
                ? file << ";F"
                : file << ";";
            }
        }
        file << endl;
        map<string, string> renamedState;

        int i;
        for (const auto& [state, transitions] : dfa.transitions) {
            if (state == dfa.start_state)
            {
                renamedState[state] = "q" + to_string(i);
                file << ";" << renamedState[state];
            }

            i++;
        }

        i = 0;
        for (const auto& [state, transitions] : dfa.transitions) {
            if (state != dfa.start_state)
            {
                renamedState[state] = "q" + to_string(i);
                file << ";" << renamedState[state];
            }
            i++;
        }
        file << endl;

        for (const auto& symbol : dfa.alphabet) {
            file << symbol;

            for (const auto& [state, transitions] : dfa.transitions) {
                if (state == dfa.start_state)
                {
                    if (transitions.count(symbol)) {
                        file << ";" << renamedState[transitions.at(symbol)];
                    } else
                    {
                        file << ";";
                    }
                }

            }

            for (const auto& [state, transitions] : dfa.transitions) {
                if (state != dfa.start_state)
                {
                    if (transitions.count(symbol)) {
                        file << ";" << renamedState[transitions.at(symbol)];
                    } else
                    {
                        file << ";";
                    }
                }
            }
            file << endl;
        }
    }


    static vector<string> computeEpsilonClosure(
            const string& state,
            const unordered_map<string, unordered_map<string, vector<string>>>& transitions
    ) {
        unordered_set<string> closure;
        vector<string> stack = {state};

        while (!stack.empty()) {
            string current = stack.back();
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

        return vector<string>(closure.begin(), closure.end());
    }

private:
    struct NFA
    {
        vector<string> states;
        vector<string> alphabet;
        unordered_map<string, unordered_map<string, vector<string>>> transitions;
        string startState;
        vector<string> finalStates;
        vector<pair<string, vector<string>>> epsilonClosures;
    };

    struct DFA
    {
        set<string> states;
        vector<string> alphabet;
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
        queue<set<string>> stateQueue;

        // Начальное состояние ДКА — ε-замыкание начального состояния НКА
        auto startClosure = EpsilonClosure(nfa.startState);
        stateQueue.push(startClosure);

        // Множество всех уникальных состояний (для предотвращения дублирования)
        set<set<string>> processedStates;
        processedStates.insert(startClosure);

        // Устанавливаем начальное состояние ДКА
        string startStateName = StateSetToString(startClosure);
        dfa.start_state = startStateName;

        // Алфавит ДКА (без "ε")
        vector<string> dfaAlphabet(nfa.alphabet.begin(), nfa.alphabet.end());
        auto it = find(dfaAlphabet.begin(),dfaAlphabet.end(), "ε");
        if(it != dfaAlphabet.end()) //если найден
            dfaAlphabet.erase(it);
        dfa.alphabet = dfaAlphabet;

        // Проверяем финальность начального состояния
        for (const auto& state : startClosure) {
            if (find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
                dfa.final_states.insert(startStateName);
                break;
            }
        }

        // Обработка всех состояний
        while (!stateQueue.empty()) {
            auto currentStates = stateQueue.front();
            stateQueue.pop();

            string currentStateName = StateSetToString(currentStates);

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
                    string reachableStateName = StateSetToString(reachable);

                    // Добавляем переход
                    dfa.transitions[currentStateName][symbol] = reachableStateName;

                    // Если это новое множество состояний, добавляем его в очередь
                    if (processedStates.find(reachable) == processedStates.end()) {
                        stateQueue.push(reachable);
                        processedStates.insert(reachable);

                        // Проверяем, является ли новое состояние финальным
                        for (const auto& state : reachable) {
                            if (find(nfa.finalStates.begin(), nfa.finalStates.end(), state) != nfa.finalStates.end()) {
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
    string StateSetToString(const set<string>& states) {
        vector<string> sortedStates(states.begin(), states.end());
        return accumulate(next(sortedStates.begin()), sortedStates.end(),
                                     sortedStates[0], [](string a, string b) { return a + "," + b; });
    }

    void PrintDFATransitionTable() const {
        cout << "DFA Transition Table:\n";

        // Выводим заголовок
        cout << "State";
        for (const auto& symbol : dfa.alphabet) {
            cout << "\t" << symbol;
        }
        cout << "\n";

        // Выводим каждую строку таблицы переходов
        for (const auto& [state, transitions] : dfa.transitions) {
            cout << state;
            for (const auto& symbol : dfa.alphabet) {
                if (transitions.count(symbol)) {
                    cout << "\t" << transitions.at(symbol);
                } else {
                    cout << "\t-"; // Для отсутствующих переходов
                }
            }
            cout << "\n";
        }

        // Вывод финальных состояний
        cout << "\nFinal States:\n";
        for (const auto& state : dfa.final_states) {
            cout << state << "\n";
        }

        // Стартовое состояние
        cout << "\nStart State:\n" << dfa.start_state << "\n";
    }
};

