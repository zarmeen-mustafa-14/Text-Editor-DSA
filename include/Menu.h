#ifndef MENU_H
#define MENU_H

#include "MyString.h"

// Terminal user interface: every screen the editor shows, plus input helpers.
// The menu is hierarchical: a short main menu opens one sub-menu per topic
// (File, Edit, Find & Replace, View). Each sub-menu has 0 = Back.
class Menu
{
public:
    // ---- Menus -----------------------------------------------------------
    // Main menu. filename may be empty (untitled); modified marks unsaved
    // changes; lineCount / wordCount form the status line.
    void displayMainMenu(
        const MyString& filename,
        bool modified,
        int lineCount,
        int wordCount
    ) const;

    void displayFileMenu(
        const MyString& filename,
        bool modified
    ) const;

    // undoCount = number of steps that can still be undone.
    void displayEditMenu(
        int undoCount
    ) const;

    void displayFindMenu() const;

    void displayViewMenu() const;

    void displayHelp() const;

    // ---- Input -----------------------------------------------------------
    // Reads a menu choice. Text that is not a number gives choice -1
    // (handled as invalid). Returns false only when input has ended.
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

    // Asks a y/n question and repeats until the answer is valid.
    // Returns false only if input ended (EOF); answer is then false.
    bool askYesNo(
        const MyString& prompt,
        bool& answer
    ) const;

    // ---- Output ----------------------------------------------------------
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
        int wordCount,
        int characterCount
    ) const;
};

<<<<<<< HEAD
#endif
=======
#endif
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
