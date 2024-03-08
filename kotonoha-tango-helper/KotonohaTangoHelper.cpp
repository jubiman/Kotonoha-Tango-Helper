// ことのはたんごヘルパー.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <regex>
#include <utility>
#include <cstring>

// include FTXUI
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "word_search.h"
#include "colored_text.h"
#include "constants.h"
#include "translation.h"

enum Mode {
	input,
	color_edit
};

void renderTUI();
void reset_board(std::vector<ftxui::Element>& input_components,
				 ftxui::ColoredText*& c_input_text,
				 Mode& mode,
				 int& current_input,
				 jubiman::WordSearch& search
				 );
bool handleInput(const ftxui::Event& event,
				 std::string& input_text,
				 ftxui::ColoredText*& c_input_text,
				 std::vector<ftxui::Element>& input_components,
				 int& current_input, Mode& mode,
				 bool& modal_open,
				 bool& settings_modal_open,
				 std::string& debug_output,
				 ftxui::ScreenInteractive& screen,
				 jubiman::WordSearch& search
				 );
bool handleColorEdit(const ftxui::Event& event,
					 std::string& input_text,
					 ftxui::ColoredText*& c_input_text,
					 std::vector<ftxui::Element>& input_components,
					 int& current_input, Mode& mode,
					 bool& modal_open,
					 jubiman::WordSearch& search
					 );
std::wregex hiragana_regex(L"[\u3041-\u3096]");
std::wregex katakana_regex(L"[\u30A0-\u30FF]");
std::wregex japanese_regex(L"[\u3041-\u3096\u30A0-\u30FF]");
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
jubiman::translation translation;


int main() {
    // jubiman::Regex reg;
    std::wstring inp, bl, dgl, gl, yl;
#ifdef _WIN32
    std::wcin.imbue(std::locale("Japanese"));
    std::wcout.imbue(std::locale("Japanese"));
#else
	std::wcin.imbue(std::locale("C.UTF-8"));
	std::wcout.imbue(std::locale("C.UTF-8"));
#endif

#ifdef _WIN32
    _wmkdir(L"./output"); // can be used on Windows
    _wmkdir(L"./cfg"); // can be used on Windows
#else
	// Create the output directory with normal permissions
	mkdir("./output", 0777);
	mkdir("./cfg", 0777);
#endif
    if (errno != EEXIST && errno != ENOENT) {
        std::cout << "Could not create directories. Path not valid, exiting... " << strerror(errno) << std::endl;
		exit(1);
    }
	renderTUI();
	return 0;
}

void renderTUI() {
	using namespace ftxui;
	auto screen = ScreenInteractive::Fullscreen();

	std::string debug_output;
	std::vector<Element> guesses = std::vector<Element>();
	std::vector<Element> input_components = std::vector<Element>();

	for (int i = 0; i < 10; i++) {
		std::string str;
		for (int j = 0; j < 5; j++) {
			str += filler_char;
		}
		Element input = colored_text(str);
		input_components.push_back(input);
		guesses.push_back(input | ftxui::size(WIDTH, EQUAL, i < 5 ? 17 : 10));
	}

	int current_input = 0;
	std::string input_text;
	// Get the ColorText object from the element, which is stored in:
	// (*guesses[current_input].get()).children_[0].get()
	auto* c_input_text = reinterpret_cast<ColoredText *>(input_components[current_input].get());
	c_input_text->focus();

	// Create the search object
	jubiman::WordSearch search(c_input_text);

	Mode mode = input;
	bool modal_open = false;
	bool settings_modal_open = false;
	Component text_input = Input(&input_text, {
			.on_change = [&] {
				// translate all the characters to katakana
				std::wstring wide_input = converter.from_bytes(input_text);
				std::wstring replaced = wide_input;
				for (std::wsregex_iterator i = std::wsregex_iterator(wide_input.begin(), wide_input.end(), hiragana_regex);
					 i != std::wsregex_iterator(); ++i) {
					const std::wsmatch& match = *i;
					std::wstring replacement = std::wstring(1, match.str(0)[0] + 0x60);
					replaced = std::regex_replace(replaced, std::wregex(match.str(0)), replacement);
				}
				// Tell the input text component to update
				c_input_text->insertText(converter.to_bytes(replaced));
				input_text = "";
			},
	}) | CatchEvent([&](const Event& event) {
		switch (mode) {
			case input:
				return handleInput(event, input_text, c_input_text, input_components, current_input, mode, modal_open, settings_modal_open, debug_output, screen, search);
			case color_edit:
				return handleColorEdit(event, input_text, c_input_text, input_components, current_input, mode, modal_open, search);
		}
		return false;
	});

	// MAIN RENDERER
	auto renderer = Renderer(text_input, [&] {
		return vbox({
							text(" ことのはたんごヘルパー ") | bold,
							hbox(guesses[0], guesses[5]),
							hbox(guesses[1], guesses[6]),
							hbox(guesses[2], guesses[7]),
							hbox(guesses[3], guesses[8]),
							hbox(guesses[4], guesses[9]),
							separator(),
							text(" " + translation.translate("words_left") + " " +
								 std::to_string(search.getWordsLeft())),
							text(" " + translation.translate("best_word") + " " + search.getBestWord()),
							text(" Debug info:  " + debug_output),
							// write the settings hint at the bottom
							filler(),
							text(translation.translate("open_settings_hint"))
							| color(Color::GrayDark)
							| align_right,
					})
					| border;
	});

	// RESTART MODAL
	bool restart_modal_open = false;
	auto restart_modal = Container::Vertical({
		Button("Confirm", [&] {
			restart_modal_open = false;
			reset_board(input_components, c_input_text, mode, current_input, search);
		}, {
				.transform = [](const EntryState& s) {
					auto element = text(s.label) | color(ftxui::Color::Green) | border;
					if (s.active) {
						element |= bold;
					}
					return element;
				},
		}),
		Button("Cancel", [&] {
			restart_modal_open = false;
		}, {
				.transform = [](const EntryState& s) {
					auto element = text(s.label) | color(ftxui::Color::Red) | border;
					if (s.active) {
						element |= bold;
					}
					return element;
				},
		}),
	});

	// CONFIRM MODAL
	auto confirm_modal = Container::Vertical({
		Button("Confirm", [&] {
			modal_open = false;
			mode = input;
			// TODO: first have to convert the colorings to the regex thingy. might want to rewrite that a bit as well

			search.update_colors(c_input_text);
			size_t matches = search.filter_words();
			search.calculate_best_word();
			debug_output = "Matches: " + std::to_string(matches);

			// move to the next input
			int next_input = ++current_input;
			if (next_input == 10 || matches <= 1) {
				restart_modal_open = true;
				return;
			}
			c_input_text->unfocus();
			c_input_text = reinterpret_cast<ColoredText *>(input_components[next_input].get());
			c_input_text->focus();
		}, {
				.transform = [](const EntryState& s) {
					auto element = text(s.label) | color(ftxui::Color::Green) | border;
					if (s.active) {
						element |= bold;
					}
					return element;
				},
		}),
		Button("Cancel", [&] {
			modal_open = false;
		}, {
				.transform = [](const EntryState& s) {
					auto element = text(s.label) | color(ftxui::Color::Red) | border;
					if (s.active) {
						element |= bold;
					}
					return element;
				},
		}),
	});

	// LANGUAGE MODAL
	bool language_modal_open = false;
	int selected_language = 0;
	auto languages = &translation.getLanguages();
	auto language_menu = Menu(languages, &selected_language, {
			.on_enter = [&] {
				if (selected_language == languages->size() - 1) {
					language_modal_open = false;
					settings_modal_open = true;
					return;
				}
				translation.setLanguageFromName(languages->at(selected_language));
			},
	});
	auto language_modal = Container::Vertical({
		language_menu
	});

	// SETTINGS MODAL
	int settings_selected_index = 0;
	auto settings_menu = Menu(&translation.getSettingsEntries(), &settings_selected_index, {
			.on_enter = [&] {
				switch (settings_selected_index) {
					case 0:
						language_modal_open = true;
						settings_modal_open = false;
						break;
					default: // back
						settings_modal_open = false;
						break;
				}
			},
	});

	confirm_modal |= Renderer([&](Element inner) {
		return vbox({
							text("Are you sure you want to submit?") | color(Color::Red) | center,
							separator(),
							std::move(inner),
					})
			   | size(WIDTH, GREATER_THAN, 15)
			   | size(HEIGHT, GREATER_THAN, 5)
			   | border
			   | center;
	});
	restart_modal |= Renderer([&](Element inner) {
		return vbox({
							text("Do you want to restart?") | center,
							separator(),
							std::move(inner),
		})
		| size(WIDTH, GREATER_THAN, 15)
		| size(HEIGHT, GREATER_THAN, 5)
		| border
		| center;
	});
	settings_menu |= Renderer([&](Element inner) {
		return vbox({
							text(translation.translate("settings")) | center,
							separator(),
							std::move(inner),
					})
			   | size(WIDTH, GREATER_THAN, 15)
			   | size(HEIGHT, GREATER_THAN, 5)
			   | border
			   | center;
	});
	language_modal |= Renderer([&](Element inner) {
		return vbox({
							text(translation.translate("language")) | center,
							separator(),
							std::move(inner),
					})
			   | size(WIDTH, GREATER_THAN, 15)
			   | size(HEIGHT, GREATER_THAN, 5)
			   | border
			   | center;
	});

	renderer |= Modal(confirm_modal, &modal_open);
	renderer |= Modal(restart_modal, &restart_modal_open);
	renderer |= Modal(settings_menu, &settings_modal_open);
	renderer |= Modal(language_modal, &language_modal_open);

	screen.Loop(renderer);
}

void reset_board(std::vector<ftxui::Element>& input_components,
				 ftxui::ColoredText*& c_input_text,
				 Mode& mode, int& current_input,
				 jubiman::WordSearch& search
				 ) {
	using namespace ftxui;
	c_input_text->unfocus();
	for (const auto& input : input_components) {
		auto* c_input = reinterpret_cast<ColoredText *>(input.get());
		c_input->reset();
	}
	c_input_text = reinterpret_cast<ColoredText *>(input_components[0].get());
	c_input_text->focus();
	mode = input;
	current_input = 0;

	// TODO: actually reset the search stuff
}

bool handleInput(const ftxui::Event& event,
				 std::string& input_text,
				 ftxui::ColoredText*& c_input_text,
				 std::vector<ftxui::Element>& input_components,
				 int& current_input,
				 Mode& mode,
				 bool& modal_open,
				 bool& settings_modal_open,
				 std::string& debug_output,
				 ftxui::ScreenInteractive& screen,
				 jubiman::WordSearch& search
				 ) {
	using namespace ftxui;
	if (event.is_character()) {
		if (c_input_text->length() >= 5) return true;
		// try to read a wide character from the input
		std::wstring wide_char = converter.from_bytes(event.character());

		// if input is not hiragana or katakana, ignore it (return true)
		if (!std::regex_match(wide_char, japanese_regex)) {
			return true;
		}
		return false;
	} else if (event == Event::Backspace) {
		// remove the last character from the input
		c_input_text->handleBackspace();
		return true;
	} else if (event == Event::Delete) {
		c_input_text->handleDelete();
	} else if (event == Event::ArrowLeft) {
		// move the cursor left
		c_input_text->moveCursorLeft();
		return true;
	} else if (event == Event::ArrowRight) {
		// move the cursor right
		c_input_text->moveCursorRight();
		return true;
	} else if (event == Event::Return) {
		if (c_input_text->length() >= 5) {
			// Color known letters and lock those colors
			search.lock_colors(c_input_text);

			mode = color_edit;
			c_input_text->colorMode();
		}
		return true;
	} else if (event.is_mouse()) {
		return false;
	} else if (event == Event::F2) {
		// Open the settings modal
		settings_modal_open = true;
		return true;
	} else if (event == Event::F5) {
		// reset the board
		reset_board(input_components, c_input_text, mode, current_input, search);
		return true;
	}
	return false;
}

bool handleColorEdit(const ftxui::Event& event,
					 std::string& input_text,
					 ftxui::ColoredText*& c_input_text,
					 std::vector<ftxui::Element>& input_components,
					 int& current_input,
					 Mode& mode,
					 bool& modal_open, jubiman::WordSearch& search) {
	using namespace ftxui;
	if (event == Event::Return) {
		// TODO: add an undo button? Snapshot last state or all states?
		modal_open = true;
		// TODO: wait for it's return value?
		return true;
	} else if (event == Event::ArrowLeft) {
		// move the cursor left
		c_input_text->moveCursorLeft();
		return true;
	} else if (event == Event::ArrowRight) {
		// move the cursor right
		c_input_text->moveCursorRight();
		return true;
	} else if (event.is_character()) {
		return true;
	} else if (event == Event::ArrowUp) {
		// rotate the color of the character at the cursor up
		c_input_text->rotateColor(true);
		return true;
	} else if (event == Event::ArrowDown) {
		// rotate the color of the character at the cursor down
		c_input_text->rotateColor(false);
		return true;
	} else if (event == Event::F5) {
		// go back to input mode
		mode = input;
		return true;
	}
	return false;
}
