#include "Menu.h"

#include <iostream>
#include <limits>

using namespace std;

// Displays the main menu.
void Menu::displayMainMenu() const
{
    cout << "\n========== TEXT EDITOR ==========\n";
    cout << "1.  New Document\n";
    cout << "2.  Open Document\n";
    cout << "3.  Save Document\n";
    cout << "4.  Save As\n";
    cout << "5.  Insert Text\n";
    cout << "6.  Delete Text\n";
    cout << "7.  Insert Line\n";
    cout << "8.  Delete Line\n";
    cout << "9.  Search Text\n";
    cout << "10. Replace Text\n";
    cout << "11. Replace All\n";
    cout << "12. Undo\n";
    cout << "13. Display Document & Statistics\n";
    cout << "14. Exit\n";
    cout << "=================================\n";
    cout << "Enter your choice: ";
}

// Reads the menu choice.
bool Menu::getChoice(int& choice) const
{
    if (!(cin >> choice))
    {
        if (cin.eof())
            return false;

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        choice = 0;
        return true;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

// Reads an integer.
bool Menu::getInteger(const MyString& prompt, int& value) const
{
    cout << prompt;

    if (!(cin >> value))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

// Reads a complete line of text using MyString.
bool Menu::getString(const MyString& prompt, MyString& value) const
{
    cout << prompt;

    return static_cast<bool>(getLine(cin, value));
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
    int characterCount) const
{
    cout << "\n========== STATISTICS ==========\n";
    cout << "Total Lines: " << lineCount << '\n';
    cout << "Total Characters: " << characterCount << '\n';
    cout << "================================\n";
}