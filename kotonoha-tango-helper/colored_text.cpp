//
// Created by jubiman on 3/7/24.
//

#include "colored_text.h"

/**
 * Constructor for ColoredChar
 * @param character The character to use
 * @param fg The foreground color
 * @param bg The background color
 */
ftxui::ColoredChar::ColoredChar(const std::wstring& character, ftxui::Color fg, ftxui::Color bg) {
	// Convert the character to a string
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	character_ = converter.to_bytes(character);
	fg_color_ = fg;
	bg_color_ = bg;
}

/**
 * Move constructor for ColoredChar
 * @param other The ColoredChar to move
 */
ftxui::ColoredChar::ColoredChar(ftxui::ColoredChar&& other) noexcept {
	character_ = std::move(other.character_);
	fg_color_ = other.fg_color_;
	bg_color_ = other.bg_color_;
}

std::wstring ftxui::ColoredChar::getWideCharacter() const {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.from_bytes(character_);
}

/**
 * Constructor for ColoredText
 * @param str The string to use
 * @param fg The foreground color
 * @param bg The background color
 */
ftxui::ColoredText::ColoredText(const std::string& str, ftxui::Color fg, ftxui::Color bg) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::wstring wide_str = converter.from_bytes(str);
	for (wchar_t c : wide_str) {
		characters_.emplace_back(std::wstring(1, c), fg, bg);
	}
}

/**
 * Constructor for ColoredText
 * @param text The text to use
 * @param fg The foreground color
 * @param bg The background color
 */
ftxui::ColoredText::ColoredText(const std::wstring& text, ftxui::Color fg, ftxui::Color bg) {
	for (wchar_t c : text) {
		characters_.emplace_back(std::wstring(1, c), fg, bg);
	}
}

/**
 * Constructor for ColoredText
 * @param str The string to use
 * @param fg The foreground color
 * @param bg The background color
 */
ftxui::ColoredText::ColoredText(const char * str, ftxui::Color fg, ftxui::Color bg) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::wstring wide_str = converter.from_bytes(str);
	for (wchar_t c : wide_str) {
		characters_.emplace_back(std::wstring(1, c), fg, bg);
	}
}

/**
 * Constructor for ColoredText
 * @param vec The vector of ColoredChar to use
 */
ftxui::ColoredText::ColoredText(std::vector<ColoredChar> vec) {
	characters_ = std::move(vec);
}

/**
 * Compute the size of the text
 */
void ftxui::ColoredText::ComputeRequirement() {
	for (auto& c : characters_) {
		requirement_.min_x += string_width(c.getCharacter());
		requirement_.min_y = 1;
	}
	requirement_.min_y = 1;
}

/**
 * Render the text to the screen
 * @param screen The screen to render to
 */
void ftxui::ColoredText::Render(ftxui::Screen &screen) {
	int x = box_.x_min;
	const int y = box_.y_min;
	if (y > box_.y_max) {
		return;
	}
	int i = 0;
	for (auto& c : characters_) {
		if (x > box_.x_max) {
			return;
		}
		screen.PixelAt(x, y).character = c.getCharacter();
		screen.PixelAt(x, y).foreground_color = c.getFgColor();
		screen.PixelAt(x, y).background_color = c.getBgColor();
		screen.PixelAt(x, y).bold = true;
		// If the cursor is at the current character, or at the end of the line, invert the colors
		if (is_focused_ && cursor_x_ == i) {
			screen.PixelAt(x, y).inverted = true;
		}
		x += string_width(c.getCharacter());
		++i;
	}
	if (is_focused_ && cursor_x_ == characters_.size()) {
		screen.PixelAt(x, y).character = "　";
		screen.PixelAt(x, y).inverted = true;
	}
}

/**
 * Add a character to the end of the text
 * @param str The character to add
 */
void ftxui::ColoredText::addText(const std::string &str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::wstring wide_str = converter.from_bytes(str);
	for (auto& c : characters_) {
		if (c == filler_char) {
			c = str;
			++cursor_x_;
			++max_cursor_x_;
			break;
		}
	}
}

/**
 * Insert a character at the cursor position
 * @param str The character to insert
 */
void ftxui::ColoredText::insertText(const std::string &str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::wstring wide_str = converter.from_bytes(str);
	characters_.emplace(characters_.begin() + cursor_x_, wide_str, ftxui::Color::Default, ftxui::Color::Default);
	characters_.erase(characters_.end() - 1);
	++cursor_x_;
	++max_cursor_x_;
}

/**
 * Remove the character at the cursor
 */
void ftxui::ColoredText::handleBackspace() {
	if (cursor_x_ == 0) {
		return;
	}
	// Remove the character at the cursor and shift the rest of the characters to the left
	characters_.erase(characters_.begin() + cursor_x_ - 1);
	characters_.emplace_back(wfiller_char, ftxui::Color::Default, ftxui::Color::Default);
	--cursor_x_;
}

/**
 * Remove the character at the cursor + 1
 */
void ftxui::ColoredText::handleDelete() {
	if (cursor_x_ == max_cursor_x_) {
		return;
	}
	// Remove the character at the cursor + 1 and shift the rest of the characters to the left
	characters_.erase(characters_.begin() + cursor_x_);
	characters_.emplace_back(wfiller_char, ftxui::Color::Default, ftxui::Color::Default);
}

/**
 * Move the cursor to the left
 */
void ftxui::ColoredText::moveCursorLeft() {
	if (cursor_x_ == 0) {
		return;
	}
	--cursor_x_;
}

/**
 * Move the cursor to the right
 */
void ftxui::ColoredText::moveCursorRight() {
	if (cursor_x_ == max_cursor_x_) {
		return;
	}
	++cursor_x_;
}

/**
 * Reset the cursor to the beginning of the text
 */
void ftxui::ColoredText::resetCursor() {
	cursor_x_ = 0;
}

/**
 * Rotate the color of the character at the cursor
 * @param direction The direction to rotate the color
 */
void ftxui::ColoredText::rotateColor(bool direction) {
	if (characters_[cursor_x_].getBgColor() == ftxui::Color::Default) {
		characters_[cursor_x_].setBgColor(direction ? ftxui::Color::Green : ftxui::Color::Yellow);
		characters_[cursor_x_].setFgColor(ftxui::Color::White);
	} else if (characters_[cursor_x_].getBgColor() == ftxui::Color::Green) {
		if (direction) {
			characters_[cursor_x_].setBgColor(ftxui::Color::Yellow);
			characters_[cursor_x_].setFgColor(ftxui::Color::White);
		} else {
			characters_[cursor_x_].setBgColor(ftxui::Color::Default);
			characters_[cursor_x_].setFgColor(ftxui::Color::Default);
		}
	} else { // yellow
		if (direction) {
			characters_[cursor_x_].setBgColor(ftxui::Color::Default);
			characters_[cursor_x_].setFgColor(ftxui::Color::Default);
		} else {
			characters_[cursor_x_].setBgColor(ftxui::Color::Green);
			characters_[cursor_x_].setFgColor(ftxui::Color::White);
		}
	}
}

/**
 * Set the text to color mode
 */
void ftxui::ColoredText::colorMode() {
	max_cursor_x_ = 4;
	cursor_x_ = 0;
}

/**
 * Set the text to input mode
 */
void ftxui::ColoredText::inputMode() {
	max_cursor_x_ = 0;
	cursor_x_ = 0;
}

/**
 * Reset the text
 */
void ftxui::ColoredText::reset() {
	for (auto& c : characters_) {
		c = filler_char;
	}
	cursor_x_ = 0;
	max_cursor_x_ = 0;
}

std::string ftxui::ColoredText::getCharacters() {
	std::string result;
	for (auto& c : characters_) {
		result += c.getCharacter();
	}
	return result;
}

/**
 * Create a ColoredText from a string
 * @param text The string to use
 * @param fg The foreground color
 * @param bg The background color
 * @return The created ColoredText
 */
ftxui::Element ftxui::colored_text(const std::string& text, ftxui::Color fg, ftxui::Color bg) {
	return std::make_shared<ColoredText>(text, fg, bg);
}