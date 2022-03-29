#include "regex.h"


jubiman::Regex::Regex() {
	init();
}

void jubiman::Regex::init() {
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

/**
 * Searches all words. That's it.
 *
 * @returns The amount of matches.
 */
int jubiman::Regex::search() {
	if (query.empty()) return 0; // Return if query is empty

	// Setup file reader
	std::wstring line;
	std::wifstream fs(L"data/data_sorted.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

	// Setup regex matches
	std::wsmatch wsm;

	// Clear the results map
	results.clear();

	// Count lines
	unsigned int checked = 0;
	std::wcout << L"Searching: " << query << std::endl;
	if (fs.is_open()) {
		while (std::getline(fs, line)) {
			checked++;
			std::regex_search(line, wsm, std::wregex(query));
			if (wsm.ready() && wsm.empty()) continue;

			// Push back the result only if it contains all the yellow letters
			bool skip = false;
			for (const auto& t : yellow_letters)
				if (line.find(t.first) == std::wstring::npos) {
					skip = true; break;
				}
			if (!skip) results.insert(line);
		}
		std::cout << "Checked " << checked << " lines." << std::endl;
	}
	fs.close();

	return results.size();
}

std::unordered_set<std::wstring> jubiman::Regex::get_matches() {
	return results;
}

std::wstring jubiman::Regex::find_best_word() {
	std::cout << "Finding best word..." << std::endl;

	// Calculate frequencies of unfound letters
	std::map<wchar_t, int> freq;
	for (std::wstring word : results)
		for (wchar_t l : word)
			if (!yellow_letters.count(std::wstring(1, l)))
				freq[l]++;
	

	// Sort array on ascending order
	/*std::sort(freq.begin(), freq.end(), [](const std::tuple<wchar_t, int>& a, const std::tuple<wchar_t, int>& b) -> bool {
		return (std::get<1>(a) < std::get<1>(b));
	});*/

	std::wofstream wof(L"output/frequencies.txt");
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof.is_open()) for (const auto &b : freq) wof << b.first << L" " << b.second << std::endl;
	wof.close();


	// Setup file reader
	std::wstring line;
	//std::wifstream fs(L"data/data_sorted.csv", std::ios::in);
	std::wifstream fs(L"./output/possible words.txt", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

	good_words.clear();

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
			for (wchar_t l : bad_letters) {
				if (line.find(l) != std::wstring::npos) { skip = true; break; }
			}
			if (skip) continue;
			

			// Calculate score
			double score = 0;
			std::wstring had = L"";

			for (wchar_t l : line) {
				if (good_letters.count(std::wstring(1, l)) || had.find(l) != std::wstring::npos) continue;
				// Check for duplicate letters
				if (size_t count = std::count(line.begin(), line.end(), l) > 0) {
					score -= count*10;
				}
				score += freq[l];
				had += l;
			}

			// Push back the result
			good_words[line] = score;

			//matches++;
		}
	}
	fs.close();

	// Remove duplicate words
	/*auto end = good_words.end();
	for (auto it = good_words.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
		//matches--;
	}
	good_words.erase(end, good_words.end());*/

	// Sort words by score (ascending) (bypass hahahahaha)
	std::vector<std::pair<std::wstring, double>> v(good_words.begin(), good_words.end());
	std::sort(v.begin(), v.end(), [](const std::pair<std::wstring, double>& a, const std::pair<std::wstring, double>& b) -> bool {
		return (a.second > b.second);
	});
	good_words.clear();
	for (const auto& i : v) good_words[i.first] = i.second;

	std::wofstream wof2(L"output/good words.txt");
	wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof2.is_open()) for (const auto& b : good_words) wof2 << b.first << L" " << b.second << std::endl;
	wof2.close();

	return (*good_words.begin()).first;
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
		good_letters[std::wstring(1, strtup.at(1))] = good_letters[std::wstring(1, strtup.at(1))] | (int)pow(2, _wtoi(strtup.substr(4, 1).c_str()));
		yellow_letters.erase(std::wstring(1, strtup.at(1)));
	}
}

// Add yellow letters
void jubiman::Regex::add_yellow_letters(std::wstring yl) {
	if (yl.empty()) return;
	std::vector<std::wstring> strtuples;
	size_t pos = 5;
	while ((pos = yl.find(L") (")) != std::wstring::npos) {
		strtuples.push_back(yl.substr(0, pos + 1));
		yl.erase(0, pos + 2);
	}
	strtuples.push_back(yl.substr(0, pos));
	for (std::wstring strtup : strtuples) {
		yellow_letters[std::wstring(1, strtup.at(1))] = yellow_letters[std::wstring(1, strtup.at(1))] | (int)pow(2, _wtoi(strtup.substr(4, 1).c_str()));
	}
}

int jubiman::Regex::set_query_and_search() {
	std::wstring tmp = L".....";
	std::wstring arr[5] = {}, notArr[5] = {};

	// Replace known letters
	for (const auto& g : good_letters)
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			g.second == (g.second | i) ? tmp.replace(j, 1, g.first) : L"";
			//g.second == (g.second | (int)pow(2, i)) ? tmp.replace(i, 1, g.first) : L"";

	// g.second = 0b10010
	// g.second | (int)pow(2, i) -> ans
	// 0b10010 | 0b00001 -> 0b10011 false pos 2^0
	// 0b10010 | 0b00010 -> 0b10010 true  pos 2^1
	// 0b10010 | 0b00100 -> 0b10110 false pos 2^2
	// 0b10010 | 0b01000 -> 0b11010 false pos 2^3
	// 0b10010 | 0b10000 -> 0b10010 true  pos 2^4
	//

	// Make not or char letter regex
	for (auto const& x : yellow_letters)
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			tmp.at(j) == L'.' ? ((x.second != (i | x.second)) ? ((arr[j].find(x.first) == std::wstring::npos) ? arr[j] += x.first : L"") : ((notArr[j].find(x.first) == std::wstring::npos) ? notArr[j] += x.first : L"")) : L"";

	for (unsigned int i = 0, j = 0; i < 5; ++i, ++j)
		if (arr[i] != L"") {
			if (arr[i].length() > 1) {
				tmp.replace(j, 1, L"([^" + bad_letters + notArr[i] + L"]|[" + arr[i] + L"])");
				j += std::wstring(L"([^" + bad_letters + notArr[i] + L"]|[" + arr[i] + L"])").length()-1;
			}
			else if (arr[i].length() == 1) {
				tmp.replace(j, 1, L"([^" + bad_letters + notArr[i] + L"]|" + arr[i] + L")");
				j += std::wstring(L"([^" + bad_letters + notArr[i] + L"]|" + arr[i] + L")").length()-1;
			}
		}

	size_t pos = 0, i, j;
	while (tmp.at(0) == L'.' || (pos = tmp.find(L".", pos)) != std::wstring::npos) {
		j = 0; i = 0;
		while ((i = tmp.find(L']', i + 1)) != std::wstring::npos && i < pos) ++j;
		if (j > good_letters.size()) j += good_letters.size();
		tmp.replace(pos, 1, L"[^" + bad_letters + notArr[j] + L"]");
		pos += std::wstring(L"[^" + bad_letters + notArr[j] + L"]").length() - 1;
	}

	query = tmp;

	return search();
	//return check_yellow_letters();
}

int jubiman::Regex::check_yellow_letters() {
	for (const auto& t : yellow_letters)
		for (const auto& i : results)
			if ((*results.find(i)).find(t.first) == std::wstring::npos) {
				results.erase(i);
			}
	return results.size();
}

// OLD: vector way
/*int jubiman::Regex::check_yellow_letters() {
	for (const auto& t : yellow_letters)
		for (size_t i = 0; i < results.size(); ++i)
			if (results[i].find(t.first) == std::wstring::npos) {
				results.erase(results.begin() + i);
				--i;
			}
	return results.size();
}*/