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

std::vector<size_t> findAll(std::wstring data, std::wstring toSearch)
{
	std::vector<size_t> vec;
	// Get the first occurrence
	size_t pos = data.find(toSearch);
	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Add position to the vector
		vec.push_back(pos);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + toSearch.length());
	}
	return vec;
}

/**
 * Searches all words. That's it.
 *
 * @returns The amount of matches.
 */
int jubiman::Regex::search() {
	// Clear the results map
	results.clear();

	// Count lines
	std::cout << "Searching..." << std::endl;
	int checked = 0;
	for (const auto& word : skimmed_words) {
		checked++;

		// Skip if it contains a used letter
		for (const auto& bl : bad_letters)
			if (word.find(bl) != std::wstring::npos) goto next;

		// Skip if it does not have the good letters in the right position
		for (const auto& gl : good_letters) {
			// Get the positions of the good letter
			unsigned char num = gl.second, count = 0;
			std::vector<unsigned char> vec;
			while (num > 0) {
				if ((num & 1) == 1) {
					vec.push_back(count);
				}
				count++;
				num >>= 1;	// bitwise rightshift 
			}
			for (auto it = vec.begin(); it != vec.end(); ++it)
				if (std::wstring(1, word.at(*it)) != gl.first) goto next;
		}

		// Skip if it has a gray letter in known bad position
		for (const auto& bl : bad_pos_letters) {
			// Get the positions of the bad letter
			unsigned char num = bl.second, count = 0;
			std::vector<unsigned char> vec;
			while (num > 0) {
				if ((num & 1) == 1) {
					vec.push_back(count);
				}
				count++;
				num >>= 1;	// bitwise rightshift 
			}
			for (auto it = vec.begin(); it != vec.end(); ++it)
				if (std::wstring(1, word.at(*it)) != bl.first) goto next;
		}

		// Skip if it has a yellow letter in a known position
		for (const auto& yl : yellow_letters) {
			if (word.find(yl.first) == std::wstring::npos) goto next;
			std::vector<size_t> vec = findAll(word, yl.first);
			for (std::vector<size_t>::iterator it = vec.begin(); it != vec.end(); ++it)
				if (yl.second & 1 << *it) goto next;
		}

		results.insert(word);
	next:
		continue;
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
	for (const auto& word : results) {
		// Check if any of the used letters is in the string
		bool skip = false;
		for (wchar_t l : bad_letters)
			if (word.find(l) != std::wstring::npos) { skip = true; break; }
		for (const auto& l : bad_pos_letters)
			if (1 << word.find(l.first) & l.second) { skip = true; break; }
		for (const auto& l : yellow_letters) 
			if (word.find(l.first) == std::wstring::npos) { skip = true; break; }
		for (const auto& l : good_letters)
			if (word.find(l.first) == std::wstring::npos || ((1 << word.find(l.first)) | l.second) != l.second) { skip = true; break; }
		if (skip) continue;


		// Calculate score
		int score = 0;
		std::wstring had = L"";

		for (wchar_t l : word) {
			if (good_letters.count(std::wstring(1, l)) || had.find(l) != std::wstring::npos) continue;
			score += freq[l] * 4;
			// Check for duplicate letters
			if (size_t count = std::count(word.begin(), word.end(), l) > 0) {
				score -= count * 25;
			}
			had += l;
		}

		// Push back the result
		good_words[word] = score;

		//matches++;
	}

	// Sort words by score (ascending) (bypass hahahahaha)
	std::vector<std::pair<std::wstring, int>> v(good_words.begin(), good_words.end());
	std::sort(v.begin(), v.end(), [](const std::pair<std::wstring, int>& a, const std::pair<std::wstring, int>& b) -> bool {
		return (a.second > b.second);
	});

	std::wofstream wof2(L"output/good words.txt");
	wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof2.is_open()) for (const auto& b : v) wof2 << b.first << L" " << b.second << std::endl;
	wof2.close();

	return v.size() > 0 ? (*v.begin()).first : L"No good word found";
}

std::wstring jubiman::Regex::find_best_word_last() {
	std::cout << "Finding best word..." << std::endl;

	unsigned char pos = 0b11100000;
	unsigned char j = 0;
	for (const auto& l : good_letters)
		pos |= l.second;
	pos = ~pos;
	for (unsigned char i = 1; i < 33; i <<= 1)
		if (pos == (pos | i)) {
			while (i > 0)
				++j, i >>= 1;
			break;
		}


	std::unordered_set<wchar_t> last_letters;
	for (const auto& r : results)
		last_letters.insert(r.at(j-1));

	good_words.clear();

	// Count lines
	for (const auto& word : words) {
		// Check if any of the used letters is in the string
		bool skip = false;
		for (const auto& l : bad_letters)
			if (word.find(l) != std::wstring::npos) { skip = true; break; }
		for (const auto& l : good_letters)
			if (word.find(l.first) != std::wstring::npos) { skip = true; break; }
		if (skip) continue;


		// Calculate score
		int score = 0;
		std::wstring had = L"";

		for (const auto& l : word) {
			if (had.find(l) != std::wstring::npos) continue;
			//if (good_letters.count(std::wstring(1, l)) || had.find(l) != std::wstring::npos) continue;

			had += l;

			/*for (const auto& r : results)
				if (r.find(l) == std::wstring::npos)
					skip = true;
			if (skip) continue;*/

			// Check for duplicate letters
			//if (size_t count = std::count(word.begin(), word.end(), l) > 0) break;
			
			if (last_letters.find(l) != last_letters.end())
				++score;
		}

		// Push back the result
		good_words[word] = score;
	}

	// Sort words by score (ascending) (bypass hahahahaha)
	std::vector<std::pair<std::wstring, int>> v(good_words.begin(), good_words.end());
	std::sort(v.begin(), v.end(), [](const std::pair<std::wstring, int>& a, const std::pair<std::wstring, int>& b) -> bool {
		return (a.second > b.second);
	});

	std::wofstream wof2(L"output/good words.txt");
	wof2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	if (wof2.is_open()) for (const auto& b : v) wof2 << b.first << L" " << b.second << std::endl;
	wof2.close();

	return v.size() > 0 ? (*v.begin()).first : L"No good word found";
}

// Add bad letters
void jubiman::Regex::add_bad_letters(std::wstring bl) {
	bad_letters += bl;
	std::set<char> chars;

	bad_letters.erase(
		std::remove_if(
			bad_letters.begin(),
			bad_letters.end(),
			[&chars](char i) {
				// If encountered character, remove this one.
				if (chars.count(i)) { return true; }

				// Otherwise, mark this character encountered and don't remove.
				chars.insert(i);
				return false;
			}
		),
		bad_letters.end()
	);
}

void jubiman::Regex::add_duplicate_gray_letters(std::wstring dgl) {
	if (dgl.empty()) return;
	std::vector<std::wstring> strtuples;
	size_t pos = 5;
	while ((pos = dgl.find(L") (")) != std::wstring::npos) {
		strtuples.push_back(dgl.substr(0, pos + 1));
		dgl.erase(0, pos + 2);
	}
	strtuples.push_back(dgl.substr(0, pos));
	for (std::wstring strtup : strtuples) {
		bad_pos_letters[std::wstring(1, strtup.at(1))] |= (1 << _wtoi(strtup.substr(4, 1).c_str()));
	}
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
		good_letters[std::wstring(1, strtup.at(1))] = good_letters[std::wstring(1, strtup.at(1))] | (1 << _wtoi(strtup.substr(4, 1).c_str()));
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
		bad_pos_letters[std::wstring(1, strtup.at(1))] = yellow_letters[std::wstring(1, strtup.at(1))] | (1 << _wtoi(strtup.substr(4, 1).c_str()));
		yellow_letters[std::wstring(1, strtup.at(1))] = yellow_letters[std::wstring(1, strtup.at(1))] | (1 << _wtoi(strtup.substr(4, 1).c_str()));
	}
}

int jubiman::Regex::check_yellow_letters() {
	for (const auto& t : yellow_letters)
		for (auto it = results.begin(); it != results.end();)
			if ((*it).find(t.first) == std::wstring::npos)
				it = results.erase(it);
			else ++it;

	for (const auto& l : bad_pos_letters)
		for (auto it = results.begin(); it != results.end(); )
			if ((1 << (*it).find(l.first)) == ((1 << ((*it).find(l.first))) | l.second))
				it = results.erase(it);
			else ++it;
	return results.size();
}

void jubiman::Regex::skim_words() {
	skimmed_words = results;
}