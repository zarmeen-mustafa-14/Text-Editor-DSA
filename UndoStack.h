#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <string>

enum class OperationType
{
    INSERT_TEXT,
    DELETE_TEXT,
    REPLACE_TEXT,
    REPLACE_ALL,
    INSERT_LINE,
    DELETE_LINE
};

struct TextChange
{
    int lineNumber;
    int position;

    std::string oldText;
    std::string newText;
};

class TextChangeArray
{
private:
    TextChange* data;
    int size;
    int capacity;

    bool resize();

public:
    TextChangeArray();
    ~TextChangeArray();

    // Required because UndoRecord owns a collection of changes.
    TextChangeArray(const TextChangeArray& other);

    TextChangeArray& operator=(
        const TextChangeArray& other
    );

    bool add(const TextChange& change);

    bool get(
        int index,
        TextChange& result
    ) const;

    int getSize() const;

    bool isEmpty() const;

    void clear();
};

struct UndoRecord
{
    OperationType type;

    int lineNumber;
    int position;

    std::string oldText;
    std::string newText;

    TextChangeArray changes;

    UndoRecord();
};

class UndoStack
{
private:
    struct UndoNode
    {
        UndoRecord record;
        UndoNode* next;

        UndoNode(const UndoRecord& undoRecord);
    };

    UndoNode* top;
    int size;

public:
    UndoStack();
    ~UndoStack();

    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    bool isEmpty() const;

    int getSize() const;

    // Add a record. Allocation failures may throw.
    void push(const UndoRecord& record);

    // Read the top record without removing it.
    bool peek(UndoRecord& result) const;

    // Remove the top record after its operation is reversed.
    bool discardTop();

    // Retrieve and remove the top record.
    bool pop(UndoRecord& result);

    // Remove all records.
    void clear();
};

#endif