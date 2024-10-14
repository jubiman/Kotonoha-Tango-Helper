//
// Created by jubiman on 3/7/24.
//

#ifndef KOTONOHA_TANGO_HELPER_COLORED_TEXT_H
#define KOTONOHA_TANGO_HELPER_COLORED_TEXT_H

#include <cassert>
#include <codecvt>
#include <locale>

#include "ftxui/dom/node.hpp"  // for Node
#include "ftxui/screen/screen.hpp"  // for Pixel, Screen
#include "ftxui/screen/string.hpp"  // for string_width, Utf8ToGlyphs
#include "ftxui/dom/elements.hpp"  // for Element
#include "constants.h"

namespace ftxui {
	class ColoredChar {
	public:
		explicit ColoredChar(const std::wstring& character, Color fg, Color bg);
		ColoredChar(ColoredChar&& other) noexcept;
		[[nodiscard]] const std::string &getCharacter() const { return character_; }
		[[nodiscard]] std::wstring getWideCharacter() const;
		[[nodiscard]] const Color &getFgColor() const { return fg_color_; }
		[[nodiscard]] const Color &getBgColor() const { return bg_color_; }
		void erase() { character_ = filler_char; }
		void setBgColor(const Color color) { if(!is_locked_) bg_color_ = color; }
		void setFgColor(const Color color) { if(!is_locked_) fg_color_ = color; }
		void lock() { is_locked_ = true; }
		void unlock() { is_locked_ = false; }
		bool operator!= (const ColoredChar& other) const {
			return character_ != other.character_;
		}
		bool operator!= (const std::string& other) const {
			return character_ != other;
		}
		bool operator== (const ColoredChar& other) const {
			return character_ == other.character_;
		}
		bool operator== (const std::string& other) const {
			return character_ == other;
		}
		ColoredChar& operator= (const std::string& str) {
			assert(str.size() == 1);
			character_ = str;
			return *this;
		}
		ColoredChar& operator= (const ColoredChar& other) {
			character_ = other.character_;
			fg_color_ = other.fg_color_;
			bg_color_ = other.bg_color_;
			return *this;
		}
	private:
		std::string character_;
		Color fg_color_;
		Color bg_color_;
		bool is_locked_ = false;
		mutable std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	};

	class ColoredText final : public Node {
	public:
		explicit ColoredText(const std::string& str, Color fg = Color::Default, Color bg = Color::Default);
		explicit ColoredText(const std::wstring& text, Color fg = Color::Default, Color bg = Color::Default);
		explicit ColoredText(const char* str, Color fg = Color::Default, Color bg = Color::Default);
		explicit ColoredText(std::vector<ColoredChar> vec);
		void ComputeRequirement() override;
		void Render(Screen& screen) override;
		void insertText(const std::string& str);
		void addText(const std::string& str);
		void handleBackspace();
		void handleDelete();
		void moveCursorLeft();
		void moveCursorRight();
		void resetCursor();
		void colorMode();
		void inputMode();
		void rotateColor(bool direction);
		void focus() { is_focused_ = true; }
		void unfocus() { is_focused_ = false; }
		void reset();
		// overload the iteration begin() and end() functions
		auto begin() { return characters_.begin(); }
		auto end() { return characters_.end(); }
		[[nodiscard]] auto begin() const { return characters_.begin(); }
		[[nodiscard]] auto end() const { return characters_.end(); }
		[[nodiscard]] bool isFocused() const { return is_focused_; }
		[[nodiscard]] size_t length() const { size_t i = 0; for (auto& c : characters_) { if (c != filler_char) ++i; } return i; }

		[[nodiscard]] std::string getCharacters() const;

		void lockColor(size_t pos, Color color);

		[[nodiscard]] std::wstring getWideCharacters() const;

	private:
		std::vector<ColoredChar> characters_;
		uint8_t cursor_x_ = 0;
		uint8_t max_cursor_x_ = 0;
		bool is_focused_ = false;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	};

	Element colored_text(const std::string& text, Color fg = Color::Default, Color bg = Color::Default);
	Element colored_text(const std::string& text, int, Color fg = Color::Default, Color bg = Color::Default);
}

#endif //KOTONOHA_TANGO_HELPER_COLORED_TEXT_H
