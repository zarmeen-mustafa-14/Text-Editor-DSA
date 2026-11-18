#include "FileManager.h"

#include <fstream>
#include <string>

bool FileManager::openFile(const std::string& filename, Document& document)
{
    if (filename.empty())
    {
        return false;
    }

    // Binary mode makes newline handling identical on Windows and POSIX.
    // A temporary document ensures a failed read never damages the current one.
    std::ifstream input(filename, std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        return false;
    }

    try
    {
        Document loadedDocument;
        std::string line;

        while (std::getline(input, line))
        {
            // Normalize CRLF files to the editor's internal '\n' separator.
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (!loadedDocument.appendLine(line))
            {
                return false;
            }
        }

        // eofbit is expected after the final getline. badbit, or failbit
        // without eofbit, indicates an actual read failure.
        if (input.bad() || (input.fail() && !input.eof()))
        {
            return false;
        }

        document.swap(loadedDocument);
        return true;
    }
    catch (...)
    {
        // Allocation or string-copy failure leaves `document` untouched.
        return false;
    }
}

bool FileManager::saveFile(const std::string& filename, const Document& document)
{
    if (filename.empty())
    {
        return false;
    }

    // Binary mode prevents platform-specific newline conversion. The file
    // format uses one '\n' separator between lines and no trailing separator.
    std::ofstream output(
        filename,
        std::ios::out | std::ios::trunc | std::ios::binary
    );

    if (!output.is_open())
    {
        return false;
    }

    try
    {
        const int lineCount = document.getLineCount();

        for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
        {
            std::string line;
            if (!document.getLine(lineNumber, line))
            {
                output.close();
                return false;
            }

            output.write(line.data(), static_cast<std::streamsize>(line.size()));
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
    catch (...)
    {
        output.close();
        return false;
    }
}
