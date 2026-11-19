#include "FileManager.h"

#include <cstdio>
#include <fstream>
#include <ios>

namespace
{
    bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
               c == '\f' || c == '\v';
    }

    bool isSeparator(char c)
    {
        return c == '/' || c == '\\';
    }

    // A directory can be opened by ifstream on some platforms, so reject it
    // explicitly: opening it for reading is allowed, reading from it is not.
    bool isRegularReadable(const MyString& filename)
    {
        std::ifstream probe(filename.c_str(), std::ios::in | std::ios::binary);
        if (!probe.is_open())
        {
            return false;
        }

        char c;
        probe.get(c);
        // bad() means a read error (e.g. directory). EOF on an empty file is OK.
        return !probe.bad();
    }
}

MyString FileManager::normalizeFilename(const MyString& filename)
{
    int start = 0;
    int end = filename.length();

    while (start < end && isWhitespace(filename[start])) ++start;
    while (end > start && isWhitespace(filename[end - 1])) --end;

    if (end - start >= 2 &&
        filename[start] == '"' && filename[end - 1] == '"')
    {
        ++start;
        --end;
        while (start < end && isWhitespace(filename[start])) ++start;
        while (end > start && isWhitespace(filename[end - 1])) --end;
    }

    return filename.subpart(start, end - start);
}

bool FileManager::isValidFilename(const MyString& filename)
{
    const int n = filename.length();

    if (n == 0)
    {
        return false;
    }

    for (int i = 0; i < n; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(filename[i]);

        if (c < 32 || c == 127 ||
            c == '<' || c == '>' || c == '"' ||
            c == '|' || c == '?' || c == '*')
        {
            return false;
        }
    }

    if (isSeparator(filename[n - 1]))
    {
        return false;
    }

    if (filename == MyString(".") || filename == MyString(".."))
    {
        return false;
    }

    return true;
}

bool FileManager::fileExists(const MyString& filename)
{
    if (filename.empty())
    {
        return false;
    }

    std::ifstream probe(filename.c_str(), std::ios::in | std::ios::binary);
    return probe.is_open();
}

bool FileManager::openFile(const MyString& filename, Document& document)
{
    if (!isValidFilename(filename))
    {
        return false;
    }

    if (!isRegularReadable(filename))
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
        bool sawNul = false;

        // Read each complete line directly into the project's MyString type.
        while (true)
        {
            bool lineHadNul = false;

            if (!getLine(input, line, &lineHadNul))
            {
                break;
            }

            if (lineHadNul)
            {
                sawNul = true;
                break;
            }

            if (!loadedDocument.appendLine(line))
            {
                return false;
            }
        }

        // Binary files cannot be represented safely, refuse instead of
        // silently dropping bytes.
        if (sawNul)
        {
            return false;
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
    if (!isValidFilename(filename))
    {
        return false;
    }

    try
    {
        MyString tempName(filename);
        tempName.append(".tmp");

        bool ok = true;

        {
            std::ofstream output(
                tempName.c_str(),
                std::ios::out | std::ios::trunc | std::ios::binary
            );

            if (!output.is_open())
            {
                return false;
            }

            // Walk the list once with an iterator (O(n), not O(n^2)).
            for (Document::LineIterator it = document.firstLine();
                 it.valid(); it.next())
            {
                const MyString& line = it.text();

                output.write(
                    line.c_str(),
                    static_cast<std::streamsize>(line.length())
                );
                output.put('\n');

                if (!output)
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                output.flush();
                ok = !output.fail();
            }
        }   // file closed here

        if (!ok)
        {
            std::remove(tempName.c_str());
            return false;
        }

        // Replace the target. rename() cannot overwrite on Windows, so fall
        // back to remove + rename there.
        if (std::rename(tempName.c_str(), filename.c_str()) != 0)
        {
            std::remove(filename.c_str());

            if (std::rename(tempName.c_str(), filename.c_str()) != 0)
            {
                std::remove(tempName.c_str());
                return false;
            }
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool FileManager::deleteFile(const MyString& filename)
{
    if (!isValidFilename(filename))
    {
        return false;
    }

    // Refuse names that do not exist so the caller can report it properly.
    if (!fileExists(filename))
    {
        return false;
    }

    // remove() also deletes empty directories on some platforms, so make sure
    // we are looking at a readable regular file first.
    if (!isRegularReadable(filename))
    {
        return false;
    }

    return std::remove(filename.c_str()) == 0;
}