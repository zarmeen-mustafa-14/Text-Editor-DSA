#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>

#include "Document.h"

// Performs file I/O for Document without owning the document itself.
// File format: byte-oriented text, '\n' line separators, no required final newline.
class FileManager
{
public:
    // Loads into a temporary document and commits only after a successful read.
    // Returns false for an empty filename, open/read failure, or allocation failure.
    static bool openFile(const std::string& filename, Document& document);

    // Writes the document using '\n' separators and no trailing newline.
    // An empty document is saved as a zero-byte file.
    static bool saveFile(const std::string& filename, const Document& document);
};

#endif // FILEMANAGER_H
