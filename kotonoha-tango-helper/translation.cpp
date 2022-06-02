//
// Created by jubiman on 3/8/24.
//

#include "translation.h"

/**
 * Constructor
 */
jubiman::translation::translation() {
	std::wifstream langFile;
	std::basic_string<wchar_t> langName;
	std::wstring langKey;
	std::wstring langValue;
	std::wstring langFileName;
	std::wstring langFileContents;
	std::wstring langFileLine;
	std::map<std::wstring, std::wstring> langMap;

	std::locale::global(std::locale("C.UTF-8"));
	langFile.imbue(std::locale());

	for (const auto& entry : std::filesystem::directory_iterator("lang")) {
		langFileName = entry.path().wstring();
		langFile.open(std::string(langFileName.begin(), langFileName.end()));
		langFileContents = L"";
		while (std::getline(langFile, langFileLine)) {
			langFileContents += langFileLine + L"\n";
		}
		langFile.close();
		langName = langFileName.substr(5, langFileName.length() - 10);
		langMap = std::map<std::wstring, std::wstring>();
		langKey = L"";
		langValue = L"";
		for (wchar_t c : langFileContents) {
			if (c == L'=') {
				langKey = langValue;
				langValue = L"";
			} else if (c == L'\n') {
				langMap[langKey] = langValue;
				langKey = L"";
				langValue = L"";
			} else {
				langValue += c;
			}
		}
		languages[langName] = langMap;
	}
	currentLanguage = &languages[L"en"];

	// Populate settings entries
	populate();
}

/**
 * Translate a string
 * @param key
 * @return
 */
std::string jubiman::translation::translate(const std::string& key) {
	std::wstring wideKey = converter.from_bytes(key);
	// make sure the key exists
	if (currentLanguage->find(wideKey) == currentLanguage->end()) {
		return key;
	}
	std::wstring wideValue = currentLanguage->at(wideKey);
	return converter.to_bytes(wideValue);
}

/**
 * Translate a string
 * @param key
 * @return
 */
std::string jubiman::translation::translate(const std::wstring &key) {
	// make sure the key exists
	if (currentLanguage->find(key) == currentLanguage->end()) {
		return converter.to_bytes(key);
	}
	std::wstring wideValue = currentLanguage->at(key);
	return converter.to_bytes(wideValue);
}

/**
 * Translate language names
 */
void jubiman::translation::translate_language_names() {
	language_names[language_names.size() - 1] = translate(L"back");
}

/**
 * Set language
 * @param string
 */
void jubiman::translation::setLanguage(const std::string& string) {
	std::wstring wideString = converter.from_bytes(string);
	currentLanguage = &languages[wideString];
	retranslate();
}

/**
 * Set language from name
 * @param lang_name
 */
void jubiman::translation::setLanguageFromName(const std::string& lang_name) {
	std::wstring wideLangName = converter.from_bytes(lang_name);
	for (auto const& [key, val] : languages) {
		if (val.at(L"lang_name") == wideLangName) {
			currentLanguage = &languages[key];
			retranslate();
			return;
		}
	}
}

/**
 * Translate settings entries
 */
void jubiman::translation::translate_settings_entries() {
	// TODO: add more settings
	settings_entries = {
			translate(L"language"),
	};
	settings_entries.emplace_back(translate(L"back"));
}

/**
 * Retranslate all strings
 */
void jubiman::translation::retranslate() {
	translate_settings_entries();
	translate_language_names();
}

void jubiman::translation::populate() {
	translate_settings_entries();
	for (auto const& [key, val] : languages) {
		language_names.emplace_back(converter.to_bytes(val.at(L"lang_name")));
	}
	language_names.emplace_back(translate(L"back"));
}
