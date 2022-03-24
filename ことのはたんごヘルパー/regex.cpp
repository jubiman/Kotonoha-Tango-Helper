#include "regex.h"


jubiman::Regex::Regex() {
	init();
}

void jubiman::Regex::init() {
	//jubiman::Regex::query = std::wstring(std::allocator<wchar_t>());

	jubiman::Regex::hiragana_katakana_x.assign(L"/[ぁ-ゖァ-ヺ\wx]/g");
	jubiman::Regex::all.assign(L"/[一-龯]/g");
	jubiman::Regex::non_japanese.assign(L"/[^ぁ-ゖァ-ヺ]/g");
	jubiman::Regex::hiragana_katakana_punctuation.assign(L"/[ぁ-ゖァ-ヺ\w]/g");
	jubiman::Regex::hiragana_katakana.assign(L"[ぁ-ゖァ-ヺ]");
	jubiman::Regex::hiragana.assign(L"/[ぁ-ゖ]/g");
	jubiman::Regex::full_width_katakana.assign(L"/[ァ-ヺ]/g");
	jubiman::Regex::half_width_katakana.assign(L"/[ｦ-ﾝ]/g");
}

/**
 * Puts whole input to query, if valid input.
 *
 * @param input -- Contains the whole input of length 5 to push to the query.
 * @return 0 on success, 1 on failure .
 */
int jubiman::Regex::input(std::wstring input) {
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
bool jubiman::Regex::isValidInput(std::wstring input) {
	std::wsmatch wsm;
	std::regex_search(input, wsm, hiragana_katakana_x);
	if (wsm.ready() && !wsm.empty()) return false;
	return true;
}

template <typename T>
void jubiman::Regex::unique_elements(std::vector<T>& vec)
{
	std::map<T, int> m;

	for (auto p = vec.begin(); p != vec.end(); ++p)
		m[*p]++;

	vec.clear();

	for (auto p = m.begin(); p != m.end(); ++p) vec.push_back(p->first);
		//if (p->second == 1) vec.push_back(p->first);
}


/**
 * Searches all words. That's it.
 *
 * @returns The amount of matches.
 */
int jubiman::Regex::search() {
	if (query.empty()) return 0; // Return if query is empty

	// Setup file reader
	std::wstring line;
	std::wifstream fs(L"./data/A_data_new.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

	// Setup regex matches
	std::wsmatch wsm;

	// Clear the results vector
	results.clear();

	// Count lines
	unsigned int checked = 0;
	std::wcout << L"Searching: " << query << std::endl;
	if (fs.is_open()) {
		while (std::getline(fs, line)) {
			checked++;
			std::regex_search(line, wsm, std::wregex(query));
			if (wsm.ready() && wsm.empty()) continue;

			// Push back the result
			results.push_back(line);
			//std::wcout << L"Found a match: " << line << std::endl;
		}
		std::cout << "Checked " << checked << " lines." << std::endl;
	}
	fs.close();

	// Remove duplicate matches
	/* Slow: O(n^2)
	auto end = results.end();
	for (auto it = results.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
		matches--;
	}
	results.erase(end, results.end());*/
	unique_elements(results);

	return results.size();
}

/**
 * Searches all words.
 * 
 * @returns The amount of matches.
 */
int jubiman::Regex::search_all() {
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

std::vector<std::wstring> jubiman::Regex::get_matches() {
	return results;
}

std::wstring jubiman::Regex::find_best_word() {
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

	std::wofstream wof(L"output/output2.txt");
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof.is_open()) for (auto b : freq) wof << std::get<0>(b) << L" " << std::get<1>(b) << std::endl;
	wof.close();


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
			std::wstring had = L"";

			for (wchar_t l : line) {
				if (had.find(l) != std::wstring::npos) continue;
				int let_freq = 0;
				for (size_t i = 0; i < freq.size(); ++i) {
					if (std::get<0>(freq[i]) == l) let_freq = std::get<1>(freq[i]);
				}
				// Check for duplicate letters
				if (size_t count = std::count(line.begin(), line.end(), l) > 0) {
					score -= count*10;
				}
				
				score += let_freq;
				had += l;
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

	std::wofstream wof2(L"output/output3.txt");
	wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof2.is_open()) for (auto b : good_words) wof2 << b.word << L" " << b.score << std::endl;
	wof2.close();


	return good_words[0].word;
}

void jubiman::Regex::add_used_letters(std::wstring letters) {
	for (wchar_t letter : letters) used_letters.push_back(letter);
}

// Add bad letters
void jubiman::Regex::add_bad_letters(std::wstring bl) {
	bad_letters += bl;
}

// Add good letters
void jubiman::Regex::add_good_letters(std::wstring gl) {
	if (gl.empty()) return;
	std::vector<std::wstring> strtuples;
	size_t pos = 5;
	while ((pos = gl.find(std::wstring(L") ("))) != std::wstring::npos) {
		strtuples.push_back(gl.substr(0, pos + 1));
		gl.erase(0, pos + 2);
	}
	strtuples.push_back(gl.substr(0, pos));
	for (std::wstring strtup : strtuples) {
		good_letters.push_back(std::tuple<std::wstring, int>(std::wstring(1, strtup.at(1)), _wtoi(strtup.substr(4, 1).c_str())));
	}
}

// Add yellow letters
void jubiman::Regex::add_yellow_letters(std::wstring yl) {
	if (yl.empty()) return;
	std::vector<std::wstring> strtuples;
	size_t pos = 5;
	while ((pos = yl.find(L") (")) != std::wstring::npos) {
		strtuples.push_back(yl.substr(0, pos + 1));
		yl.erase(0, pos + 3);
	}
	strtuples.push_back(yl.substr(0, pos));
	for (std::wstring strtup : strtuples) {
		yellow_letters.push_back(std::tuple<std::wstring, int>(std::wstring(1, strtup.at(1)), _wtoi(strtup.substr(4, 1).c_str())));
	}
}

int jubiman::Regex::set_query_and_search() {
	std::wstring tmp = L".....";
	std::wstring arr[5] = {}, notArr[5] = {};

	// Replace known letters
	for (std::tuple<std::wstring, int> g : good_letters)
		tmp.replace(std::get<1>(g), 1, std::get<0>(g));

	// Make not or char letter regex
	for (size_t i = 0; i < 5; ++i)
		if (tmp.at(i) == L'.')
			for (std::tuple<std::wstring, int> y : yellow_letters)
				std::get<1>(y) != i ? arr[i] += std::get<0>(y) : notArr[i] += std::get<0>(y);

	for (unsigned int i = 0, j = 0; i < 5; ++i, ++j)
		if (arr[i] != L"") {
			if (arr[i].length() > 1) {
				tmp.replace(j, 1, L"([^" + bad_letters + L"]|[" + arr[i] + L"])");
				j += std::wstring(L"([^" + bad_letters + L"]|[" + arr[i] + L"])").length()-1;
			}
			else {
				tmp.replace(j, 1, L"([^" + bad_letters + L"]|" + arr[i] + L")");
				j += std::wstring(L"([^" + bad_letters + L"]|" + arr[i] + L")").length()-1;
			}
		}

	/*for (size_t i = 0, j = 0; i < tmp.length(); ++i, ++j)
		if (tmp.at(i) == L'.') {
			tmp.replace(i, 1, L"[^" + bad_letters + notArr[j] + L"]");
			i += std::wstring(L"[^" + bad_letters + notArr[j] + L"]").length()-1;
		}*/
	size_t pos = 0, i = 0, j;
	while (tmp.at(0) == L'.' || (pos = tmp.find(L".", pos)) != std::wstring::npos) {
		j = 0;
		while ((i = tmp.find(L')', i + 1)) != std::wstring::npos && i < pos) {
			++j;
		}
		tmp.replace(pos, 1, L"[^" + bad_letters + notArr[j] + L"]");
		pos += std::wstring(L"[^" + bad_letters + notArr[j] + L"]").length() - 1;
	}

	query = tmp;
	search();

	return check_yellow_letters();
}

int jubiman::Regex::check_yellow_letters() {
	for (std::tuple<std::wstring, int> t : yellow_letters)
		for (size_t i = 0; i < results.size(); ++i)
			if (results[i].find(std::get<0>(t)) == std::wstring::npos) {
				results.erase(results.begin() + i);
				--i;
			}
	return results.size();
}