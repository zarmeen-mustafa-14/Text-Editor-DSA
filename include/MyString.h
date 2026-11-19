#ifndef MYSTRING_H
#define MYSTRING_H

#include <iostream>
#include <stdexcept>   // std::istream / std::ostream (no <string> needed)

class MyString
{
private:
    char*  buffer;      // null-terminated character array
    int    len;         // number of characters (excluding '\0')
    int    cap;         // allocated capacity (including room for '\0')

    // --- internal helpers (manual, no <cstring>) ---
    int    strleng(const char* s) const;                 // strlen
    void   strcopy(char* dst, const char* src) const;    // strcpy
    int    strcomp(const char* a, const char* b) const;  // strcmp
    void   ensureCapacity(int needed);

public:
    // --- constructors / destructor ---
    MyString();
    MyString(const char* text);
    MyString(const MyString& other);
    ~MyString();

    // --- assignment ---
    MyString& operator=(const MyString& other);
    MyString& operator=(const char* text);

    // --- capacity / state ---
    int  length()  const;      // strleng
    int  size()    const;      // alias for length
    bool empty()   const;
    void clear();              // reset to empty

    // --- element access ---
    char&       operator[](int index);
    const char& operator[](int index) const;

    // --- modifiers ---
    void append(const MyString& other);                         // strcat
    void append(const char* text);
    void append(const MyString& other, int start, int count);   // append slice
    void insert(int position, const MyString& text);            // strinsert
    void insert(int position, const char* text);
    void erase(int position, int length);                       // strerase
    void swap(MyString& other);                                 // strswap
    MyString subpart(int position, int length) const;           // substr
    MyString subpart(int position) const;                       // substr to end

    // --- comparison ---
    bool operator==(const MyString& other) const;
    bool operator!=(const MyString& other) const;
    bool operator<(const MyString& other)  const;

    // --- conversion ---
    const char* c_str() const;

    // --- stream I/O ---
    friend std::ostream& operator<<(std::ostream& os, const MyString& s);
    friend std::istream& operator>>(std::istream& is, MyString& s);
};

// --- free helpers ---
std::ostream& operator<<(std::ostream& os, const MyString& s);
std::istream& operator>>(std::istream& is, MyString& s);

// getline equivalent (reads until '\n', discards the newline)
// A '\r' directly before the '\n' (Windows line ending) is removed.
// NUL bytes cannot be stored in a C-style buffer; they are dropped and, if
// sawNul is given, *sawNul is set so callers can reject binary input.
std::istream& getLine(std::istream& is, MyString& s, bool* sawNul = nullptr);

// to_string equivalent for integers (used in TextEditor)
MyString intToStr(int value);

#endif
