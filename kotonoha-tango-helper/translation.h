//
// Created by jubiman on 3/8/24.
//

#ifndef KOTONOHA_TANGO_HELPER_TRANSLATION_H
#define KOTONOHA_TANGO_HELPER_TRANSLATION_H
#include <string>
#include <fstream>
#include <filesystem>
#include <map>
#include <vector>

namespace jubiman {
	class translation {
	public:
		translation();
		std::string translate(const std::string& key);
		std::string translate(const std::wstring& key);
		void setLanguage(const std::string& string);

		const std::vector<std::string>& getLanguages() { return language_names; }

		void setLanguageFromName(const std::string& lang_name);

		std::vector<std::string>& getSettingsEntries() { return settings_entries; }

	private:
		std::map<std::wstring, std::wstring>* currentLanguage;
		std::map<std::wstring, std::map<std::wstring, std::wstring>> languages;

		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

		std::vector<std::string> settings_entries;
		std::vector<std::string> language_names;

		void retranslate();
		void translate_settings_entries();
		void translate_language_names();

		void populate();
	};
} // namespace jubiman

#endif //KOTONOHA_TANGO_HELPER_TRANSLATION_H
