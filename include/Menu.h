#ifndef MENU_H
#define MENU_H

#include "MyString.h"

class Menu
{
public:
    // Main menu.
    void displayMainMenu() const;

    bool getChoice(int& choice) const;

    // Command-specific input.
    bool getInteger(
        const MyString& prompt,
        int& value
    ) const;

    bool getString(
        const MyString& prompt,
        MyString& value
    ) const;

    // Output.
    void displayMessage(
        const MyString& message
    ) const;

    void displayError(
        const MyString& message
    ) const;

    void displayDocument(
        const MyString& text
    ) const;

    void displayDocumentStatistics(
        int lineCount,
        int characterCount
    ) const;
};

#endif