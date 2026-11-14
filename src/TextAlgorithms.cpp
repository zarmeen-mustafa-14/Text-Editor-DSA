// ============================================================================
// TextAlgorithms.cpp
// CS-218 Data Structures and Algorithms - Text Editor (CEP)
//
// Implements:
//   - MatchArray      : custom dynamic array for search results
//   - TextAlgorithms  : findAll(), replaceFirst(), replaceAll()
//
// Library usage:
//   No string library facilities are used for the algorithms. There is no
//   <cstring>, no strlen/strcpy, and no std::string::find/substr/append/
//   compare/etc. Searching, copying and building of lines are all written by
//   hand with loops over characters. std::string appears only as the data
//   type required by the Document interface, and only its length() and
//   operator[] accessors are read here; new lines are built in a manually
//   allocated char buffer and then handed to Document::updateLine().
//
// Project conventions followed:
//   - Line numbers are 1-based, character positions are 0-based
//   - Search is case-sensitive and works on individual lines
//   - findAll() reports OVERLAPPING matches
//   - replaceFirst()/replaceAll() work on NON-overlapping matches
//   - Expected failures are reported through return values (no exceptions)
//
// Return value contract:
//   findAll()      : true  -> search completed (results.getSize() may be 0)
//                    false -> empty pattern, empty document, or memory failure
//   replaceFirst() : true  -> exactly one replacement was made
//                    false -> nothing changed (empty pattern, empty document,
//                             pattern not found, or failure)
//   replaceAll()   : true  -> at least one replacement made, count > 0
//                    false -> nothing changed (the document is restored if a
//                             failure happens midway), count == 0
// ============================================================================

#include "TextAlgorithms.h"

#include <new>

// ----------------------------------------------------------------------------
// Internal helpers (not visible outside this file)
// ----------------------------------------------------------------------------
namespace
{
    const int INITIAL_CAPACITY = 8;

    // Manual naive string matching.
    // Returns the 0-based index of the first occurrence of `pattern` in `line`
    // at or after `start`, or -1 if there is none.
    // Time: O(n * m) worst case, Space: O(1)
    int findInLine(
        const std::string& line,
        const std::string& pattern,
        int start)
    {
        const int n = static_cast<int>(line.length());
        const int m = static_cast<int>(pattern.length());

        if (m == 0 || start < 0)
        {
            return -1;
        }

        for (int i = start; i + m <= n; ++i)
        {
            int j = 0;

            while (j < m && line[i + j] == pattern[j])
            {
                ++j;
            }

            if (j == m)
            {
                return i;
            }
        }

        return -1;
    }

    // Counts non-overlapping matches in a line, stopping at `limit`
    // (pass a negative limit for "no limit").
    int countMatches(
        const std::string& line,
        const std::string& pattern,
        int limit)
    {
        const int m = static_cast<int>(pattern.length());
        int count = 0;
        int from = 0;

        while (limit < 0 || count < limit)
        {
            const int pos = findInLine(line, pattern, from);

            if (pos < 0)
            {
                break;
            }

            ++count;
            from = pos + m;     // non-overlapping
        }

        return count;
    }

    // Builds a copy of `line` in which the first `maxReplacements`
    // non-overlapping occurrences of `pattern` are replaced (negative =
    // all occurrences). The replacement text is never rescanned.
    //
    // The result is assembled in a manually allocated char buffer using
    // plain loops, then stored into `result`.
    // `replaced` receives the number of replacements made.
    // Returns false only on memory failure.
    // Time: O(n * m + n + k * r)  (k matches, r replacement length)
    // Space: O(n + k * r)
    bool buildReplacedLine(
        const std::string& line,
        const std::string& pattern,
        const std::string& replacement,
        int maxReplacements,
        std::string& result,
        int& replaced)
    {
        const int n = static_cast<int>(line.length());
        const int m = static_cast<int>(pattern.length());
        const int r = static_cast<int>(replacement.length());

        replaced = countMatches(line, pattern, maxReplacements);

        const int newLength = n + replaced * (r - m);

        char* buffer = new (std::nothrow) char[newLength + 1];

        if (buffer == nullptr)
        {
            return false;
        }

        int src = 0;        // read index in line
        int dst = 0;        // write index in buffer
        int searchFrom = 0;

        for (int done = 0; done < replaced; ++done)
        {
            const int pos = findInLine(line, pattern, searchFrom);

            // untouched text before the match
            while (src < pos)
            {
                buffer[dst++] = line[src++];
            }

            // replacement text
            for (int k = 0; k < r; ++k)
            {
                buffer[dst++] = replacement[k];
            }

            src = pos + m;          // skip the matched text
            searchFrom = pos + m;   // non-overlapping
        }

        // remaining tail of the line
        while (src < n)
        {
            buffer[dst++] = line[src++];
        }

        buffer[dst] = '\0';

        bool ok = true;

        try
        {
            result = std::string(buffer, static_cast<std::size_t>(dst));
        }
        catch (...)
        {
            ok = false;
        }

        delete[] buffer;

        return ok;
    }
}

// ============================================================================
// MatchArray - custom dynamic array of MatchPosition
// ============================================================================

MatchArray::MatchArray()
    : data(nullptr), capacity(0), size(0)
{
    data = new (std::nothrow) MatchPosition[INITIAL_CAPACITY];

    if (data != nullptr)
    {
        capacity = INITIAL_CAPACITY;
    }
}

MatchArray::~MatchArray()
{
    delete[] data;
    data = nullptr;
    size = 0;
    capacity = 0;
}

// Doubles the capacity. Amortized O(1) per add().
bool MatchArray::resize()
{
    const int newCapacity = (capacity > 0) ? capacity * 2 : INITIAL_CAPACITY;

    if (newCapacity <= capacity)
    {
        return false;   // integer overflow guard
    }

    MatchPosition* newData = new (std::nothrow) MatchPosition[newCapacity];

    if (newData == nullptr)
    {
        return false;   // old data is still intact
    }

    for (int i = 0; i < size; ++i)
    {
        newData[i] = data[i];
    }

    delete[] data;
    data = newData;
    capacity = newCapacity;

    return true;
}

bool MatchArray::add(const MatchPosition& position)
{
    if (size >= capacity)
    {
        if (!resize())
        {
            return false;
        }
    }

    data[size] = position;
    ++size;

    return true;
}

int MatchArray::getSize() const
{
    return size;
}

bool MatchArray::get(int index, MatchPosition& result) const
{
    if (index < 0 || index >= size || data == nullptr)
    {
        return false;
    }

    result = data[index];

    return true;
}

void MatchArray::clear()
{
    size = 0;   // keep the allocated storage for reuse
}

// ============================================================================
// TextAlgorithms
// ============================================================================

// Finds every occurrence (including overlapping ones) of `pattern`.
// Time: O(L * n * m) for matching, plus O(L^2) for walking the singly linked
//       list through getLine() (L lines, n line length, m pattern length)
// Space: O(k) for k matches
bool TextAlgorithms::findAll(
    const Document& document,
    const std::string& pattern,
    MatchArray& results)
{
    results.clear();

    if (pattern.length() == 0)
    {
        return false;   // searching for an empty string is invalid
    }

    if (document.isEmpty())
    {
        return false;   // nothing to search
    }

    const int lineCount = document.getLineCount();

    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;

        if (!document.getLine(lineNumber, line))
        {
            results.clear();
            return false;
        }

        int start = 0;

        while (true)
        {
            const int pos = findInLine(line, pattern, start);

            if (pos < 0)
            {
                break;
            }

            MatchPosition match;
            match.lineNumber = lineNumber;
            match.characterPosition = pos;

            if (!results.add(match))
            {
                results.clear();
                return false;   // memory failure
            }

            start = pos + 1;    // overlapping matches are included
        }
    }

    return true;
}

// Replaces the first occurrence of `pattern` in the document.
// The whole line is rebuilt and written with one updateLine() call, so the
// change is atomic: either it fully happens or the document is unchanged.
// Time: O(L * n * m) plus O(L^2) list traversal, Space: O(n)
bool TextAlgorithms::replaceFirst(
    Document& document,
    const std::string& pattern,
    const std::string& replacement,
    MatchPosition& changedPosition,
    std::string& originalText)
{
    if (pattern.length() == 0 || document.isEmpty())
    {
        return false;
    }

    const int lineCount = document.getLineCount();

    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;

        if (!document.getLine(lineNumber, line))
        {
            return false;
        }

        const int pos = findInLine(line, pattern, 0);

        if (pos < 0)
        {
            continue;   // no match on this line, try the next one
        }

        std::string newLine;
        int replaced = 0;

        if (!buildReplacedLine(line, pattern, replacement, 1,
                               newLine, replaced))
        {
            return false;
        }

        if (!document.updateLine(lineNumber, newLine))
        {
            return false;
        }

        changedPosition.lineNumber = lineNumber;
        changedPosition.characterPosition = pos;
        originalText = pattern;     // text that was replaced

        return true;
    }

    return false;   // pattern not found
}

// Replaces all non-overlapping occurrences of `pattern`.
//   Phase 1: count matches and affected lines (document untouched)
//   Phase 2: rewrite each affected line; the original lines are saved first so
//            everything can be restored if an update fails midway
// Time: O(L * n * m) plus O(L^2) list traversal
// Space: O(S) where S is the total size of the affected lines (backup)
bool TextAlgorithms::replaceAll(
    Document& document,
    const std::string& pattern,
    const std::string& replacement,
    int& replacementCount)
{
    replacementCount = 0;

    if (pattern.length() == 0 || document.isEmpty())
    {
        return false;
    }

    const int lineCount = document.getLineCount();

    // ---------------- Phase 1: count ----------------
    int totalMatches = 0;
    int affectedLines = 0;

    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;

        if (!document.getLine(lineNumber, line))
        {
            return false;
        }

        const int count = countMatches(line, pattern, -1);

        if (count > 0)
        {
            totalMatches += count;
            ++affectedLines;
        }
    }

    if (totalMatches == 0)
    {
        return false;   // pattern not found
    }

    // Backup of every line we are about to change (for rollback)
    std::string* backupText = new (std::nothrow) std::string[affectedLines];
    int* backupLine = new (std::nothrow) int[affectedLines];

    if (backupText == nullptr || backupLine == nullptr)
    {
        delete[] backupText;
        delete[] backupLine;
        return false;   // memory failure, document untouched
    }

    // ---------------- Phase 2: apply ----------------
    int saved = 0;      // number of lines backed up and updated so far
    bool failed = false;

    for (int lineNumber = 1; lineNumber <= lineCount && !failed; ++lineNumber)
    {
        std::string line;

        if (!document.getLine(lineNumber, line))
        {
            failed = true;
            break;
        }

        if (findInLine(line, pattern, 0) < 0)
        {
            continue;   // line unaffected
        }

        std::string newLine;
        int replaced = 0;

        if (!buildReplacedLine(line, pattern, replacement, -1,
                               newLine, replaced))
        {
            failed = true;
            break;
        }

        if (!document.updateLine(lineNumber, newLine))
        {
            failed = true;
            break;
        }

        backupText[saved] = line;
        backupLine[saved] = lineNumber;
        ++saved;
    }

    if (failed)
    {
        // Restore every line that was already changed
        for (int i = saved - 1; i >= 0; --i)
        {
            document.updateLine(backupLine[i], backupText[i]);
        }

        delete[] backupText;
        delete[] backupLine;
        return false;
    }

    delete[] backupText;
    delete[] backupLine;

    replacementCount = totalMatches;

    return true;
}
