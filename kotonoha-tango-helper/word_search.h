//
// Created by jubiman on 3/8/24.
//

#ifndef KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
#define KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
#include <fstream>
#include <unordered_set>
#include <map>

#include "colored_text.h"
namespace jubiman {
	class WordSearch {
	public:
		WordSearch(ftxui::ColoredText *currentText) : currentText(currentText) { init(); }
		size_t filter_words();

		void update_colors(ftxui::ColoredText *pText);

		size_t getWordsLeft();

		std::string getBestWord();

		void calculate_best_word();

	private:
		ftxui::ColoredText *currentText;

		void init();

		std::unordered_set<std::wstring> words;
		std::unordered_set<std::wstring> skimmed_words;

		std::string best_word;
		std::wstring bad_letters;
		std::map<std::wstring, uint8_t> bad_pos_letters;
		std::map<std::wstring, uint8_t> good_letters;	// char, pos
		std::map<std::wstring, uint8_t> yellow_letters;	// char, pos
	};
} // namespace jubiman

#endif //KOTONOHA_TANGO_HELPER_WORD_SEARCH_H
