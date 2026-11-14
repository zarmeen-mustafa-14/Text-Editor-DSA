#include "FileManager.h"

#include <fstream>
#include <string>

bool FileManager::openFile(const std::string& filename, Document& document)
{
    std::ifstream input(filename);
    if (!input.is_open()) {
    	return false;
    }

    // Build the file in a temporary Document. The current document is not
    // changed unless the entire read succeeds.
    Document loadedDocument;
    std::string line;

    while (std::getline(input, line))
    {
        if (!loadedDocument.appendLine(line))
        {
            return false;
        }
    }

    // getline can stop because of an I/O error as well as because it reached
    // EOF. Only commit the temporary document when EOF was reached normally.
    if (input.bad() || (!input.eof() && input.fail()))
    {
        return false;
    }

    document.swap(loadedDocument);
    return true;
}

bool FileManager::saveFile(const std::string& filename, const Document& document)
{
    std::ofstream output(filename, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!output.is_open())
    {
        return false;
    }

    const int lineCount = document.getLineCount();
    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;
        if (!document.getLine(lineNumber, line))
        {
            output.close();
            return false;
        }

        output << line;
        // Put separators between lines, not after the final line. This avoids
        // creating an extra blank line when the file is opened again.
        if (lineNumber < lineCount)
        {
            output.put('\n');
        }

        if (!output)
        {
            output.close();
            return false;
        }
    }

    output.close();
    return !output.fail();
}
