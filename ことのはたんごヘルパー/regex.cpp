#include "regex.h"


Regex::Regex() {
	init();
}

void Regex::init() {
	//Regex::query = std::wstring(std::allocator<wchar_t>());

	Regex::hiragana_katakana_x.assign(L"/[ぁ-ゖァ-ヺ\wx]/g");
	Regex::all.assign(L"/[一-龯]/g");
	Regex::non_japanese.assign(L"/[^ぁ-ゖァ-ヺ]/g");
	Regex::hiragana_katakana_punctuation.assign(L"/[ぁ-ゖァ-ヺ\w]/g");
	Regex::hiragana_katakana.assign(L"[ぁ-ゖァ-ヺ]");
	Regex::hiragana.assign(L"/[ぁ-ゖ]/g");
	Regex::full_width_katakana.assign(L"/[ァ-ヺ]/g");
	Regex::half_width_katakana.assign(L"/[ｦ-ﾝ]/g");
}

/**
 * Puts whole input to query, if valid input.
 *
 * @param input -- Contains the whole input of length 5 to push to the query.
 * @return 0 on success, 1 on failure .
 */
int Regex::input(std::wstring input) {
	std::cout << "Length: " << input.length() << std::endl;
	// Japanese input has length 2 per character
	//if (input.length() == 10 && isValidInput(input)) {
	if (isValidInput(input)) {
		query = input;
		
		size_t s = 0;
		while ((s = query.find(L"x", s)) != std::wstring::npos) {
			query.replace(s, 1, L".");
			s++;
		}
		while ((s = query.find(L"。", s)) != std::wstring::npos) {
			query.replace(s, 1, L".");
			s++;
		}
		while ((s = query.find(L"ｘ", s)) != std::wstring::npos) {
			query.replace(s, 1, L".");
			s++;
		}

		//std::wcout << L"Query: " << query << std::endl;
		return 0;
	}
	return 1;
}

/**
 * Checks if the input is vald (whether it contains valid characters)
 * 
 * @param input -- Contains the input to check.
 * @returns true if valid, otherwise false.
 */
bool Regex::isValidInput(std::wstring input) {
	std::wsmatch wsm;
	std::regex_search(input, wsm, hiragana_katakana_x);
	if (wsm.ready() && !wsm.empty()) return false;
	return true;
}

/**
 * Searches all words.
 * 
 * @returns The amount of matches.
 */
int Regex::search_all() {
	if (query.empty()) return 0; // Return if query is empty

	// Add query to used letters (only actual letters, not the regex part)
	std::wstring letters;
	std::wsmatch res;
	std::wstring quer = query;
	while (std::regex_search(quer, res, hiragana_katakana)) {
		if (res.ready() && !res.empty()) for (auto x : res) letters += x;
		quer = res.suffix().str();
	}
	add_used_letters(letters);

	// Setup file reader
	std::wstring line;
	std::wifstream fs(L"./data/A_data_new.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	
	// Setup regex matches
	std::wsmatch wsm;

	// Count lines
	unsigned int checked = 0, matches = 0;
	std::wcout << L"Searching: " << query << std::endl;
	if (fs.is_open()) {
		while (std::getline(fs, line)) {
			checked++;
			std::regex_search(line, wsm, std::wregex(query));
			if (wsm.ready() && wsm.empty()) continue;

			// Push back the result
			results.push_back(line);
			std::wcout << L"Found a match: " << line << std::endl;
			matches++;
		}
		std::cout << "Checked " << checked << " lines." << std::endl;
	}
	fs.close();

	// Remove duplicate matches
	auto end = results.end();
	for (auto it = results.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
		matches--;
	}
	results.erase(end, results.end());

	return matches;
}

std::vector<std::wstring> Regex::get_matches() {
	return results;
}

std::wstring Regex::find_best_word() {
	std::cout << "Finding best word..." << std::endl;

	// Calculate frequencies of unfound letters
	std::vector<std::tuple<wchar_t, int>> freq;
	std::wsmatch m;
	for (std::wstring word : results) {
		std::regex_search(word, m, std::wregex(query));
		if (m.ready() && m.empty()) continue;
		
		for (wchar_t l : m.suffix().str()) {
			for (size_t i = 0; i < freq.size(); ++i) {
				if (l == std::get<0>(freq[i])) {
					std::get<1>(freq[i])++; goto cnt;
				}
			}
			freq.push_back(std::tuple<wchar_t, int>(l, 1));
		cnt:;
		}
	}

	// Sort array on ascending order
	std::sort(freq.begin(), freq.end(), [](const std::tuple<wchar_t, int>& a, const std::tuple<wchar_t, int>& b) -> bool {
		return (std::get<1>(a) < std::get<1>(b));
	});

	// Setup file reader
	std::wstring line;
	std::wifstream fs(L"./data/A_data_new.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

	// Count lines
	//unsigned int checked = 0, matches = 0;
	if (fs.is_open()) {
		while (std::getline(fs, line)) {
			//checked++;
			//std::regex_search(line, wsm, std::wregex(L"[^" + std::wstring(used_letters.begin(), used_letters.end()) +L"]"));
			//std::regex_search(line, wsm, regex);
			//if (wsm.ready() && wsm.empty()) continue;
			

			// Check if any of the used letters is in the string
			bool skip = false;
			for (wchar_t l : used_letters) {
				if (line.find(l) != std::wstring::npos) { skip = true; break; }
			}
			if (skip) continue;
			 
			
			// Calculate score
			double score = 0;

			for (wchar_t l : line) {
				// Check for duplicate letters
				if (size_t count = std::count(line.begin(), line.end(), l) > 0) score -= count;
				
				for (size_t i = 0; i < freq.size(); ++i) {
					if (std::get<0>(freq[i]) == l) score += std::get<1>(freq[i]);
				}

			}

			// Push back the result
			good_words.push_back(Word(line, score));

			//std::wcout << L"Found a match: " << line << std::endl;
			//matches++;
		}
		//std::cout << "Checked " << checked << " lines." << std::endl;

	}
	fs.close();

	// Remove duplicate words
	/*auto end = good_words.end();
	for (auto it = good_words.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
		//matches--;
	}
	good_words.erase(end, good_words.end());*/

	// Sort words by score (ascending)
	std::sort(good_words.begin(), good_words.end(), [](const Word& a, const Word& b) -> bool {
		return (a.score > b.score);
	});

	std::wofstream wof(L"output/output2.txt");
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof.is_open()) for (auto b : good_words) wof << b.word << L" " << b.score << std::endl;
	wof.close();


	return good_words[0].word;
}

void Regex::add_used_letters(std::wstring letters) {
	for (wchar_t letter : letters) used_letters.push_back(letter);
}