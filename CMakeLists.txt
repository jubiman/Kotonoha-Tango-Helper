cmake_minimum_required(VERSION 3.26)
project(Kotonoha_Tango_Helper)

# Include FTXUI
include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CONTENT_ROOT kotonoha-tango-helper)

include_directories(${CONTENT_ROOT})
include_directories(${ftxui_SOURCE_DIR}/include)

add_executable(Kotonoha_Tango_Helper
        ${CONTENT_ROOT}/KotonohaTangoHelper.cpp
        kotonoha-tango-helper/colored_text.cpp
        kotonoha-tango-helper/colored_text.h
        kotonoha-tango-helper/constants.h
        kotonoha-tango-helper/word_search.cpp
        kotonoha-tango-helper/word_search.h
        kotonoha-tango-helper/translation.cpp
        kotonoha-tango-helper/translation.h
)

target_link_libraries(Kotonoha_Tango_Helper PRIVATE ftxui::screen ftxui::dom ftxui::component)
