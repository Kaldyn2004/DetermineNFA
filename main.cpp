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
    DetermineNFA(inputFile,outputFile);

    return 0;
}