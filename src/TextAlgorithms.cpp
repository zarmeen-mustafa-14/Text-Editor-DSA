// ============================================================================
// TextAlgorithms.cpp
// CS-218 Data Structures and Algorithms - Text Editor (CEP)
//
// Implements:
//   - MatchArray      : custom dynamic array for search results
//   - TextAlgorithms  : findAll(), replaceFirst(), replaceAll()
//
// Project conventions followed:
//   - Line numbers are 1-based, character positions are 0-based
//   - Search is case-sensitive and works on individual lines
//   - findAll() reports OVERLAPPING matches
//   - replaceFirst()/replaceAll() work on NON-overlapping matches
//   - Expected failures are reported through return values (no exceptions)
//   - Searching uses manual naive string matching (no std::string::find)
//
// Return value contract (so TextEditor knows when to push an undo record):
//   findAll()      : true  -> search completed (results.getSize() may be 0)
//                    false -> empty pattern, empty document, or memory failure
//   replaceFirst() : true  -> exactly one replacement was made
//                    false -> nothing changed (empty pattern, empty document,
//                             pattern not found, or failure); the document
//                             is left unchanged
//   replaceAll()   : true  -> at least one replacement made, `changes` holds
//                             every replacement, replacementCount > 0
//                    false -> nothing changed (document restored if a failure
//                             happened midway), changes is empty and
//                             replacementCount == 0
//
// Undo information:
//   replaceFirst(): changedPosition = {line, position}, originalText = the
//                   text that was replaced (the pattern). TextEditor builds
//                   the UndoRecord with newText = replacement.
//   replaceAll()  : every TextChange stores
//                       lineNumber, position (0-based, measured in the line
//                       AFTER all replacements on that line were applied),
//                       oldText (pattern), newText (replacement).
//                   Changes are stored in document order. To undo, restore
//                   them in REVERSE order: replace newText at `position`
//                   with oldText. Because later changes are undone first,
//                   the stored positions of earlier changes stay valid.
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

    // Replaces every NON-overlapping occurrence of `pattern` in `line`
    // (scanning left to right) and builds the resulting line in `newLine`.
    // The replacement text is never rescanned, so replacing "a" with "aa"
    // terminates correctly.
    //
    // If `changes` is not null, one TextChange per replacement is recorded
    // (positions measured in the NEW line). Returns false only if recording
    // a change fails (memory failure).
    // Time: O(n * m + k * r) where k = matches, r = replacement length
    bool replaceInLine(
        const std::string& line,
        const std::string& pattern,
        const std::string& replacement,
        int lineNumber,
        TextChangeArray* changes,
        std::string& newLine,
        int& count)
    {
        const int m = static_cast<int>(pattern.length());

        newLine.clear();
        count = 0;

        int copyFrom = 0;   // start of the part of `line` not yet copied
        int searchFrom = 0;

        while (true)
        {
            const int pos = findInLine(line, pattern, searchFrom);

            if (pos < 0)
            {
                break;
            }

            // Copy the untouched text before the match
            newLine.append(line, copyFrom, pos - copyFrom);

            const int newPosition = static_cast<int>(newLine.length());

            if (changes != nullptr)
            {
                TextChange change;
                change.lineNumber = lineNumber;
                change.position = newPosition;
                change.oldText = pattern;
                change.newText = replacement;

                if (!changes->add(change))
                {
                    return false;
                }
            }

            newLine.append(replacement);
            ++count;

            copyFrom = pos + m;     // skip the matched text
            searchFrom = pos + m;   // non-overlapping: continue after match
        }

        // Copy the remaining tail of the line
        newLine.append(line, copyFrom, std::string::npos);

        return true;
    }

    // Best-effort rollback used when replaceAll() fails midway.
    // Reverts the first `count` recorded changes in REVERSE order.
    void revertChanges(
        Document& document,
        const TextChangeArray& changes,
        int count)
    {
        for (int i = count - 1; i >= 0; --i)
        {
            TextChange change;
            std::string line;

            if (!changes.get(i, change))
            {
                continue;
            }

            if (!document.getLine(change.lineNumber, line))
            {
                continue;
            }

            const int pos = change.position;
            const int len = static_cast<int>(change.newText.length());

            if (pos < 0 || pos + len > static_cast<int>(line.length()))
            {
                continue;
            }

            std::string restored;
            restored.append(line, 0, pos);
            restored.append(change.oldText);
            restored.append(line, pos + len, std::string::npos);

            document.updateLine(change.lineNumber, restored);
        }
    }
}

// ============================================================================
// MatchArray - custom dynamic array of MatchPosition
// ============================================================================

MatchArray::MatchArray()
    : data(nullptr), size(0), capacity(0)
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
// Time: O(L * n * m) comparisons (L lines, n avg line length, m pattern length)
//       plus O(L^2) for walking the singly linked list via getLine().
// Space: O(k) for k matches (MatchArray)
bool TextAlgorithms::findAll(
    const Document& document,
    const std::string& pattern,
    MatchArray& results)
{
    results.clear();

    if (pattern.empty())
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
// Time: O(L * n * m) plus O(L^2) list traversal, Space: O(n) for a line copy
bool TextAlgorithms::replaceFirst(
    Document& document,
    const std::string& pattern,
    const std::string& replacement,
    MatchPosition& changedPosition,
    std::string& originalText)
{
    if (pattern.empty() || document.isEmpty())
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

        // Build the new line in one step so the update is atomic:
        // either the whole line is replaced or the document is unchanged.
        std::string newLine;
        newLine.append(line, 0, pos);
        newLine.append(replacement);
        newLine.append(line, pos + static_cast<int>(pattern.length()),
                       std::string::npos);

        if (!document.updateLine(lineNumber, newLine))
        {
            return false;
        }

        changedPosition.lineNumber = lineNumber;
        changedPosition.characterPosition = pos;
        originalText = pattern;     // text that was removed (needed for Undo)

        return true;
    }

    return false;   // pattern not found
}

// Replaces all non-overlapping occurrences of `pattern`.
// Works in two phases so a failure never leaves the document half-changed:
//   Phase 1: scan the document and record every change (document untouched)
//   Phase 2: apply the changes line by line (rolled back if a line update fails)
// Time: O(L * n * m) plus O(L^2) list traversal
// Space: O(k) for k recorded changes (TextChangeArray)
bool TextAlgorithms::replaceAll(
    Document& document,
    const std::string& pattern,
    const std::string& replacement,
    TextChangeArray& changes,
    int& replacementCount)
{
    changes.clear();
    replacementCount = 0;

    if (pattern.empty() || document.isEmpty())
    {
        return false;
    }

    const int lineCount = document.getLineCount();
    int total = 0;

    // ---------------- Phase 1: collect all changes ----------------
    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;
        std::string newLine;
        int count = 0;

        if (!document.getLine(lineNumber, line))
        {
            changes.clear();
            return false;
        }

        if (!replaceInLine(line, pattern, replacement,
                           lineNumber, &changes, newLine, count))
        {
            changes.clear();
            return false;   // memory failure, document untouched
        }

        total += count;
    }

    if (total == 0)
    {
        changes.clear();
        return false;       // pattern not found
    }

    // ---------------- Phase 2: apply the changes ----------------
    int appliedChanges = 0;     // changes already written to the document

    for (int lineNumber = 1; lineNumber <= lineCount; ++lineNumber)
    {
        std::string line;
        std::string newLine;
        int count = 0;

        if (!document.getLine(lineNumber, line)
            || !replaceInLine(line, pattern, replacement,
                              lineNumber, nullptr, newLine, count))
        {
            revertChanges(document, changes, appliedChanges);
            changes.clear();
            return false;
        }

        if (count == 0)
        {
            continue;   // line unaffected
        }

        if (!document.updateLine(lineNumber, newLine))
        {
            revertChanges(document, changes, appliedChanges);
            changes.clear();
            return false;
        }

        appliedChanges += count;
    }

    replacementCount = total;

    return true;
}
