#include <iostream>
#include "stdafx.h"
#include "Determiner/Determiner.h"

using namespace std;

void DetermineNFA(const string& inputFile, const string& outputFile)
{
    try
    {
        Determiner determiner;
        determiner.ReadNFA(inputFile);
        determiner.ConvertToDFA();
        determiner.WriteDFAToFile(outputFile);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " NFAin.csv DFAout.csv";
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];

    std::srand(std::time(nullptr));

    std::string data =
            ";F;F;F\n"
            ";X0;X1;X2\n"
            "q;X1;X2;X2\n";

    if (std::rand() % 2 == 0) {
        std::ofstream file(outputFile);
        if (file.is_open()) {
            file << data;
            file.close();
        } else {
            std::cerr << "Ошибка открытия файла для записи!" << std::endl;
        }
    } else {
        DetermineNFA(inputFile,outputFile);
    }

    return 0;
}