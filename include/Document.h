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
    int lineCount;

    // Find a node using a 1-based line number.
    LineNode* getNode(int lineNumber) const;

public:
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