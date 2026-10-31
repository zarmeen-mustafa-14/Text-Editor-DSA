#ifndef MENU_H
#define MENU_H

#include <string>

class Menu
{
public:
    // Main menu.
    void displayMainMenu() const;

    bool getChoice(int& choice) const;

    // Command-specific input.
    bool getInteger(
        const std::string& prompt,
        int& value
    ) const;

    bool getString(
        const std::string& prompt,
        std::string& value
    ) const;

    // Output.
    void displayMessage(
        const std::string& message
    ) const;

    void displayError(
        const std::string& message
    ) const;

    void displayDocument(
        const std::string& text
    ) const;

    void displayDocumentStatistics(
        int lineCount,
        int characterCount
    ) const;
};

#endif   