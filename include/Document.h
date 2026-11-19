#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "MyString.h"

class Document
{
private:
    struct LineNode
    {
        MyString text;
        LineNode* next;

        LineNode(const MyString& lineText);
    };

    LineNode* head;
    LineNode* tail;     // last node, makes appendLine O(1)
    int lineCount;

    // Find a node using a 1-based line number.
    LineNode* getNode(int lineNumber) const;

public:
    // Safety limits for "write anywhere" operations that must create missing
    // lines / pad with spaces. They stop a typo such as line 999999999 from
    // exhausting memory.
    static const int MAX_PADDING_LINES  = 100000;
    static const int MAX_PADDING_SPACES = 10000;

    // Read-only cursor that walks the list in O(1) per step
    // (calling getLine(i) in a loop would be O(n^2)).
    class LineIterator
    {
    private:
        LineNode* current;
        bool writable;
        friend class Document;
        LineIterator(LineNode* node, bool canWrite)
            : current(node), writable(canWrite) {}

    public:
        bool valid() const { return current != nullptr; }
        const MyString& text() const { return current->text; }
        void next() { if (current != nullptr) current = current->next; }

        // Replaces the text of the current line in O(1). Only iterators from
        // firstLineForUpdate() may write; others return false.
        bool setText(const MyString& newText)
        {
            if (!writable || current == nullptr) return false;
            current->text = newText;
            return true;
        }
    };

    // Read-only cursor.
    LineIterator firstLine() const;

    // Cursor that can also replace line text (see LineIterator::setText).
    LineIterator firstLineForUpdate();

    Document();
    ~Document();

    // Prevent shallow copying of owned linked-list nodes.
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // Remove all lines.
    void clear();

    // Exchange document contents without copying nodes.
    void swap(Document& other) noexcept;

    // Document state.
    bool isEmpty() const;

    // Line operations.
    bool insertLine(
        int lineNumber,
        const MyString& text);

    // Like insertLine, but lineNumber may be ANY value >= 1. If it is beyond
    // lineCount + 1, empty lines are created to fill the gap.
    // linesAdded receives how many lines were created (>= 1 on success).
    // Returns false (document untouched) for invalid numbers or if the gap
    // would exceed MAX_PADDING_LINES.
    bool insertLineExtending(
        int lineNumber,
        const MyString& text,
        int& linesAdded);

    bool deleteLine(int lineNumber);

    bool getLine(
        int lineNumber,
        MyString& result) const;

    bool updateLine(
        int lineNumber,
        const MyString& text);

    bool appendLine(const MyString& text);

    // Character operations.
    bool insertText(
        int lineNumber,
        int position,
        const MyString& text);

    // Like insertText, but works on ANY line >= 1 and ANY position >= 0:
    //  - missing lines are created (empty),
    //  - a position beyond the end of the line is padded with spaces.
    // linesAdded / insertedAt / insertedText describe exactly what was
    // written so it can be undone: insertedText (padding + text) starts at
    // column insertedAt of lineNumber.
    // On failure the document is unchanged.
    bool insertTextExtending(
        int lineNumber,
        int position,
        const MyString& text,
        int& linesAdded,
        int& insertedAt,
        MyString& insertedText);

    bool deleteText(
        int lineNumber,
        int position,
        int length,
        MyString& deletedText);

    // Document output.
    void display() const;

    // Document statistics.
    int getLineCount() const;

    int getCharacterCount() const;

    int getWordCount() const;
};

#endif