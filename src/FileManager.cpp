#include "FileManager.h"

#include <fstream>
#include <ios>

bool FileManager::openFile(const MyString& filename, Document& document)
{
    if (filename.empty())
    {
        return false;
    }

    // Binary mode keeps file handling consistent across operating systems.
    std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        return false;
    }

    try
    {
        Document loadedDocument;
        MyString line;

        // Read each complete line directly into the project's MyString type.
        while (getLine(input, line))
        {
            if (!loadedDocument.appendLine(line))
            {
                return false;
            }
        }

        // A normal EOF is expected. Any other stream failure is an error.
        if (input.bad() || (input.fail() && !input.eof()))
        {
            return false;
        }

        document.swap(loadedDocument);
        return true;
    }
    catch (...)
    {
        // The original document remains unchanged on failure.
        return false;
    }
}

bool FileManager::saveFile(const MyString& filename, const Document& document)
{
    if (filename.empty())
    {
        return false;
    }

    std::ofstream output(
        filename.c_str(),
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
            MyString line;

            if (!document.getLine(lineNumber, line))
            {
                output.close();
                return false;
            }

            output.write(
                line.c_str(),
                static_cast<std::streamsize>(line.length())
            );

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
