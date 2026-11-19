#include "Menu.h"

#include <iostream>
#include <limits>

using namespace std;

namespace
{
    bool isBlank(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    // Strict integer parser: optional spaces, optional sign, digits only,
    // no trailing garbage, no overflow.
    bool parseInt(const MyString& text, int& value)
    {
        int i = 0;
        const int n = text.length();

        while (i < n && isBlank(text[i])) ++i;

        bool negative = false;
        if (i < n && (text[i] == '+' || text[i] == '-'))
        {
            negative = (text[i] == '-');
            ++i;
        }

        if (i >= n || text[i] < '0' || text[i] > '9')
        {
            return false;
        }

        long long result = 0;
        while (i < n && text[i] >= '0' && text[i] <= '9')
        {
            result = result * 10 + (text[i] - '0');
            if (result > 2147483648LL)
            {
                return false;           // too large for int
            }
            ++i;
        }

        while (i < n && isBlank(text[i])) ++i;
        if (i != n)
        {
            return false;               // trailing garbage such as "3abc"
        }

        if (negative) result = -result;
        if (result > 2147483647LL || result < -2147483647LL - 1)
        {
            return false;
        }

        value = static_cast<int>(result);
        return true;
    }
    const int MENU_WIDTH = 56;

    void printRule(char ch)
    {
        for (int i = 0; i < MENU_WIDTH; ++i) cout << ch;
        cout << '\n';
    }

    // Prints "=====  TITLE  =====" centred in the menu width.
    void printTitle(const char* title)
    {
        const MyString text(title);
        const int inner = text.length() + 2;            // one space each side
        const int left  = (MENU_WIDTH - inner) / 2;
        const int right = MENU_WIDTH - inner - left;

        cout << '\n';
        for (int i = 0; i < left;  ++i) cout << '=';
        cout << ' ' << text << ' ';
        for (int i = 0; i < right; ++i) cout << '=';
        cout << '\n';
    }

    void printFileStatus(const MyString& filename, bool modified)
    {
        cout << " File : ";
        if (filename.empty()) cout << "(untitled)";
        else                  cout << filename;
        if (modified)         cout << "  [modified]";
        cout << '\n';
    }
}

// ---------------------------------------------------------------------------
// Menus
// ---------------------------------------------------------------------------

// Displays the main menu.
void Menu::displayMainMenu(
    const MyString& filename,
    bool modified,
    int lineCount,
    int wordCount) const
{
    printTitle("TEXT EDITOR");
    printFileStatus(filename, modified);
    cout << " Lines: " << lineCount << "    Words: " << wordCount << '\n';
    printRule('-');
    cout << "  1. File             new, open, save, delete, close\n";
    cout << "  2. Edit             text, lines, undo, save\n";
    cout << "  3. Find & Replace   search and replace text\n";
    cout << "  4. View             show document and statistics\n";
    cout << "  5. Help\n";
    cout << "  0. Exit\n";
    printRule('-');
    cout << "Enter your choice: ";
}

void Menu::displayFileMenu(const MyString& filename, bool modified) const
{
    printTitle("FILE");
    printFileStatus(filename, modified);
    printRule('-');
    cout << "  1. New Document\n";
    cout << "  2. Open Document\n";
    cout << "  3. Save\n";
    cout << "  4. Save As...\n";
    cout << "  5. Delete a File\n";
    cout << "  6. Close Document\n";
    cout << "  0. Back to Main Menu\n";
    printRule('-');
    cout << "Enter your choice: ";
}


void Menu::displayEditMenu(int undoCount) const
{
    printTitle("EDIT");
    cout << " Text\n";
    cout << "  1. Insert Text\n";
    cout << "  2. Delete Text\n";
    cout << " Lines\n";
    cout << "  3. Insert Line\n";
    cout << "  4. Delete Line\n";
    cout << " History\n";

    cout << "  5. Undo";
    if (undoCount > 0) cout << "  (" << undoCount << " step(s) available)";
    else               cout << "  (nothing to undo)";
    cout << '\n';
    cout << " Document\n";
    cout << "  6. Save\n";

    printRule('-');
    cout << "  0. Back to Main Menu\n";
    cout << "Enter your choice: ";
}

void Menu::displayFindMenu() const
{
    printTitle("FIND & REPLACE");
    cout << "  1. Find Text (list every match)\n";
    cout << "  2. Replace First Occurrence\n";
    cout << "  3. Replace All Occurrences\n";
    printRule('-');
    cout << "  0. Back to Main Menu\n";
    cout << "Enter your choice: ";
}

void Menu::displayViewMenu() const
{
    printTitle("VIEW");
    cout << "  1. Show Document & Statistics\n";
    printRule('-');
    cout << "  0. Back to Main Menu\n";
    cout << "Enter your choice: ";
}

void Menu::displayHelp() const
{
    printTitle("HELP");
    cout << " Navigation\n";
    cout << "  - Type the number of an item and press Enter.\n";
    cout << "  - 0 always goes back (or exits from the main menu).\n";
    cout << "  - A sub-menu stays open, so you can repeat actions.\n";
    cout << '\n';
    cout << " Numbers\n";
    cout << "  - Line numbers start at 1.\n";
    cout << "  - Character positions start at 0 (0 = start of line).\n";
    cout << "  - Positions count bytes, so avoid the middle of\n";
    cout << "    multi-byte (non-English) characters.\n";
    cout << '\n';
    cout << " Good to know\n";
    cout << "  - Insert Text / Insert Line create missing lines and\n";
    cout << "    pad with spaces if you go past the end.\n";
    cout << "  - Undo goes back one step at a time.\n";
    cout << "  - Undo history is cleared when you open, create or\n";
    cout << "    close a file.\n";
    cout << "  - Search is case-sensitive and works line by line.\n";
    cout << "  - Delete a File removes it from disk permanently.\n";
    printRule('-');
}

// Reads the menu choice. Text that is not a number gives choice -1, because
// 0 now means Back / Exit. Returns false only when input has ended.
bool Menu::getChoice(int& choice) const
{
    MyString line;
    if (!getLine(cin, line))
    {
        return false;
    }

    if (!parseInt(line, choice))
    {
        choice = -1;
    }

    return true;
}


// Reads an integer. Returns false on invalid input or end of input.
bool Menu::getInteger(const MyString& prompt, int& value) const
{
    cout << prompt;

    MyString line;
    if (!getLine(cin, line))
    {
        return false;
    }

    int parsed = 0;
    if (!parseInt(line, parsed))
    {
        return false;
    }

    value = parsed;
    return true;
}

// Reads a complete line of text using MyString.
bool Menu::getString(const MyString& prompt, MyString& value) const
{
    cout << prompt;

    return static_cast<bool>(getLine(cin, value));
}

// Asks a yes/no question.
bool Menu::askYesNo(const MyString& prompt, bool& answer) const
{
    answer = false;

    while (true)
    {
        cout << prompt << " (y/n): ";

        MyString line;
        if (!getLine(cin, line))
        {
            return false;
        }

        int start = 0;
        while (start < line.length() && isBlank(line[start])) ++start;

        if (start < line.length())
        {
            const char c = line[start];
            const bool restBlank = [&]()
            {
                for (int i = start + 1; i < line.length(); ++i)
                {
                    if (!isBlank(line[i])) return false;
                }
                return true;
            }();

            if (restBlank && (c == 'y' || c == 'Y'))
            {
                answer = true;
                return true;
            }

            if (restBlank && (c == 'n' || c == 'N'))
            {
                answer = false;
                return true;
            }
        }

        cout << "Please answer y or n.\n";
    }
}

// Displays a message.
void Menu::displayMessage(const MyString& message) const
{
    cout << "\n" << message << "\n";
}

// Displays an error message.
void Menu::displayError(const MyString& message) const
{
    cout << "\nError: " << message << "\n";
}

// Displays the document.
void Menu::displayDocument(const MyString& text) const
{
    cout << "\n========== DOCUMENT ==========\n";
    cout << text;
    cout << "==============================\n";
}

// Displays document statistics.
void Menu::displayDocumentStatistics(
    int lineCount,
    int wordCount,
    int characterCount) const
{
    cout << "\n========== STATISTICS ==========\n";
    cout << "Total Lines: " << lineCount << '\n';
    cout << "Total Words: " << wordCount << '\n';
    cout << "Total Characters: " << characterCount << '\n';
    cout << "================================\n";
}