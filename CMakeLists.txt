cmake_minimum_required(VERSION 3.16)

project(TextEditor LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the executable
add_executable(TextEditor
    src/main.cpp
    src/TextEditor.cpp
    src/Menu.cpp
    src/Document.cpp
    src/UndoStack.cpp
    src/TextAlgorithms.cpp
    src/FileManager.cpp
)

# Tell the compiler where header files are located
target_include_directories(TextEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)