#include "MyString.h"

// ============================================================================
// Internal helpers — all manual, no <cstring> functions
// ============================================================================

int MyString::strleng(const char* s) const
{
    if (s == nullptr) return 0;
    int n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

void MyString::strcopy(char* dst, const char* src) const
{
    int i = 0;
    while (src[i] != '\0')
    {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

int MyString::strcomp(const char* a, const char* b) const
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        // compare as unsigned char, same as the real strcmp
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (x < y) return -1;
        if (x > y) return  1;
        ++i;
    }
    if (a[i] == '\0' && b[i] == '\0') return 0;
    return (a[i] == '\0') ? -1 : 1;
}

void MyString::ensureCapacity(int needed)
{
    if (needed <= cap) return;

    if (needed < 0) throw std::length_error("MyString: text too long");

    int newCap = (cap > 0) ? cap : 8;
    while (newCap < needed)
    {
        if (newCap > 1073741823) newCap = needed;   // avoid int overflow
        else                     newCap *= 2;
    }

    char* newBuf = new char[newCap];
    if (len > 0) strcopy(newBuf, buffer);
    else         newBuf[0] = '\0';

    delete[] buffer;
    buffer = newBuf;
    cap    = newCap;
}

// ============================================================================
// Constructors / destructor
// ============================================================================

MyString::MyString()
    : buffer(nullptr), len(0), cap(0)
{
    cap = 8;
    buffer = new char[cap];
    buffer[0] = '\0';
}

MyString::MyString(const char* text)
    : buffer(nullptr), len(0), cap(0)
{
    len = strleng(text);
    cap = (len + 1 > 8) ? (len + 1) : 8;
    buffer = new char[cap];
    if (text != nullptr) strcopy(buffer, text);
    else                 buffer[0] = '\0';
}

MyString::MyString(const MyString& other)
    : buffer(nullptr), len(0), cap(0)
{
    len = other.len;
    cap = (len + 1 > 8) ? (len + 1) : 8;
    buffer = new char[cap];
    strcopy(buffer, other.buffer);
}

MyString::~MyString()
{
    delete[] buffer;
    buffer = nullptr;
    len = 0;
    cap = 0;
}

// ============================================================================
// Assignment
// ============================================================================

MyString& MyString::operator=(const MyString& other)
{
    if (this != &other)
    {
        if (cap < other.len + 1)
        {
            // allocate first so *this stays valid if new[] throws
            int newCap = (other.len + 1 > 8) ? (other.len + 1) : 8;
            char* newBuf = new char[newCap];
            delete[] buffer;
            buffer = newBuf;
            cap = newCap;
        }
        len = other.len;
        strcopy(buffer, other.buffer);
    }
    return *this;
}

MyString& MyString::operator=(const char* text)
{
    int n = strleng(text);
    if (cap < n + 1)
    {
        int newCap = (n + 1 > 8) ? (n + 1) : 8;
        char* newBuf = new char[newCap];
        if (text != nullptr) strcopy(newBuf, text);
        else                 newBuf[0] = '\0';
        delete[] buffer;
        buffer = newBuf;
        cap = newCap;
        len = n;
        return *this;
    }
    // use memmove-safe copy in case text points inside our own buffer
    if (text != nullptr)
    {
        if (text >= buffer && text < buffer + cap)
        {
            MyString tmp(text);
            len = tmp.len;
            strcopy(buffer, tmp.buffer);
            return *this;
        }
        strcopy(buffer, text);
    }
    else buffer[0] = '\0';
    len = n;
    return *this;
}

// ============================================================================
// Capacity / state
// ============================================================================

int  MyString::length() const { return len; }
int  MyString::size()   const { return len; }
bool MyString::empty()  const { return len == 0; }

void MyString::clear()
{
    len = 0;
    if (buffer != nullptr) buffer[0] = '\0';
}

// ============================================================================
// Element access
// ============================================================================

char& MyString::operator[](int index)
{
    return buffer[index];        // caller must keep index in range
}

const char& MyString::operator[](int index) const
{
    return buffer[index];
}

// ============================================================================
// Modifiers
// ============================================================================

void MyString::append(const MyString& other)
{
    if (other.len == 0) return;
    if (other.len > 2147483646 - len) throw std::length_error("MyString: text too long");
    if (&other == this)
    {
        MyString copy(other);
        append(copy);
        return;
    }
    ensureCapacity(len + other.len + 1);
    int i = 0;
    while (i < other.len)
    {
        buffer[len + i] = other.buffer[i];
        ++i;
    }
    len += other.len;
    buffer[len] = '\0';
}

void MyString::append(const char* text)
{
    if (text == nullptr) return;
    int n = strleng(text);
    if (n == 0) return;
    if (n > 2147483646 - len) throw std::length_error("MyString: text too long");
    if (text >= buffer && text < buffer + cap)
    {
        MyString copy(text);
        append(copy);
        return;
    }
    ensureCapacity(len + n + 1);
    int i = 0;
    while (i < n)
    {
        buffer[len + i] = text[i];
        ++i;
    }
    len += n;
    buffer[len] = '\0';
}

void MyString::append(const MyString& other, int start, int count)
{
    if (start < 0 || start >= other.len || count <= 0) return;
    int avail = other.len - start;
    if (count > avail) count = avail;
    if (count > 2147483646 - len) throw std::length_error("MyString: text too long");
    if (&other == this)
    {
        MyString copy(other);
        append(copy, start, count);
        return;
    }

    ensureCapacity(len + count + 1);
    int i = 0;
    while (i < count)
    {
        buffer[len + i] = other.buffer[start + i];
        ++i;
    }
    len += count;
    buffer[len] = '\0';
}

void MyString::insert(int position, const MyString& text)
{
    // inserting a string into itself: copy first, because ensureCapacity
    // may reallocate and leave text.c_str() dangling
    if (&text == this)
    {
        MyString copy(text);
        insert(position, copy.c_str());
        return;
    }
    insert(position, text.c_str());
}

void MyString::insert(int position, const char* text)
{
    if (position < 0 || position > len) return;
    int n = strleng(text);
    if (n == 0) return;
    if (n > 2147483646 - len) throw std::length_error("MyString: text too long");
    if (text >= buffer && text < buffer + cap)
    {
        MyString copy(text);
        insert(position, copy.c_str());
        return;
    }

    ensureCapacity(len + n + 1);

    // shift tail (including '\0') to the right
    for (int i = len; i >= position; --i)
        buffer[i + n] = buffer[i];

    // copy new text in
    for (int i = 0; i < n; ++i)
        buffer[position + i] = text[i];

    len += n;
    buffer[len] = '\0';
}

void MyString::erase(int position, int length)
{
    if (position < 0 || position >= len || length <= 0) return;
    int actual = length;
    if (position + actual > len) actual = len - position;

    // shift tail (including '\0') to the left
    for (int i = position; i + actual <= len; ++i)
        buffer[i] = buffer[i + actual];

    len -= actual;
    buffer[len] = '\0';
}

void MyString::swap(MyString& other)
{
    char* tBuf = buffer; buffer = other.buffer; other.buffer = tBuf;
    int   tLen = len;    len    = other.len;    other.len    = tLen;
    int   tCap = cap;    cap    = other.cap;    other.cap    = tCap;
}

MyString MyString::subpart(int position, int length) const
{
    MyString result;
    if (position < 0 || position >= len || length <= 0) return result;

    int actual = length;
    if (position + actual > len) actual = len - position;

    result.ensureCapacity(actual + 1);
    for (int i = 0; i < actual; ++i)
        result.buffer[i] = buffer[position + i];
    result.len = actual;
    result.buffer[result.len] = '\0';
    return result;
}

MyString MyString::subpart(int position) const
{
    return subpart(position, len - position);
}

// ============================================================================
// Comparison
// ============================================================================

bool MyString::operator==(const MyString& other) const
{
    return strcomp(buffer, other.buffer) == 0;
}

bool MyString::operator!=(const MyString& other) const
{
    return strcomp(buffer, other.buffer) != 0;
}

bool MyString::operator<(const MyString& other) const
{
    return strcomp(buffer, other.buffer) < 0;
}

// ============================================================================
// Conversion
// ============================================================================

const char* MyString::c_str() const
{
    return buffer;
}

// ============================================================================
// Stream I/O
// ============================================================================

std::ostream& operator<<(std::ostream& os, const MyString& s)
{
    if (s.buffer != nullptr) os << s.buffer;
    return os;
}

std::istream& operator>>(std::istream& is, MyString& s)
{
    // skip leading whitespace
    char c = '\0';
    while (is.get(c))
    {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    }
    if (!is) return is;

    s.clear();

    // read until next whitespace.
    // NOTE: a lone char's address is NOT a null-terminated string,
    // so build a proper 2-char array before appending.
    char tmp[2] = { '\0', '\0' };
    bool hitEnd = false;
    for (;;)
    {
        tmp[0] = c;
        s.append(tmp);
        if (!is.get(c)) { hitEnd = true; break; }
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') break;
    }

    if (hitEnd)
        is.clear(std::ios::eofbit);   // we did read a word, so don't report failure
    else
        is.putback(c);                // leave the terminator for the next read

    return is;
}

std::istream& getLine(std::istream& is, MyString& s, bool* sawNul)
{
    s.clear();
    if (sawNul != nullptr) *sawNul = false;

    char c;
    char tmp[2] = { '\0', '\0' };
    bool any = false;
    bool endedWithNewline = false;

    while (is.get(c))
    {
        any = true;
        if (c == '\n') { endedWithNewline = true; break; }
        if (c == '\0')
        {
            if (sawNul != nullptr) *sawNul = true;
            continue;                       // cannot be stored in a C buffer
        }
        tmp[0] = c;
        s.append(tmp);
    }

    // Strip the '\r' of a CRLF line ending (only when it is the last char).
    if (s.length() > 0 && s[s.length() - 1] == '\r' &&
        (endedWithNewline || is.eof()))
    {
        s.erase(s.length() - 1, 1);
    }

    if (!any) is.setstate(std::ios::failbit);
    else if (!endedWithNewline && is.eof()) is.clear(std::ios::eofbit);  // last line had no '\n'
    return is;
}

// ============================================================================
// intToStr — replaces std::to_string
// ============================================================================

MyString intToStr(int value)
{
    if (value == 0)
    {
        MyString result("0");
        return result;
    }

    bool negative = (value < 0);
    long long v = value;           // avoids overflow on INT_MIN
    if (v < 0) v = -v;

    char digits[32];
    int  n = 0;
    while (v > 0)
    {
        digits[n++] = static_cast<char>('0' + (v % 10));
        v /= 10;
    }

    char out[34];
    int  k = 0;
    if (negative) out[k++] = '-';
    for (int i = n - 1; i >= 0; --i)
        out[k++] = digits[i];
    out[k] = '\0';

    MyString result(out);
    return result;
}
