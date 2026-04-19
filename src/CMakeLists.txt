cmake_minimum_required(VERSION 3.8)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(VCPKG_TARGET_TRIPLET ${TRIPLET})

project(TechStone VERSION 1.0.0 LANGUAGES CXX)
if (EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    # Not using -DCMAKE_TOOLCHAIN_FILE becuase it apparently is not 100% cross-compilation proof
    # Source: (https://stackoverflow.com/questions/55496611/cmake-cannot-find-libraries-installed-with-vcpkg)
    include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CODE_LOC "./code")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

PKG_CHECK_MODULES(GTKMM gtkmm-4.0)
pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_search_module(LIBSIGCPP REQUIRED sigc++ sigc++-3.0)
pkg_check_modules(GTK4 REQUIRED gtk4)

####################################  

add_executable(TechStone 
"${CODE_LOC}/main.cpp"
"${CODE_LOC}/MainWindow/MainWindow.cpp"
"${CODE_LOC}/TextThingy/TextThingy.cpp"
"${CODE_LOC}/TextThingy/PageInterface.cpp"
"${CODE_LOC}/IndexedTextView/IndexedTextView.cpp"
"${CODE_LOC}/ScrollingPageWrapper/ScrollingPageWrapper.cpp"
"${CODE_LOC}/TopPanel/TopPanel.cpp"
"${CODE_LOC}/UndoQueue/UndoQueue.cpp"
"${CODE_LOC}/UndoQueue/UndoNode.cpp"
)

target_include_directories(TechStone PRIVATE 
"${CODE_LOC}/MainWindow"
"${CODE_LOC}/TextThingy"
"${CODE_LOC}/PageInterface"
"${CODE_LOC}/IndexedTextView"
"${CODE_LOC}/ScrollingPageWrapper"
"${CODE_LOC}/TopPanel"
"${CODE_LOC}/UndoQueue"
)

####################################

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TechStone)

target_include_directories(TechStone PRIVATE ${GTK4_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS} ${fmt_INCLUDE_DIRS} ${LIBSIGCPP_INCLUDE_DIRS})
target_link_directories(TechStone PRIVATE ${GTK4_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS} ${VCPKG_ROOT}/packages/gtk_x64-windows/lib)
target_link_libraries(TechStone PRIVATE ${GTKMM_LIBRARIES} fmt::fmt PkgConfig::GTKMM_VARS ${LIBSIGCPP_LINK_LIBRARIES})

add_definitions(${GTK4_CFLAGS_OTHER})
target_compile_options(TechStone PRIVATE ${GTKMM_CFLAGS_OTHER})

# Zahrňte podprojekty.
# add_subdirectory ("code")
