﻿#pragma once

#include <string>
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <math.h>


namespace jubiman {
	class Regex {
	private:
		void init();
		std::wstring query;
		std::unordered_set<std::wstring> results;
		std::unordered_set<std::wstring> words;
		std::unordered_set<std::wstring> skimmed_words;
		std::wstring bad_letters;
		std::unordered_map<std::wstring, double> good_words;
		std::map<std::wstring, unsigned char> bad_pos_letters;
		std::map<std::wstring, unsigned char> good_letters; // char, pos
		std::map<std::wstring, unsigned char> yellow_letters; // char, pos
	public:
		Regex();
		unsigned char good_letters_size;
		int search();
		std::unordered_set<std::wstring> get_matches();
		std::wstring find_best_word();
		std::wstring find_best_word_last();
		void add_bad_letters(std::wstring);
		void add_good_letters(std::wstring);
		void add_yellow_letters(std::wstring);
		int set_query_and_search();
		int check_yellow_letters();
		void skim_words();
	};
}