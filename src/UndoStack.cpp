<<<<<<< HEAD

#include "UndoStack.h"
#include <limits>

// TextChangeArray

TextChangeArray::TextChangeArray()
    : data(nullptr), size(0), capacity(0)
{
}

TextChangeArray::~TextChangeArray()
{
    delete[] data;
}

// Copy Constructor (Deep Copy)
TextChangeArray::TextChangeArray(const TextChangeArray& other)
    : data(nullptr), size(0), capacity(0)
{
    if (other.capacity > 0)
    {
        TextChange* newData = new TextChange[other.capacity];

        try
        {
            for (int i = 0; i < other.size; i++)
            {
                newData[i] = other.data[i];
            }
        }
        catch (...)
        {
            delete[] newData;
            throw;
        }

        data = newData;
        size = other.size;
        capacity = other.capacity;
    }
}

// Assignment Operator
TextChangeArray& TextChangeArray::operator=(
    const TextChangeArray& other)
{
    if (this != &other)
    {
        TextChangeArray temp(other);

        TextChange* tempData = data;
        data = temp.data;
        temp.data = tempData;

        int tempSize = size;
        size = temp.size;
        temp.size = tempSize;

        int tempCapacity = capacity;
        capacity = temp.capacity;
        temp.capacity = tempCapacity;
    }

    return *this;
}

// Resize Dynamic Array
bool TextChangeArray::resize()
{
    int newCapacity;

    if (capacity == 0)
    {
        newCapacity = 4;
    }
    else
    {
        if (capacity > std::numeric_limits<int>::max() / 2)
        {
            return false;
        }

        newCapacity = capacity * 2;
    }

    TextChange* newData = nullptr;

    try
    {
        newData = new TextChange[newCapacity];

        for (int i = 0; i < size; i++)
        {
            newData[i] = data[i];
        }
    }
    catch (...)
    {
        delete[] newData;
        return false;
    }

    delete[] data;

    data = newData;
    capacity = newCapacity;

    return true;
}

// Add New Change
bool TextChangeArray::add(const TextChange& change)
{
    if (size == capacity)
    {
        if (!resize())
        {
            return false;
        }
    }

    try
    {
        data[size] = change;
    }
    catch (...)
    {
        return false;
    }

    size++;

    return true;
}

// Get Change at Specific Index
bool TextChangeArray::get(
    int index,
    TextChange& result) const
{
    if (index < 0 || index >= size)
    {
        return false;
    }

    try
    {
        result = data[index];
    }
    catch (...)
    {
        return false;
    }

    return true;
}

int TextChangeArray::getSize() const
{
    return size;
}

bool TextChangeArray::isEmpty() const
{
    return size == 0;
}

// Clear Array
void TextChangeArray::clear()
{
    for (int i = 0; i < size; i++)
    {
        data[i].lineNumber = 0;
        data[i].position = 0;
        data[i].oldText.clear();
        data[i].newText.clear();
    }

    size = 0;
}

// UndoRecord

UndoRecord::UndoRecord()
    : type(OperationType::INSERT_TEXT),
      lineNumber(0),
      position(0),
      oldText(""),
      newText(""),
      changes()
{
}

// UndoStack Node

UndoStack::UndoNode::UndoNode(
    const UndoRecord& undoRecord)
    : record(undoRecord), next(nullptr)
{
}

// UndoStack

UndoStack::UndoStack()
    : top(nullptr), size(0)
{
}

UndoStack::~UndoStack()
{
    clear();
}

// Check if Stack is Empty
bool UndoStack::isEmpty() const
{
    return top == nullptr;
}

// Get Stack Size
int UndoStack::getSize() const
{
    return size;
}

// Push New Record
void UndoStack::push(const UndoRecord& record)
{
    UndoNode* newNode = new UndoNode(record);

    newNode->next = top;
    top = newNode;

    size++;
}

// View Top Record
bool UndoStack::peek(UndoRecord& result) const
{
    if (isEmpty())
    {
        return false;
    }

    try
    {
        result = top->record;
    }
    catch (...)
    {
        return false;
    }

    return true;
}

// Remove Top Node
bool UndoStack::discardTop()
{
    if (isEmpty())
    {
        return false;
    }

    UndoNode* temp = top;

    top = top->next;

    delete temp;

    size--;

    return true;
}

// Retrieve and Remove Top Record
bool UndoStack::pop(UndoRecord& result)
{
    if (isEmpty())
    {
        return false;
    }

    if (!peek(result))
    {
        return false;
    }

    return discardTop();
}

// Clear Entire Stack
void UndoStack::clear()
{
    while (top != nullptr)
    {
        UndoNode* temp = top;

        top = top->next;

        delete temp;
    }

    size = 0;
}
=======

#include "UndoStack.h"
#include <limits>

// TextChangeArray

TextChangeArray::TextChangeArray()
    : data(nullptr), size(0), capacity(0)
{
}

TextChangeArray::~TextChangeArray()
{
    delete[] data;
}

// Copy Constructor (Deep Copy)
TextChangeArray::TextChangeArray(const TextChangeArray& other)
    : data(nullptr), size(0), capacity(0)
{
    if (other.capacity > 0)
    {
        TextChange* newData = new TextChange[other.capacity];

        try
        {
            for (int i = 0; i < other.size; i++)
            {
                newData[i] = other.data[i];
            }
        }
        catch (...)
        {
            delete[] newData;
            throw;
        }

        data = newData;
        size = other.size;
        capacity = other.capacity;
    }
}

// Assignment Operator
TextChangeArray& TextChangeArray::operator=(
    const TextChangeArray& other)
{
    if (this != &other)
    {
        TextChangeArray temp(other);

        TextChange* tempData = data;
        data = temp.data;
        temp.data = tempData;

        int tempSize = size;
        size = temp.size;
        temp.size = tempSize;

        int tempCapacity = capacity;
        capacity = temp.capacity;
        temp.capacity = tempCapacity;
    }

    return *this;
}

// Resize Dynamic Array
bool TextChangeArray::resize()
{
    int newCapacity;

    if (capacity == 0)
    {
        newCapacity = 4;
    }
    else
    {
        if (capacity > std::numeric_limits<int>::max() / 2)
        {
            return false;
        }

        newCapacity = capacity * 2;
    }

    TextChange* newData = nullptr;

    try
    {
        newData = new TextChange[newCapacity];

        for (int i = 0; i < size; i++)
        {
            newData[i] = data[i];
        }
    }
    catch (...)
    {
        delete[] newData;
        return false;
    }

    delete[] data;

    data = newData;
    capacity = newCapacity;

    return true;
}

// Add New Change
bool TextChangeArray::add(const TextChange& change)
{
    if (size == capacity)
    {
        if (!resize())
        {
            return false;
        }
    }

    try
    {
        data[size] = change;
    }
    catch (...)
    {
        return false;
    }

    size++;

    return true;
}

// Get Change at Specific Index
bool TextChangeArray::get(
    int index,
    TextChange& result) const
{
    if (index < 0 || index >= size)
    {
        return false;
    }

    try
    {
        result = data[index];
    }
    catch (...)
    {
        return false;
    }

    return true;
}

int TextChangeArray::getSize() const
{
    return size;
}

bool TextChangeArray::isEmpty() const
{
    return size == 0;
}

// Clear Array
void TextChangeArray::clear()
{
    for (int i = 0; i < size; i++)
    {
        data[i].lineNumber = 0;
        data[i].position = 0;
        data[i].oldText.clear();
        data[i].newText.clear();
    }

    size = 0;
}

// UndoRecord

UndoRecord::UndoRecord()
    : type(OperationType::INSERT_TEXT),
      lineNumber(0),
      position(0),
      oldText(""),
      newText(""),
      extraLines(0),
      changes()
{
}

// UndoStack Node

UndoStack::UndoNode::UndoNode(
    const UndoRecord& undoRecord)
    : record(undoRecord), next(nullptr)
{
}

// UndoStack

UndoStack::UndoStack()
    : top(nullptr), size(0)
{
}

UndoStack::~UndoStack()
{
    clear();
}

// Check if Stack is Empty
bool UndoStack::isEmpty() const
{
    return top == nullptr;
}

// Get Stack Size
int UndoStack::getSize() const
{
    return size;
}

// Push New Record
void UndoStack::push(const UndoRecord& record)
{
    UndoNode* newNode = new UndoNode(record);

    newNode->next = top;
    top = newNode;

    size++;
}

// View Top Record
bool UndoStack::peek(UndoRecord& result) const
{
    if (isEmpty())
    {
        return false;
    }

    try
    {
        result = top->record;
    }
    catch (...)
    {
        return false;
    }

    return true;
}

// Remove Top Node
bool UndoStack::discardTop()
{
    if (isEmpty())
    {
        return false;
    }

    UndoNode* temp = top;

    top = top->next;

    delete temp;

    size--;

    return true;
}

// Retrieve and Remove Top Record
bool UndoStack::pop(UndoRecord& result)
{
    if (isEmpty())
    {
        return false;
    }

    if (!peek(result))
    {
        return false;
    }

    return discardTop();
}

// Clear Entire Stack
void UndoStack::clear()
{
    while (top != nullptr)
    {
        UndoNode* temp = top;

        top = top->next;

        delete temp;
    }

    size = 0;
}
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
