#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include "Document.h"

class FileManager {
public:
    static bool openFile(
        const std::string& filename,
        Document& document
    );

    static bool saveFile(
        const std::string& filename,
        const Document& document
    );
};

#endif