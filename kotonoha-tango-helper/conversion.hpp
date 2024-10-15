//
// Created by jubiman on 15-10-24.
//

#ifndef CONVERSION_H
#define CONVERSION_H
#include <string>
#include <cstdlib>
#include <stdexcept>

namespace jubiman {
	static std::wstring to_wstring(const std::string& str) {
		const size_t len = std::mbstowcs(nullptr, str.c_str(), 0);
		if (len == static_cast<size_t>(-1)) {
			throw std::runtime_error("Conversion error");
		}

		std::wstring wstr(len, L'\0');
		std::mbstowcs(&wstr[0], str.c_str(), len);
		return wstr;
	}

	static std::string to_string(const std::wstring& wstr) {
		const size_t len = std::wcstombs(nullptr, wstr.c_str(), 0);
		if (len == static_cast<size_t>(-1)) {
			throw std::runtime_error("Conversion error");
		}

		std::string str(len, '\0');
		std::wcstombs(&str[0], wstr.c_str(), len);
		return str;
	}
} // jubiman
#endif //CONVERSION_H
