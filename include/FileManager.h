#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Document.h"
#include "MyString.h"

// Performs file I/O for Document without owning the document itself.
// File format: byte-oriented text, '\n' line separators, no required final newline.
class FileManager
{
public:
    // Loads into a temporary document and commits only after a successful read.
    static bool openFile(const MyString& filename, Document& document);

    // Writes the document using '\n' separators and no trailing newline.
    static bool saveFile(const MyString& filename, const Document& document);
};

#endif // FILEMANAGER_H
