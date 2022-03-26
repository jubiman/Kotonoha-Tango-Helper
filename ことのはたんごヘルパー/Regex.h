#pragma once

#include <string>
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <vector>
#include <tuple>
#include <map>
#include <math.h>


namespace jubiman {
	class Regex {
	private:
		void init();
		std::wstring query;
		std::vector<std::wstring> results;
		std::vector<std::tuple<std::wstring, double>> good_words;
		std::vector<wchar_t> used_letters;
		std::wstring bad_letters;
		//std::vector<std::tuple<std::wstring, int>> good_letters; // char, pos
		std::map<std::wstring, unsigned char> good_letters; // char, pos
		std::map<std::wstring, unsigned char> yellow_letters; // char, pos
		template <typename T> void unique_elements(std::vector<T>& vec);
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
		int search();
		int search_all();
		std::vector<std::wstring> get_matches();
		std::wstring find_best_word();
		void add_used_letters(std::wstring);
		void add_bad_letters(std::wstring);
		void add_good_letters(std::wstring);
		void add_yellow_letters(std::wstring);
		int set_query_and_search();
		int check_yellow_letters();
	};
}