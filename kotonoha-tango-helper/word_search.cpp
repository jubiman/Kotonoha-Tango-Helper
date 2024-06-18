//
// Created by jubiman on 3/8/24.
//

#include "word_search.h"

void jubiman::WordSearch::init() {
	// Load words
	std::wstring line;
#ifdef _WIN32
	std::wifstream fs(L"data/data_sorted.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
#else
	std::wifstream fs("data/data_sorted.csv", std::ios::in);
	fs.imbue(std::locale(std::locale("C.UTF-8"), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
#endif
	if (fs.is_open()) {
		while (std::getline(fs, line))
			words.insert(line);
		fs.close();
	}
	skimmed_words = words;

    calculate_best_word();
}

std::vector<uint8_t> unpack_flags(uint8_t flags) {
	std::vector<uint8_t> pos_vec;
	uint8_t count = 0;
	while (flags > 0) {
		// Check if the current position is a 1
		if ((flags & 1) == 1) {
			pos_vec.push_back(count);
		}
		count++;
		flags >>= 1;	// bitwise rightshift
	}
	return pos_vec;
}

/**
 * Filter the words based on the input and the colors of the input
 * @param words The words to filter
 * @param skimmed_words The words to filter from
 * @param input The input to filter by
 */
size_t jubiman::WordSearch::filter_words() {
	// Loop over all the words
	std::unordered_set<std::wstring> results;
	for (const auto& word : skimmed_words) {
		// Skip if it contains a used letter
		for (const auto& letter : word) {
			size_t pos = bad_letters.find(letter);
			if (pos != std::wstring::npos) {
				// Check if we have a yellow letter and/or a good letter
				auto yellow_pos = yellow_letters.find(std::wstring(1, letter));
				if (yellow_pos != yellow_letters.end()) {
					// unpack the positions of the yellow letter from flags into a vector that contains the indices of the yellow letters
					auto pos_vec = unpack_flags(yellow_pos->second);
					// check if the pos vector contains the position of the letter, if it does not, we can still use the word
					if (std::find(pos_vec.begin(), pos_vec.end(), pos) != pos_vec.end()) {
						continue;
					}
				}
				auto good_pos = good_letters.find(std::wstring(1, letter));
				if (good_pos != good_letters.end()) {
					// unpack the positions of the good letter from flags into a vector that contains the indices of the good letters
					auto pos_vec = unpack_flags(good_pos->second);
					// check if the pos vector contains the position of the letter, if it does not, we can still use the word
					if (std::find(pos_vec.begin(), pos_vec.end(), pos) != pos_vec.end()) {
						continue;
					}
				}
				goto next;
			}
		}

		// Skip if it does not have the good letters in the right position
		for (const auto& gl : good_letters) {
			// Unpack the positions of the good letter from flags into a vector that contains the indices of the good letters
			std::vector<uint8_t> pos_vec = unpack_flags(gl.second);
			for (const auto& pos : pos_vec)
				if (std::wstring(1, word.at(pos)) != gl.first)
					goto next;
		}

		// Skip if it has a yellow letter in a known position
		for (const auto& yl : yellow_letters) {
			size_t pos = word.find(yl.first);
			if (pos == std::wstring::npos)
				goto next;

			std::vector<size_t> pos_vec;
			// Repeat till end is reached
			while (pos != std::string::npos) {
				// Add position to the vector
				pos_vec.push_back(pos);
				// Get the next occurrence from the current position
				pos = word.find(yl.first, pos + yl.first.length());
			}
			for (const auto& it : pos_vec)
				if (yl.second & 1 << it)
					goto next;
		}

		results.insert(word);
		next: continue;
	}
	this->skimmed_words = results;
	return results.size();
}

void jubiman::WordSearch::update_colors(ftxui::ColoredText *pText) {
	int i = 0;
	for (const auto& character : *pText) {
		if (character.getBgColor() == ftxui::Color::Default) {
			// Check if this letter is a green or yellow letter, and add it to the bad letters if it is not
			if (good_letters.find(character.getWideCharacter()) == good_letters.end() && yellow_letters.find(character.getWideCharacter()) == yellow_letters.end())
				bad_letters += character.getWideCharacter();
		} else if (character.getBgColor() == ftxui::Color::Green) {
			good_letters[character.getWideCharacter()] |= 1 << i;
		} else if (character.getBgColor() == ftxui::Color::Yellow) {
			// Check if the letter is already in the map good_letters
			if (good_letters.find(character.getWideCharacter()) != good_letters.end()) {
				// If it is, we need to check how many times it occurs in the word
				// If it occurs more than once, we need to add the position to the yellow_letters
				// If it occurs only once, we do not need to add it to the yellow_letters
				size_t pos = pText->getCharacters().find(character.getCharacter());
				if (pText->getCharacters().find(character.getCharacter(), pos + 1) != std::wstring::npos) {
					yellow_letters[character.getWideCharacter()] |= 1 << i;
				}
			} else {
				yellow_letters[character.getWideCharacter()] |= 1 << i;
			}
		}
		++i;
	}
}

size_t jubiman::WordSearch::getWordsLeft() {
	return skimmed_words.size();
}

std::string jubiman::WordSearch::getBestWord() {
	return best_word;
}

void jubiman::WordSearch::calculate_best_word() {
	if (skimmed_words.size() == 1) {
		best_word = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(*skimmed_words.begin());
		return;
	}

	// Create a frequency map of all the letters in the remaining words
	std::unordered_map<wchar_t, int> frequency_map;
	for (const auto& word : skimmed_words) {
		for (const auto& letter : word) {
			frequency_map[letter]++;
		}
	}

	// Sort the frequency map in descending order
	std::vector<std::pair<wchar_t, int>> sorted_map(frequency_map.begin(), frequency_map.end());
	std::sort(sorted_map.begin(), sorted_map.end(), [](const auto& a, const auto& b) {
		return a.second > b.second;
	});

	// Try to find out which words give the most information when used
	const int deduction = (int)skimmed_words.size() / 10;
	std::unordered_map<std::wstring, int32_t> information_map;
	for (const auto& word : skimmed_words) {
		int information = 0;
		std::unordered_set<wchar_t> had_letters;
		for (const auto& letter : word) {
			if (had_letters.find(letter) != had_letters.end()) {
				information -= deduction;
				continue;
			}
			for (const auto& [key, value] : sorted_map) {
				if (key == letter) {
					information += value;
					had_letters.insert(letter);
					break;
				}
			}
		}
		information_map[word] = information;
	}

	// Find the word with the highest information
	std::wstring wbest_word;
	int32_t max_information = 0;
	for (const auto& [key, value] : information_map) {
		if (value > max_information) {
			max_information = value;
			wbest_word = key;
		}
	}

	// Convert the best word to a string
	best_word = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(wbest_word);
}

void jubiman::WordSearch::lock_colors(ftxui::ColoredText *&pText) {
    for (const auto& [key, value] : good_letters) {
        auto pos = unpack_flags(value);
        for (const auto& p : pos) {
            if (pText->getWideCharacters().at(p) == key.at(0)) {
                pText->lockColor(p, ftxui::Color::Green);
            }
        }
    }
    for (const auto& [key, value] : yellow_letters) {
        auto pos = unpack_flags(value);
        for (const auto& p : pos) {
            if (pText->getWideCharacters().at(p) == key.at(0)) {
                pText->lockColor(p, ftxui::Color::Yellow);
            }
        }
    }

    for (const auto& letter : bad_letters) {
        auto pos = pText->getWideCharacters().find(letter);
        while (pos != std::wstring::npos) {
            pText->lockColor(pos, ftxui::Color::Default);
            pos = pText->getWideCharacters().find(letter, pos + 1);
        }
    }

    // New code: Check for good letters in the wrong position
    for (const auto& [key, value] : good_letters) {
        auto pos = pText->getWideCharacters().find(key.at(0));
        while (pos != std::wstring::npos) {
            // If the position is not in the good_letters map, color it yellow
            if ((value & (1 << pos)) == 0) {
                pText->lockColor(pos, ftxui::Color::Yellow);
            }
            pos = pText->getWideCharacters().find(key.at(0), pos + 1);
        }
    }
}
void jubiman::WordSearch::reset() {
	bad_letters.clear();
	good_letters.clear();
	yellow_letters.clear();
	skimmed_words = words;
	calculate_best_word();
}
