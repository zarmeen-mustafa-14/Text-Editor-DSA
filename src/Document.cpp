<<<<<<< HEAD
#include "Document.h"

#include <iostream>

using namespace std;

// Creates a node containing one line of text.
Document::LineNode::LineNode(const MyString& lineText)
    : text(lineText), next(nullptr)
{
}

// Creates an empty document.
Document::Document()
    : head(nullptr), lineCount(0)
{
}

// Releases all dynamically allocated nodes.
Document::~Document()
{
    clear();
}

// Returns the node at a 1-based line number.
// Returns nullptr if the line number is invalid.
Document::LineNode* Document::getNode(int lineNumber) const
{
    if (lineNumber < 1 || lineNumber > lineCount)
    {
        return nullptr;
    }

    LineNode* current = head;

    for (int i = 1; i < lineNumber; i++)
    {
        current = current->next;
    }

    return current;
}

// Deletes all nodes and resets the document.
void Document::clear()
{
    LineNode* current = head;

    while (current != nullptr)
    {
        LineNode* nodeToDelete = current;
        current = current->next;
        delete nodeToDelete;
    }

    head = nullptr;
    lineCount = 0;
}

// Exchanges the contents of two documents without copying nodes.
void Document::swap(Document& other) noexcept
{
    // Swap the head pointers.
    LineNode* tempHead = head;
    head = other.head;
    other.head = tempHead;

    // Swap the line counts.
    int tempCount = lineCount;
    lineCount = other.lineCount;
    other.lineCount = tempCount;
}

// Returns true if the document has no lines.
bool Document::isEmpty() const
{
    return head == nullptr;
}

// Inserts a line at a 1-based position.
// Valid positions are from 1 to lineCount + 1.
bool Document::insertLine(int lineNumber, const MyString& text)
{
    if (lineNumber < 1 || lineNumber > lineCount + 1)
    {
        return false;
    }

    // Create the new node before changing the linked list.
    LineNode* newNode = new LineNode(text);

    if (lineNumber == 1)
    {
        newNode->next = head;
        head = newNode;
    }
    else
    {
        LineNode* previous = getNode(lineNumber - 1);

        newNode->next = previous->next;
        previous->next = newNode;
    }

    lineCount++;
    return true;
}

// Deletes a line at a 1-based position.
bool Document::deleteLine(int lineNumber)
{
    if (lineNumber < 1 || lineNumber > lineCount)
    {
        return false;
    }

    LineNode* nodeToDelete;

    if (lineNumber == 1)
    {
        nodeToDelete = head;
        head = head->next;
    }
    else
    {
        LineNode* previous = getNode(lineNumber - 1);

        nodeToDelete = previous->next;
        previous->next = nodeToDelete->next;
    }

    delete nodeToDelete;
    lineCount--;

    return true;
}

// Copies the requested line into result.
bool Document::getLine(int lineNumber, MyString& result) const
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    result = node->text;
    return true;
}

// Replaces the complete text of an existing line.
bool Document::updateLine(int lineNumber, const MyString& text)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    node->text = text;
    return true;
}

// Adds a new line at the end of the document.
bool Document::appendLine(const MyString& text)
{
    return insertLine(lineCount + 1, text);
}

// Inserts text at a 0-based character position.
// Position equal to the line length inserts at the end.
bool Document::insertText(
    int lineNumber,
    int position,
    const MyString& text)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        position > node->text.length())
    {
        return false;
    }

    node->text.insert(position, text);

    return true;
}

// Deletes characters from a 0-based position.
// If length goes beyond the end of the line,
// deletes the remaining characters.
// The deleted characters are returned through deletedText.
bool Document::deleteText(
    int lineNumber,
    int position,
    int length,
    MyString& deletedText)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        length < 0 ||
        position > node->text.length())
    {
        return false;
    }

    int availableLength = node->text.length() - position;

    int actualLength = length;

    if (actualLength > availableLength)
    {
        actualLength = availableLength;
    }

    // Store the removed text before changing the original line.
    MyString removedText = node->text.subpart(
        position,
        actualLength);

    node->text.erase(position, actualLength);

    deletedText.swap(removedText);

    return true;
}

// Displays all lines with their 1-based line numbers.
void Document::display() const
{
    if (head == nullptr)
    {
        cout << "Document is empty.\n";
        return;
    }

    LineNode* current = head;
    int currentLine = 1;

    while (current != nullptr)
    {
        cout << currentLine << ". " << current->text << '\n';

        current = current->next;
        currentLine++;
    }
}

// Returns the number of lines.
int Document::getLineCount() const
{
    return lineCount;
}

// Returns the total number of characters.
// Newline separators between lines are not counted.
int Document::getCharacterCount() const
{
    int characterCount = 0;
    LineNode* current = head;

    while (current != nullptr)
    {
        characterCount += current->text.length();
        current = current->next;
    }

    return characterCount;
}

// Counts words separated by whitespace characters.
int Document::getWordCount() const
{
    int wordCount = 0;
    LineNode* current = head;

    while (current != nullptr)
    {
        bool insideWord = false;

        for (int i = 0; i < current->text.length(); i++)
        {
            char ch = current->text[i];

            // Check for whitespace characters.
            if (ch == ' ' || ch == '\t' ||
                ch == '\n' || ch == '\r' ||
                ch == '\f' || ch == '\v')
            {
                insideWord = false;
            }
            else
            {
                if (!insideWord)
                {
                    wordCount++;
                    insideWord = true;
                }
            }
        }

        current = current->next;
    }

    return wordCount;
=======
#include "Document.h"

#include <iostream>

using namespace std;

// Creates a node containing one line of text.
Document::LineNode::LineNode(const MyString& lineText)
    : text(lineText), next(nullptr)
{
}

// Creates an empty document.
Document::Document()
    : head(nullptr), tail(nullptr), lineCount(0)
{
}

// Releases all dynamically allocated nodes.
Document::~Document()
{
    clear();
}

// Returns the node at a 1-based line number.
// Returns nullptr if the line number is invalid.
Document::LineNode* Document::getNode(int lineNumber) const
{
    if (lineNumber < 1 || lineNumber > lineCount)
    {
        return nullptr;
    }

    LineNode* current = head;

    for (int i = 1; i < lineNumber; i++)
    {
        current = current->next;
    }

    return current;
}

// Deletes all nodes and resets the document.
void Document::clear()
{
    LineNode* current = head;

    while (current != nullptr)
    {
        LineNode* nodeToDelete = current;
        current = current->next;
        delete nodeToDelete;
    }

    head = nullptr;
    tail = nullptr;
    lineCount = 0;
}

// Exchanges the contents of two documents without copying nodes.
void Document::swap(Document& other) noexcept
{
    // Swap the head pointers.
    LineNode* tempHead = head;
    head = other.head;
    other.head = tempHead;

    LineNode* tempTail = tail;
    tail = other.tail;
    other.tail = tempTail;

    // Swap the line counts.
    int tempCount = lineCount;
    lineCount = other.lineCount;
    other.lineCount = tempCount;
}

// Returns a cursor positioned on the first line.
Document::LineIterator Document::firstLine() const
{
    return LineIterator(head, false);
}

// Returns a cursor that is allowed to replace line text.
Document::LineIterator Document::firstLineForUpdate()
{
    return LineIterator(head, true);
}

// Returns true if the document has no lines.
bool Document::isEmpty() const
{
    return head == nullptr;
}

// Inserts a line at a 1-based position.
// Valid positions are from 1 to lineCount + 1.
bool Document::insertLine(int lineNumber, const MyString& text)
{
    if (lineNumber < 1 || lineNumber > lineCount + 1)
    {
        return false;
    }

    // Create the new node before changing the linked list.
    LineNode* newNode = new LineNode(text);

    if (lineNumber == 1)
    {
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) tail = newNode;
    }
    else
    {
        // Appending at the end uses the tail pointer instead of walking.
        LineNode* previous =
            (lineNumber == lineCount + 1) ? tail : getNode(lineNumber - 1);

        newNode->next = previous->next;
        previous->next = newNode;
        if (previous == tail) tail = newNode;
    }

    lineCount++;
    return true;
}

// Inserts a line at any 1-based number; gaps are filled with empty lines.
bool Document::insertLineExtending(
    int lineNumber,
    const MyString& text,
    int& linesAdded)
{
    linesAdded = 0;

    if (lineNumber < 1)
    {
        return false;
    }

    // Normal case: inside the document or directly after the last line.
    if (lineNumber <= lineCount + 1)
    {
        if (!insertLine(lineNumber, text))
        {
            return false;
        }
        linesAdded = 1;
        return true;
    }

    // Gap case. Use long long so lineCount + padding cannot overflow.
    const long long gap = static_cast<long long>(lineNumber) - lineCount - 1;
    if (gap > MAX_PADDING_LINES)
    {
        return false;
    }

    const int originalCount = lineCount;

    try
    {
        for (long long i = 0; i < gap; ++i)
        {
            appendLine(MyString());
            ++linesAdded;
        }
        appendLine(text);
        ++linesAdded;
    }
    catch (...)
    {
        // Roll back so a failed insert never leaves stray empty lines.
        while (lineCount > originalCount)
        {
            deleteLine(lineCount);
        }
        linesAdded = 0;
        throw;
    }

    return true;
}

// Deletes a line at a 1-based position.
bool Document::deleteLine(int lineNumber)
{
    if (lineNumber < 1 || lineNumber > lineCount)
    {
        return false;
    }

    LineNode* nodeToDelete;

    if (lineNumber == 1)
    {
        nodeToDelete = head;
        head = head->next;
        if (head == nullptr) tail = nullptr;
    }
    else
    {
        LineNode* previous = getNode(lineNumber - 1);

        nodeToDelete = previous->next;
        previous->next = nodeToDelete->next;
        if (nodeToDelete == tail) tail = previous;
    }

    delete nodeToDelete;
    lineCount--;

    return true;
}

// Copies the requested line into result.
bool Document::getLine(int lineNumber, MyString& result) const
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    result = node->text;
    return true;
}

// Replaces the complete text of an existing line.
bool Document::updateLine(int lineNumber, const MyString& text)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    node->text = text;
    return true;
}

// Adds a new line at the end of the document.
bool Document::appendLine(const MyString& text)
{
    return insertLine(lineCount + 1, text);
}

// Inserts text at a 0-based character position.
// Position equal to the line length inserts at the end.
bool Document::insertText(
    int lineNumber,
    int position,
    const MyString& text)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        position > node->text.length())
    {
        return false;
    }

    node->text.insert(position, text);

    return true;
}

// Inserts text at any line/position, creating lines and padding as needed.
bool Document::insertTextExtending(
    int lineNumber,
    int position,
    const MyString& text,
    int& linesAdded,
    int& insertedAt,
    MyString& insertedText)
{
    linesAdded = 0;
    insertedAt = 0;
    insertedText.clear();

    if (lineNumber < 1 || position < 0)
    {
        return false;
    }

    // Work out how much padding is needed before touching anything.
    const long long missingLines =
        (lineNumber > lineCount) ? static_cast<long long>(lineNumber) - lineCount : 0;

    if (missingLines > MAX_PADDING_LINES)
    {
        return false;
    }

    int existingLength = 0;
    if (missingLines == 0)
    {
        existingLength = getNode(lineNumber)->text.length();
    }

    const int spaces = (position > existingLength) ? position - existingLength : 0;
    if (spaces > MAX_PADDING_SPACES)
    {
        return false;
    }

    // Build the exact string that will be written (padding + text) first.
    MyString toWrite;
    for (int i = 0; i < spaces; ++i)
    {
        toWrite.append(" ");
    }
    toWrite.append(text);

    const int originalCount = lineCount;

    try
    {
        for (long long i = 0; i < missingLines; ++i)
        {
            appendLine(MyString());
            ++linesAdded;
        }

        LineNode* node = getNode(lineNumber);
        const int startAt = (position < node->text.length())
            ? position : node->text.length();

        node->text.insert(startAt, toWrite);

        insertedAt = startAt;
        insertedText.swap(toWrite);
    }
    catch (...)
    {
        while (lineCount > originalCount)
        {
            deleteLine(lineCount);
        }
        linesAdded = 0;
        throw;
    }

    return true;
}

// Deletes characters from a 0-based position.
// If length goes beyond the end of the line,
// deletes the remaining characters.
// The deleted characters are returned through deletedText.
bool Document::deleteText(
    int lineNumber,
    int position,
    int length,
    MyString& deletedText)
{
    LineNode* node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        length < 0 ||
        position > node->text.length())
    {
        return false;
    }

    int availableLength = node->text.length() - position;

    int actualLength = length;

    if (actualLength > availableLength)
    {
        actualLength = availableLength;
    }

    // Store the removed text before changing the original line.
    MyString removedText = node->text.subpart(
        position,
        actualLength);

    node->text.erase(position, actualLength);

    deletedText.swap(removedText);

    return true;
}

// Displays all lines with their 1-based line numbers.
void Document::display() const
{
    if (head == nullptr)
    {
        cout << "Document is empty.\n";
        return;
    }

    LineNode* current = head;
    int currentLine = 1;

    while (current != nullptr)
    {
        cout << currentLine << ". " << current->text << '\n';

        current = current->next;
        currentLine++;
    }
}

// Returns the number of lines.
int Document::getLineCount() const
{
    return lineCount;
}

// Returns the total number of characters.
// Newline separators between lines are not counted.
int Document::getCharacterCount() const
{
    int characterCount = 0;
    LineNode* current = head;

    while (current != nullptr)
    {
        characterCount += current->text.length();
        current = current->next;
    }

    return characterCount;
}

// Counts words separated by whitespace characters.
int Document::getWordCount() const
{
    int wordCount = 0;
    LineNode* current = head;

    while (current != nullptr)
    {
        bool insideWord = false;

        for (int i = 0; i < current->text.length(); i++)
        {
            char ch = current->text[i];

            // Check for whitespace characters.
            if (ch == ' ' || ch == '\t' ||
                ch == '\n' || ch == '\r' ||
                ch == '\f' || ch == '\v')
            {
                insideWord = false;
            }
            else
            {
                if (!insideWord)
                {
                    wordCount++;
                    insideWord = true;
                }
            }
        }

        current = current->next;
    }

    return wordCount;
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
}