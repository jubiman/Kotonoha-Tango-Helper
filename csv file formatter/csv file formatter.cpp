// csv file formatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <codecvt>
#include <set>

int main()
{
	std::set<std::wstring> s;
	// Setup file reader
	std::wstring line;
	std::wifstream fs(L"data/A_data_new.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

	if (fs.is_open())
		while (std::getline(fs, line)) s.insert(line);
	fs.close();

	std::wofstream wof2(L"data/data_sorted.csv");
	wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof2.is_open()) for (std::wstring w : s) wof2 << w << std::endl;
	wof2.close();
}