cmake_minimum_required(VERSION 3.16)
option(VITAREADER_HOST_TESTS "Build portable core tests instead of the Vita application" OFF)
set(VREED_MUPDF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.deps/mupdf" CACHE PATH "Pinned MuPDF source directory")
if(VITAREADER_HOST_TESTS)
  project(VitaReaderCoreTests LANGUAGES C CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  option(VITAREADER_EPUB_TESTS "Include tests requiring libarchive and Expat" ON)
  option(VREED_PDF_TESTS "Test the actual MuPDF renderer" OFF)
  enable_testing()
  add_executable(orientation_test tests/orientation_test.cpp src/reader/settings.cpp src/rendering/curl.cpp src/storage/settings.cpp)
  target_include_directories(orientation_test PRIVATE src)
  add_test(NAME orientation COMMAND orientation_test "${CMAKE_CURRENT_BINARY_DIR}/orientation-settings.cfg")
  add_executable(pdf_date_test tests/pdf_date_test.cpp src/platform/mupdf_compat.c)
  target_include_directories(pdf_date_test PRIVATE src)
  add_test(NAME pdf_date COMMAND pdf_date_test)
  add_executable(layout_test tests/layout_test.cpp src/reader/layout.cpp src/reader/settings.cpp src/reader/transition.cpp src/rendering/page_style.cpp)
  target_include_directories(layout_test PRIVATE src)
  add_test(NAME layout COMMAND layout_test)
  add_executable(session_test tests/session_test.cpp tests/fixture_epub.cpp src/reader/session.cpp src/reader/layout.cpp)
  target_include_directories(session_test PRIVATE src)
  add_test(NAME session COMMAND session_test)
  add_executable(interaction_test tests/interaction_test.cpp tests/fixture_epub.cpp src/reader/session.cpp src/reader/layout.cpp
    src/reader/transition.cpp src/reader/turn_transaction.cpp src/reader/settings.cpp src/input/touch.cpp
    src/rendering/curl.cpp src/storage/settings.cpp src/storage/position.cpp src/audio/events.cpp)
  target_include_directories(interaction_test PRIVATE src)
  add_test(NAME interaction COMMAND interaction_test "${CMAKE_CURRENT_BINARY_DIR}/settings-test.cfg")
  set(core_test_targets layout_test session_test interaction_test)
  if(VITAREADER_EPUB_TESTS)
    find_package(LibArchive REQUIRED)
    find_package(EXPAT REQUIRED)
    add_executable(epub_test tests/epub_test.cpp src/epub/epub.cpp src/reader/layout.cpp src/reader/session.cpp)
    target_include_directories(epub_test PRIVATE src)
    target_link_libraries(epub_test PRIVATE LibArchive::LibArchive EXPAT::EXPAT)
    if(VREED_PDF_TESTS)
      target_sources(epub_test PRIVATE src/pdf/pdf.cpp)
      target_include_directories(epub_test PRIVATE "${VREED_MUPDF_ROOT}/include")
      target_compile_definitions(epub_test PRIVATE VITAREADER_PDF=1)
      target_link_libraries(epub_test PRIVATE "${VREED_MUPDF_ROOT}/build/host/libmupdf.a" "${VREED_MUPDF_ROOT}/build/host/libmupdf-third.a" m)
      add_executable(pdf_test tests/pdf_test.cpp src/pdf/pdf.cpp src/epub/epub.cpp src/reader/layout.cpp src/reader/session.cpp)
      target_include_directories(pdf_test PRIVATE src "${VREED_MUPDF_ROOT}/include")
      target_compile_definitions(pdf_test PRIVATE VITAREADER_PDF=1)
      target_link_libraries(pdf_test PRIVATE LibArchive::LibArchive EXPAT::EXPAT "${VREED_MUPDF_ROOT}/build/host/libmupdf.a" "${VREED_MUPDF_ROOT}/build/host/libmupdf-third.a" m)
      add_test(NAME pdf COMMAND pdf_test "${CMAKE_CURRENT_BINARY_DIR}/pdf-fixture.pdf")
      list(APPEND core_test_targets pdf_test)
    endif()
    add_test(NAME epub COMMAND epub_test "${CMAKE_CURRENT_SOURCE_DIR}/assets/VitaReader-test.epub")
    list(APPEND core_test_targets epub_test)
  endif()
  foreach(test_target IN LISTS core_test_targets)
    if(MSVC)
      target_compile_options(${test_target} PRIVATE /UNDEBUG)
    else()
      target_compile_options(${test_target} PRIVATE -UNDEBUG)
    endif()
  endforeach()
  add_executable(reading_preview tools/preview_reading.cpp src/reader/layout.cpp src/reader/settings.cpp src/rendering/page_style.cpp)
  target_include_directories(reading_preview PRIVATE src)
  return()
endif()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(NOT DEFINED ENV{VITASDK})
    message(FATAL_ERROR "Set VITASDK to a VitaSDK installation; see docs/BUILD.md")
  endif()
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Vita toolchain")
endif()
project(VitaReader LANGUAGES C CXX)
include("$ENV{VITASDK}/share/vita.cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(VitaReader src/main.cpp src/app/app.cpp src/platform/vita.cpp
  src/rendering/renderer.cpp src/storage/library.cpp src/storage/position.cpp
  src/epub/epub.cpp src/reader/layout.cpp src/reader/session.cpp src/reader/settings.cpp src/rendering/page_style.cpp)
target_sources(VitaReader PRIVATE src/reader/transition.cpp src/reader/turn_transaction.cpp src/rendering/curl.cpp
  src/input/touch.cpp src/storage/settings.cpp src/audio/events.cpp src/platform/vita_audio.cpp)
target_include_directories(VitaReader PRIVATE src)
target_sources(VitaReader PRIVATE third_party/libvita2d-font/vita2d_font.c third_party/libvita2d-font/texture_atlas.c)
target_include_directories(VitaReader PRIVATE third_party/libvita2d-font "$ENV{VITASDK}/arm-vita-eabi/include/freetype2")
target_compile_definitions(VitaReader PRIVATE VITAREADER_VITA=1)
target_compile_definitions(VitaReader PRIVATE VITAREADER_PDF=1)
target_sources(VitaReader PRIVATE src/pdf/pdf.cpp src/platform/mupdf_compat.c)
target_include_directories(VitaReader PRIVATE "${VREED_MUPDF_ROOT}/include")
target_link_libraries(VitaReader "${VREED_MUPDF_ROOT}/build/vita/libmupdf.a" "${VREED_MUPDF_ROOT}/build/vita/libmupdf-third.a")
target_compile_options(VitaReader PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(VitaReader vita2d freetype png jpeg archive expat bz2 zstd z m
  SceDisplay_stub SceGxm_stub SceSysmodule_stub SceCtrl_stub ScePgf_stub
  ScePvf_stub SceCommonDialog_stub SceAppMgr_stub SceLibKernel_stub SceTouch_stub SceAudio_stub SceIofilemgr_stub)
include(config/identity.cmake)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio/library-stereo.pcm")
  find_program(FFMPEG_EXECUTABLE ffmpeg REQUIRED)
  execute_process(COMMAND "${FFMPEG_EXECUTABLE}" -nostdin -v error -i "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio/library.mp3"
    -ac 2 -ar 48000 -f s16le "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio/library-stereo.pcm" RESULT_VARIABLE music_result)
  if(NOT music_result EQUAL 0)
    message(FATAL_ERROR "Library music conversion failed")
  endif()
endif()
configure_file(config/identity.hpp.in generated/app/identity.hpp @ONLY)
target_include_directories(VitaReader PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
vita_create_self(eboot.bin VitaReader UNSAFE)
vita_create_vpk(${VITAREADER_NAME}.vpk ${VITAREADER_TITLE_ID} eboot.bin
  VERSION ${VITAREADER_VERSION} NAME ${VITAREADER_NAME}
  FILE assets/branding/icon0.png sce_sys/icon0.png
  FILE assets/branding/bg.png sce_sys/livearea/contents/bg.png
  FILE assets/branding/startup.png sce_sys/livearea/contents/startup.png
  FILE assets/branding/template.xml sce_sys/livearea/contents/template.xml
  FILE assets/branding/logo.png assets/branding/logo.png
  FILE assets/branding/library-logo.png assets/branding/library-logo.png
  FILE LICENSE assets/licenses/AGPL-3.0.txt
  FILE THIRD_PARTY_NOTICES.md assets/licenses/THIRD_PARTY_NOTICES.md
  FILE third_party/libvita2d-font/LICENSE assets/licenses/libvita2d-MIT.txt
  FILE assets/branding/mascot-poses.png assets/branding/mascot-poses.png
  FILE assets/fonts/DejaVuSerif.ttf assets/fonts/DejaVuSerif.ttf
  FILE assets/fonts/DejaVuSans.ttf assets/fonts/DejaVuSans.ttf
  FILE assets/fonts/DejaVuSansMono.ttf assets/fonts/DejaVuSansMono.ttf
  FILE assets/fonts/LICENSE assets/fonts/LICENSE
  FILE assets/audio/library-stereo.pcm assets/audio/library-stereo.pcm)
