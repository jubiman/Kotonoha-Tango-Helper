#pragma once

#include <string>

class Word {
public:
	Word(std::wstring, double);
	std::wstring word;
	int score;
};