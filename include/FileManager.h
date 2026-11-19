#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Document.h"
#include "MyString.h"

// Performs file I/O for Document without owning the document itself.
// File format: byte-oriented text, every line is terminated by '\n'
// (POSIX style), so a file round-trips exactly through open + save.
// An empty document is saved as an empty (0 byte) file.
class FileManager
{
public:

    // Trims surrounding whitespace and one pair of surrounding double quotes
    // (as produced by "Copy as path" on Windows).
    static MyString normalizeFilename(const MyString& filename);

    // False for empty names, control characters, the characters < > " | ? *,
    // "." / "..", and names that end with a path separator.
    static bool isValidFilename(const MyString& filename);

    // True if a file or directory with this name exists.
    static bool fileExists(const MyString& filename);

    // Loads into a temporary document and commits only after a successful read.
    // Fails (document untouched) for missing/unreadable files, directories,
    // and binary files containing NUL bytes.
    static bool openFile(const MyString& filename, Document& document);

    // Writes the document to "<filename>.tmp" first and then replaces the
    // target, so a failed save never destroys the previous contents.
    static bool saveFile(const MyString& filename, const Document& document);

    // Deletes a file from disk. Returns false if the name is invalid, the file
    // does not exist, it is a directory, or the OS refuses to delete it.
    static bool deleteFile(const MyString& filename);
};

#endif // FILEMANAGER_H