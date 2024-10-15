//
// Created by jubiman on 3/8/24.
//

#ifndef KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
#define KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
#include <fstream>
#include <unordered_set>
#include <map>
#include <algorithm>
#include "conversion.hpp"
#include "colored_text.h"

namespace jubiman {
	class WordSearch {
	public:
		explicit WordSearch(ftxui::ColoredText *currentText) : currentText(currentText) { init(); }
		WordSearch(const WordSearch& other) : currentText(other.currentText) { init(); }
		WordSearch& operator=(const WordSearch& other) {
			if (this != &other) {
				currentText = other.currentText;
				init();
			}
			return *this;
		}

		size_t filter_words();
		void update_colors(ftxui::ColoredText *pText);
		size_t getWordsLeft() const;
		std::string getBestWord();
		void calculate_best_word();
		void lock_colors(ftxui::ColoredText *&pText);
		void reset();

	private:
		void init();

		ftxui::ColoredText *currentText;

		// TODO: make a it refer to the game's word list?
		std::unordered_set<std::wstring> words;
		std::unordered_set<std::wstring> skimmed_words;

		std::string best_word;
		std::wstring bad_letters;
		std::map<std::wstring, uint8_t> bad_pos_letters;
		std::map<std::wstring, uint8_t> good_letters;	// char, pos
		std::map<std::wstring, uint8_t> yellow_letters;	// char, pos

		// std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	};
} // namespace jubiman

#endif //KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
