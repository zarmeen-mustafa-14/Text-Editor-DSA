#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>

class Document
{
private:
    struct LineNode
    {
        std::string text;
        LineNode* next;

        LineNode(const std::string& lineText);
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
        const std::string& text
    );

    bool deleteLine(int lineNumber);

    bool getLine(
        int lineNumber,
        std::string& result
    ) const;

    bool updateLine(
        int lineNumber,
        const std::string& text
    );

    bool appendLine(const std::string& text);

    // Character operations.
    bool insertText(
        int lineNumber,
        int position,
        const std::string& text
    );

    bool deleteText(
        int lineNumber,
        int position,
        int length,
        std::string& deletedText
    );

    // Document output.
    void display() const;

    // Document statistics.
    int getLineCount() const;

    int getCharacterCount() const;

    int getWordCount() const;
};

#endif