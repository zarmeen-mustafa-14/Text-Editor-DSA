#include "Document.h"

#include <iostream>
#include <sstream>

using namespace std;

// Creates a node containing one line of text.
Document::LineNode::LineNode(const string &lineText)
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
Document::LineNode *Document::getNode(int lineNumber) const
{
    if (lineNumber < 1 || lineNumber > lineCount)
    {
        return nullptr;
    }

    LineNode *current = head;

    for (int i = 1; i < lineNumber; i++)
    {
        current = current->next;
    }

    return current;
}

// Deletes all nodes and resets the document.
void Document::clear()
{
    LineNode *current = head;

    while (current != nullptr)
    {
        LineNode *nodeToDelete = current;
        current = current->next;
        delete nodeToDelete;
    }

    head = nullptr;
    lineCount = 0;
}

// Exchanges the contents of two documents without copying nodes.
void Document::swap(Document &other) noexcept
{
    // Swap the head pointers.
    LineNode *tempHead = head;
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
bool Document::insertLine(int lineNumber, const string &text)
{
    if (lineNumber < 1 || lineNumber > lineCount + 1)
    {
        return false;
    }

    // Create the new node before changing the linked list.
    LineNode *newNode = new LineNode(text);

    if (lineNumber == 1)
    {
        newNode->next = head;
        head = newNode;
    }
    else
    {
        LineNode *previous = getNode(lineNumber - 1);

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

    LineNode *nodeToDelete;

    if (lineNumber == 1)
    {
        nodeToDelete = head;
        head = head->next;
    }
    else
    {
        LineNode *previous = getNode(lineNumber - 1);

        nodeToDelete = previous->next;
        previous->next = nodeToDelete->next;
    }

    delete nodeToDelete;
    lineCount--;

    return true;
}

// Copies the requested line into result.
bool Document::getLine(int lineNumber, string &result) const
{
    LineNode *node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    result = node->text;
    return true;
}

// Replaces the complete text of an existing line.
bool Document::updateLine(int lineNumber, const string &text)
{
    LineNode *node = getNode(lineNumber);

    if (node == nullptr)
    {
        return false;
    }

    node->text = text;
    return true;
}

// Adds a new line at the end of the document.
bool Document::appendLine(const string &text)
{
    return insertLine(lineCount + 1, text);
}

// Inserts text at a 0-based character position.
// Position equal to the line length inserts at the end.
bool Document::insertText(
    int lineNumber,
    int position,
    const string &text)
{
    LineNode *node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        position > static_cast<int>(node->text.length()))
    {
        return false;
    }

    node->text.insert(static_cast<string::size_type>(position), text);

    return true;
}

// Deletes characters from a 0-based position.
// If length goes beyond the end of the line, deletes the remaining characters.
// The deleted characters are returned through deletedText.
bool Document::deleteText(
    int lineNumber,
    int position,
    int length,
    string &deletedText)
{
    LineNode *node = getNode(lineNumber);

    if (node == nullptr ||
        position < 0 ||
        length < 0 ||
        position > static_cast<int>(node->text.length()))
    {
        return false;
    }

    int availableLength =
        static_cast<int>(node->text.length()) - position;

    int actualLength = length;

    if (actualLength > availableLength)
    {
        actualLength = availableLength;
    }

    // Store the removed text before changing the original line.
    string removedText = node->text.substr(
        static_cast<string::size_type>(position),
        static_cast<string::size_type>(actualLength));

    node->text.erase(
        static_cast<string::size_type>(position),
        static_cast<string::size_type>(actualLength));

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

    LineNode *current = head;
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
    LineNode *current = head;

    while (current != nullptr)
    {
        characterCount += static_cast<int>(current->text.length());
        current = current->next;
    }

    return characterCount;
}

// Counts words separated by whitespace characters.
int Document::getWordCount() const
{
    int wordCount = 0;
    LineNode *current = head;

    while (current != nullptr)
    {
        istringstream lineStream(current->text);
        string word;

        while (lineStream >> word)
        {
            wordCount++;
        }

        current = current->next;
    }

    return wordCount;
}
