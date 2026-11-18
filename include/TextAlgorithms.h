#ifndef TEXTALGORITHMS_H
#define TEXTALGORITHMS_H

#include "MyString.h"
#include "Document.h"
#include "UndoStack.h"

struct MatchPosition {
    int lineNumber;
    int characterPosition;
};

class MatchArray {
private:
    MatchPosition* data;
    int capacity;
    int size;

    bool resize();

public:
    MatchArray();
    ~MatchArray();

    MatchArray(const MatchArray&) = delete;
    MatchArray& operator=(const MatchArray&) = delete;

    bool add(const MatchPosition& position);
    int getSize() const;
    bool get(int index, MatchPosition& result) const;
    void clear();
};

class TextAlgorithms {
public:
    static bool findAll(
        const Document& document,
        const MyString& pattern,
        MatchArray& results
    );

    static bool replaceFirst(
        Document& document,
        const MyString& pattern,
        const MyString& replacement,
        MatchPosition& changedPosition,
        MyString& originalText
    );

    static bool replaceAll(
        Document& document,
        const MyString& pattern,
        const MyString& replacement,
        TextChangeArray& changes,
        int& replacementCount
    );
};

#endif