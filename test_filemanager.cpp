#include "Document.h"
#include "FileManager.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
    int failures = 0;

    void check(bool condition, const std::string& testName)
    {
        if (condition)
        {
            std::cout << "[PASS] " << testName << '\n';
        }
        else
        {
            std::cout << "[FAIL] " << testName << '\n';
            ++failures;
        }
    }

    bool writeBytes(const std::string& filename, const std::string& contents)
    {
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file)
            return false;
        file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        return static_cast<bool>(file);
    }

    bool readBytes(const std::string& filename, std::string& contents)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
            return false;
        contents.assign(std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>());
        return !file.bad();
    }

    bool lineEquals(const Document& document, int lineNumber,
                    const std::string& expected)
    {
        std::string actual;
        return document.getLine(lineNumber, actual) && actual == expected;
    }
}

int main()
{
    const std::string inputPath = "filemanager_test_input.txt";
    const std::string outputPath = "filemanager_test_output.txt";
    const std::string missingPath = "filemanager_file_that_does_not_exist.txt";

    // Test 1: Open a normal multi-line file.
    writeBytes(inputPath, "first line\nsecond line\nthird line");
    Document document;
    document.appendLine("old document");
    const bool opened = FileManager::openFile(inputPath, document);
    check(opened && document.getLineCount() == 3 &&
          lineEquals(document, 1, "first line") &&
          lineEquals(document, 2, "second line") &&
          lineEquals(document, 3, "third line"),
          "Open a normal three-line file");

    // Test 2: A missing file must not modify the current document.
    const bool missingOpened = FileManager::openFile(missingPath, document);
    check(!missingOpened && document.getLineCount() == 3 &&
          lineEquals(document, 1, "first line") &&
          lineEquals(document, 3, "third line"),
          "Failed open preserves the existing document");

    // Test 3: Empty file should load as a zero-line document.
    writeBytes(inputPath, "");
    const bool emptyOpened = FileManager::openFile(inputPath, document);
    check(emptyOpened && document.getLineCount() == 0 && document.isEmpty(),
          "Open an empty file as a zero-line document");

    // Test 4: Blank lines inside a file must be retained.
    writeBytes(inputPath, "alpha\n\nbeta");
    const bool blanksOpened = FileManager::openFile(inputPath, document);
    check(blanksOpened && document.getLineCount() == 3 &&
          lineEquals(document, 1, "alpha") &&
          lineEquals(document, 2, "") &&
          lineEquals(document, 3, "beta"),
          "Preserve an empty line between two non-empty lines");

    // Test 5: Saving must write line separators between lines, without an
    // additional newline after the final line.
    Document saveDocument;
    saveDocument.appendLine("red");
    saveDocument.appendLine("");
    saveDocument.appendLine("blue");
    const bool saved = FileManager::saveFile(outputPath, saveDocument);
    std::string savedBytes;
    const bool readBack = readBytes(outputPath, savedBytes);
    check(saved && readBack && savedBytes == "red\n\nblue",
          "Save exact contents with internal blank line and no final newline");

    // Test 6: Saving an empty document should create an empty file.
    Document emptyDocument;
    const bool emptySaved = FileManager::saveFile(outputPath, emptyDocument);
    std::string emptyBytes;
    const bool emptyRead = readBytes(outputPath, emptyBytes);
    check(emptySaved && emptyRead && emptyBytes.empty(),
          "Save an empty document as a zero-byte file");

    std::remove(inputPath.c_str());
    std::remove(outputPath.c_str());

    std::cout << "\nTest summary: " << (6 - failures) << "/6 passed.\n";
    return failures == 0 ? 0 : 1;
}
