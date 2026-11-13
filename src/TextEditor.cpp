#include "TextEditor.h"

#include <sstream>
#include <string>

TextEditor::TextEditor()
    : document(), undoStack(), menu(), currentFilename(), running(true)
{
}

// Main application loop. Menu input failure (for example, EOF) exits cleanly.
void TextEditor::run()
{
    running = true;

    while (running)
    {
        menu.displayMainMenu();

        int choice = 0;
        if (!menu.getChoice(choice))
        {
            menu.displayMessage("Input ended. Exiting editor.");
            running = false;
            break;
        }

        handleChoice(choice);
    }
}

// NOTE: This switch assumes the menu choices listed below. If Menu.cpp uses
// different numbers, update this mapping to match it.
void TextEditor::handleChoice(int choice)
{
    switch (choice)
    {
    case 1:
        newDocument();
        break;
    case 2:
        openDocument();
        break;
    case 3:
        saveDocument();
        break;
    case 4:
        saveDocumentAs();
        break;
    case 5:
        insertText();
        break;
    case 6:
        deleteText();
        break;
    case 7:
        insertLine();
        break;
    case 8:
        deleteLine();
        break;
    case 9:
        searchText();
        break;
    case 10:
        replaceText();
        break;
    case 11:
        replaceAllText();
        break;
    case 12:
        undo();
        break;
    case 13:
    {
        std::ostringstream output;
        if (document.isEmpty())
        {
            output << "Document is empty.\n";
        }
        else
        {
            for (int lineNumber = 1; lineNumber <= document.getLineCount(); ++lineNumber)
            {
                std::string line;
                if (document.getLine(lineNumber, line))
                {
                    output << lineNumber << ". " << line << '\n';
                }
            }
        }
        menu.displayDocument(output.str());
        menu.displayDocumentStatistics(
            document.getLineCount(), document.getCharacterCount());
        break;
    }
    case 14:
        exitEditor();
        break;
    default:
        menu.displayError("Invalid menu choice. Please select a listed option.");
        break;
    }
}

// Starts a blank document and clears undo history.
void TextEditor::newDocument()
{
    document.clear();
    undoStack.clear();
    currentFilename.clear();
    menu.displayMessage("New document created.");
}

// Loads into a temporary document through FileManager. The current document
// and undo history are changed only after a successful open.
void TextEditor::openDocument()
{
    std::string filename;
    if (!menu.getString("Enter filename to open: ", filename))
    {
        menu.displayError("Filename input was cancelled.");
        return;
    }

    if (filename.empty())
    {
        menu.displayError("Filename cannot be empty.");
        return;
    }

    if (!FileManager::openFile(filename, document))
    {
        menu.displayError("Could not open the file. The current document was preserved.");
        return;
    }

    currentFilename = filename;
    undoStack.clear();
    menu.displayMessage("File opened successfully: " + currentFilename);
}

// Saves to the current filename, or asks for a filename if this is a new doc.
void TextEditor::saveDocument()
{
    if (currentFilename.empty())
    {
        saveDocumentAs();
        return;
    }

    if (!FileManager::saveFile(currentFilename, document))
    {
        menu.displayError("Could not save the document.");
        return;
    }

    menu.displayMessage("Document saved: " + currentFilename);
}
