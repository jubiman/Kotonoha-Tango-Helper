// ことのはたんごヘルパー.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include "regex.h"


int main()
{
    Regex reg;
    std::wstring inp;
    std::wcin.imbue(std::locale("Japanese"));
    std::wcout.imbue(std::locale("Japanese"));
    std::cout << "Finished initializing!" << std::endl;

    std::wcin >> inp;
    reg.input(inp);
    std::cout << "Found " << reg.search_all() << " matches." << std::endl;

    std::wofstream wof(L"output/output.txt");
    wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    if(wof.is_open()) for (std::wstring match : reg.get_matches()) wof << match << std::endl;
    wof.close();

    std::wcout<< L"Best word: " << reg.find_best_word() << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
