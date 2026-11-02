#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <string>

#include "Document.h"
#include "UndoStack.h"
#include "TextAlgorithms.h"
#include "FileManager.h"
#include "Menu.h"

class TextEditor {
private:
    Document document;
    UndoStack undoStack;
    Menu menu;

    std::string currentFilename;
    bool running;

    void handleChoice(int choice);

public:
    TextEditor();

    void run();

    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void exitEditor();

    void insertText();
    void deleteText();
    void insertLine();
    void deleteLine();

    void searchText();
    void replaceText();
    void replaceAllText();

    void undo();
};

#endif