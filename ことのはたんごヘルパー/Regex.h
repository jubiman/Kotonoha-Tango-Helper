#pragma once

#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <vector>
#include <tuple>

#include "word.h"

class Regex {
private:
	void init();
	std::wstring query;
	std::vector<std::wstring> results;
	std::vector<Word> good_words;
	std::vector<wchar_t> used_letters;
public:
	Regex();
	std::wregex hiragana_katakana_x;
	std::wregex all;
	std::wregex non_japanese;
	std::wregex hiragana_katakana_punctuation;
	std::wregex hiragana_katakana;
	std::wregex hiragana;
	std::wregex full_width_katakana;
	std::wregex half_width_katakana;
	int input(std::wstring);
	bool isValidInput(std::wstring);
	int search_all();
	std::vector<std::wstring> get_matches();
	std::wstring find_best_word();
	void add_used_letters(std::wstring);
};