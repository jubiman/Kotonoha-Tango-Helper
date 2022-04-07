#include "regex.h"


jubiman::Regex::Regex() {
	good_letters_size = 0;
	
	// Load words
	std::wstring line;
	std::wifstream fs(L"data/data_sorted.csv", std::ios::in);
	fs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	if (fs.is_open()) while (std::getline(fs, line)) words.insert(line); fs.close();
	skimmed_words = words;
}

/**
 * Searches all words. That's it.
 *
 * @returns The amount of matches.
 */
int jubiman::Regex::search() {
	if (query.empty()) return 0; // Return if query is empty

	// Setup regex matches
	std::wsmatch wsm;

	// Clear the results map
	results.clear();

	// Count lines
	std::wcout << L"Searching: " << query << std::endl;
	int checked = 0;
	for (const auto& word : skimmed_words) {
		checked++;
		std::regex_search(word, wsm, std::wregex(query));
		if (wsm.ready() && wsm.empty()) continue;

		// Push back the result only if it contains all the yellow letters
		bool skip = false;
		for (const auto& t : yellow_letters)
			if (word.find(t.first) == std::wstring::npos) {
				skip = true; break;
			}
		if (!skip) results.insert(word);
	}
	std::cout << "Checked " << checked << " lines." << std::endl;

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
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			if (good_letters.count(std::wstring(1, word.at(j))) == 0 || (*good_letters.find(std::wstring(1, word.at(j)))).second != ((*good_letters.find(std::wstring(1, word.at(j)))).second | i))
				if (yellow_letters.count(std::wstring(1, word.at(j))) == 0 || (*yellow_letters.find(std::wstring(1, word.at(j)))).second != ((*yellow_letters.find(std::wstring(1, word.at(j)))).second | i))
					freq[word.at(j)]++;

	std::wofstream wof(L"output/frequencies.txt");
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof.is_open()) for (const auto &b : freq) wof << b.first << L" " << b.second << std::endl;
	wof.close();

	good_words.clear();

	// Count lines
	for (const auto& word : skimmed_words) {
		// Check if any of the used letters is in the string
		bool skip = false;
		for (wchar_t l : bad_letters)
			if (word.find(l) != std::wstring::npos) { skip = true; break; }
		for (const auto& l : bad_pos_letters)
			if ((int)pow(2, word.find(l.first)) == ((int)pow(2, word.find(l.first)) | l.second)) {skip = true; break;}
		if (skip) continue;


		// Calculate score
		double score = 0;
		std::wstring had = L"";

		for (wchar_t l : word) {
			if (good_letters.count(std::wstring(1, l)) || had.find(l) != std::wstring::npos) continue;
			// Check for duplicate letters
			if (size_t count = std::count(word.begin(), word.end(), l) > 0) {
				score -= count * 10;
			}
			score += freq[l];
			had += l;
		}

		// Push back the result
		good_words[word] = score;

		//matches++;
	}

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

	return good_words.size() > 0 ? (*good_words.begin()).first : L"No good word found";
}

std::wstring jubiman::Regex::find_best_word_last() {
	std::cout << "Finding best word..." << std::endl;

	// Calculate frequencies of unfound letters
	std::map<wchar_t, int> freq;
	for (std::wstring word : results)
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			if (good_letters.count(std::wstring(1, word.at(j))) == 0 || (*good_letters.find(std::wstring(1, word.at(j)))).second != ((*good_letters.find(std::wstring(1, word.at(j)))).second | i))
				if (yellow_letters.count(std::wstring(1, word.at(j))) == 0 || (*yellow_letters.find(std::wstring(1, word.at(j)))).second != ((*yellow_letters.find(std::wstring(1, word.at(j)))).second | i))
					freq[word.at(j)]++;

	std::wofstream wof(L"output/frequencies.txt");
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof.is_open()) for (const auto& b : freq) wof << b.first << L" " << b.second << std::endl;
	wof.close();

	good_words.clear();

	// Count lines
	for (const auto& word : words) {
		// Check if any of the used letters is in the string
		bool skip = false;
		for (const auto& l : bad_letters) {
			if (word.find(l) != std::wstring::npos) { skip = true; break; }
		}
		for (const auto& l : good_letters) {
			if (word.find(l.first) != std::wstring::npos) { skip = true; break; }
		}
		if (skip) continue;


		// Calculate score
		double score = 0;
		std::wstring had = L"";

		for (const auto& l : word) {
			if (good_letters.count(std::wstring(1, l)) || had.find(l) != std::wstring::npos) continue;
			// Check for duplicate letters
			if (size_t count = std::count(word.begin(), word.end(), l) > 0) {
				score -= count * 10;
			}
			score += freq[l];
			had += l;
		}

		// Push back the result
		good_words[word] = score;
	}

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

	return good_words.size() > 0 ? (*good_words.begin()).first : L"No good word found";
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
		++good_letters_size;
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
		bad_pos_letters[std::wstring(1, strtup.at(1))] = yellow_letters[std::wstring(1, strtup.at(1))] | (int)pow(2, _wtoi(strtup.substr(4, 1).c_str()));
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

	// Make not or char letter regex
	for (auto const& x : yellow_letters)
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			tmp.at(j) == L'.' ? ((x.second != (i | x.second)) ? ((arr[j].find(x.first) == std::wstring::npos) ? arr[j] += x.first : L"") : L"") : L"";
		
	for (auto const& x : bad_pos_letters)
		for (unsigned char i = 1, j = 0; j < 5; i <<= 1, ++j)
			if (notArr[j].find(x.first) == std::wstring::npos)
				notArr[j] += (x.second == (i | x.second)) ? x.first : L"";

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

	search();
	return check_yellow_letters();
}

int jubiman::Regex::check_yellow_letters() {
	for (const auto& t : yellow_letters)
		for (const auto& i : results)
			if (i.find(t.first) == std::wstring::npos)
				results.erase(i);

	for (const auto& l : bad_pos_letters)
		for (const auto& r : results)
			if ((int)pow(2, r.find(l.first)) == ((int)pow(2, r.find(l.first)) | l.second))
				results.erase(r);
	return results.size();
}

void jubiman::Regex::skim_words() {
	skimmed_words = results;
}