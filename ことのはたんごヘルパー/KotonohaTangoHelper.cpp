// ことのはたんごヘルパー.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "regex.h"

int main()
{
    jubiman::Regex reg;
    std::wstring inp, bl, dgl, gl, yl;
    std::wcin.imbue(std::locale("Japanese"));
    std::wcout.imbue(std::locale("Japanese"));
     
    int nError = 0;
    #ifdef _WIN32
    nError = _wmkdir(L"./output"); // can be used on Windows
    #else // Don't know if this works, can't test. Pls PR if u can
    mode_t nMode = 0733; // UNIX style permissions
    nError = wmkdir(L"output", nMode); // can be used on non-Windows
    #endif
    /*if (nError != 0 && nError != EEXIST) {
        std::cout << "Could not create directory. Path not valid, exiting...\n";
        exit(1);
    }*/


    std::cout << "Finished initializing!" << std::endl;

    bool solved = false;
    while (!solved) {
        std::cout << "Bad letters (wstring):" << std::endl;
        std::getline(std::wcin, bl);
        reg.add_bad_letters(bl);
        std::cout << "Duplicate gray letters L\"(wchar_t, int)\" seperated by a space with the position being 0-based index:" << std::endl;
        std::getline(std::wcin, dgl);
        reg.add_duplicate_gray_letters(dgl);
        std::cout << "Good letters L\"(wchar_t, int)\" seperated by a space with the position being 0-based index:" << std::endl;
        std::getline(std::wcin, gl);
        reg.add_good_letters(gl);
        std::cout << "Yellow letters L\"(wchar_t, int)\" seperated by a space with the position being 0-based index:" << std::endl;
        std::getline(std::wcin, yl);
        reg.add_yellow_letters(yl);

        int m;
        std::cout << "Found " << (m = reg.search()) << " matches." << std::endl;
        if (m == 1) {
            std::wcout << L"Answer: " << *reg.get_matches().begin() << std::endl;
            solved = true;
            break;
        }
        // Write matches to file for easier read/search
        std::wofstream wof2(L"output/possible words.txt");
        wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
        if (wof2.is_open()) for (std::wstring match : reg.get_matches()) wof2 << match << std::endl;
        wof2.close();
        // TODO: add 2 open letter searching
        std::wcout << L"Best word: " << (reg.good_letters_size >= 4 ? reg.find_best_word_last() : reg.find_best_word()) << std::endl;
        reg.skim_words();
    }
}