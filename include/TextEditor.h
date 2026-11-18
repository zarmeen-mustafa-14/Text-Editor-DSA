#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <string>

#include "Document.h"
#include "FileManager.h"
#include "Menu.h"
#include "TextAlgorithms.h"
#include "UndoStack.h"

// Coordinates the terminal interface, document, file operations, algorithms,
// and linked-list undo history. Document line numbers are 1-based; character
// positions within a line are 0-based.
class TextEditor
{
private:
    Document document;
    UndoStack undoStack;
    Menu menu;

    std::string currentFilename;
    bool running;

    void handleChoice(int choice);

public:
    TextEditor();

    // Starts the interactive menu loop.
    void run();

    // Document lifecycle and file operations.
    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void exitEditor();

    // Editing operations.
    void insertText();
    void deleteText();
    void insertLine();
    void deleteLine();

    // Search and replacement operations.
    void searchText();
    void replaceText();
    void replaceAllText();

    // Reverses the latest successful editing operation.
    void undo();
};

#endif // TEXTEDITOR_H
