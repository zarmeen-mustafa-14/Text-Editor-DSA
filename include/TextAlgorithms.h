#ifndef TEXTALGORITHMS_H
#define TEXTALGORITHMS_H

#include <string>
#include "Document.h"

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
        const std::string& pattern,
        MatchArray& results
    );

    static bool replaceFirst(
        Document& document,
        const std::string& pattern,
        const std::string& replacement,
        MatchPosition& changedPosition,
        std::string& originalText
    );

    static bool replaceAll(
        Document& document,
        const std::string& pattern,
        const std::string& replacement,
        int& replacementCount
    );
};

#endif