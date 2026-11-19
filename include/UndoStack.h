<<<<<<< HEAD

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "Mystring.h"

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

    MyString oldText;
    MyString newText;
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

    MyString oldText;
    MyString newText;

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

    void push(const UndoRecord& record);

    bool peek(UndoRecord& result) const;

    bool discardTop();

    bool pop(UndoRecord& result);

    void clear();
};

#endif
=======

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "MyString.h"

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

    MyString oldText;
    MyString newText;
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

    MyString oldText;
    MyString newText;

    // Number of lines created by this operation (INSERT_LINE, and INSERT_TEXT
    // when it had to create missing lines). Undo removes exactly these lines.
    int extraLines;

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

    void push(const UndoRecord& record);

    bool peek(UndoRecord& result) const;

    bool discardTop();

    bool pop(UndoRecord& result);

    void clear();
};

#endif
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
