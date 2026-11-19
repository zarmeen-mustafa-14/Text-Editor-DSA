#include "TextEditor.h"

#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace
{
    bool splitsUtf8Character(const MyString& line, int position)
    {
        if (position <= 0 || position >= line.length()) return false;
        const unsigned char byte = static_cast<unsigned char>(line[position]);
        return (byte & 0xC0) == 0x80;
    }

    void rollbackExtendedInsert(Document& document, int lineNumber,
                               int position, int insertedLength,
                               int originalLineCount)
    {
        MyString ignored;
        document.deleteText(lineNumber, position, insertedLength, ignored);
        while (document.getLineCount() > originalLineCount)
            document.deleteLine(document.getLineCount());
    }
}

TextEditor::TextEditor()
    : document(),
      undoStack(),
      menu(),
      currentFilename(),
      modified(false),
      running(true)
{
}

void TextEditor::run()
{
    running = true;

    while (running)
    {
        menu.displayMainMenu(
            currentFilename,
            modified,
            document.getLineCount(),
            document.getWordCount()
        );

        int choice = 0;
        if (!menu.getChoice(choice))
        {
            handleInputEnded();
            break;
        }

        handleMainChoice(choice);
    }
}

void TextEditor::handleInputEnded()
{
    if (modified)
    {
        menu.displayMessage(
            "Input ended. Exiting editor (unsaved changes were not saved).");
    }
    else
    {
        menu.displayMessage("Input ended. Exiting editor.");
    }

    running = false;
}

void TextEditor::handleMainChoice(int choice)
{
    switch (choice)
    {
    case 1: fileMenu(); break;
    case 2: editMenu(); break;
    case 3: findMenu(); break;
    case 4: viewMenu(); break;
    case 5: menu.displayHelp(); break;
    case 0: exitEditor(); break;
    default:
        menu.displayError("Invalid menu choice. Enter a number from the menu.");
        break;
    }
}


bool TextEditor::confirmDiscardChanges()
{
    if (!modified) return true;
    bool answer = false;
    if (!menu.askYesNo("You have unsaved changes. Discard them?", answer))
        return false;
    return answer;
}

void TextEditor::fileMenu()
{
    while (running)
    {
        menu.displayFileMenu(currentFilename, modified);
        int choice = 0;
        if (!menu.getChoice(choice)) { handleInputEnded(); return; }
        switch (choice)
        {
        case 1: newDocument(); break;
        case 2: openDocument(); break;
        case 3: saveDocument(); break;
        case 4: saveDocumentAs(); break;
        case 5: deleteFile(); break;
        case 6: closeDocument(); break;
        case 0: return;
        default: menu.displayError("Invalid menu choice."); break;
        }
    }
}

void TextEditor::editMenu()
{
    while (running)
    {
        menu.displayEditMenu(undoStack.getSize());
        int choice = 0;
        if (!menu.getChoice(choice)) { handleInputEnded(); return; }
        switch (choice)
        {
        case 1: insertText(); break;
        case 2: deleteText(); break;
        case 3: insertLine(); break;
        case 4: deleteLine(); break;
        case 5: undo(); break;
        case 6: saveDocument(); break;
        case 0: return;
        default: menu.displayError("Invalid menu choice."); break;
        }
    }
}

void TextEditor::findMenu()
{
    while (running)
    {
        menu.displayFindMenu();
        int choice = 0;
        if (!menu.getChoice(choice)) { handleInputEnded(); return; }
        switch (choice)
        {
        case 1: searchText(); break;
        case 2: replaceText(); break;
        case 3: replaceAllText(); break;
        case 0: return;
        default: menu.displayError("Invalid menu choice."); break;
        }
    }
}

void TextEditor::viewMenu()
{
    while (running)
    {
        menu.displayViewMenu();
        int choice = 0;
        if (!menu.getChoice(choice)) { handleInputEnded(); return; }
        switch (choice)
        {
        case 1: showDocument(); break;
        case 0: return;
        default: menu.displayError("Invalid menu choice."); break;
        }
    }
}

void TextEditor::showDocument()
{
    MyString output;
    if (document.isEmpty())
    {
        output.append("Document is empty.\n");
    }
    else
    {
        for (int i = 1; i <= document.getLineCount(); ++i)
        {
            MyString line;
            if (document.getLine(i, line))
            {
                output.append(intToStr(i));
                output.append(". ");
                output.append(line);
                output.append("\n");
            }
        }
    }
    menu.displayDocument(output);
    menu.displayDocumentStatistics(
        document.getLineCount(), document.getWordCount(), document.getCharacterCount());
}

void TextEditor::deleteFile()
{
    MyString filename;
    if (!menu.getString("Enter filename to delete: ", filename) || filename.empty())
    {
        menu.displayError("Input cancelled or filename is empty.");
        return;
    }
    bool confirm = false;
    if (!menu.askYesNo("Delete this file permanently?", confirm) || !confirm)
    {
        menu.displayMessage("File deletion cancelled.");
        return;
    }
    if (!FileManager::deleteFile(filename))
        menu.displayError("Could not delete file.");
    else
        menu.displayMessage("File deleted.");
}

void TextEditor::closeDocument()
{
    if (!confirmDiscardChanges())
    {
        menu.displayMessage("Close cancelled.");
        return;
    }
    document.clear();
    undoStack.clear();
    currentFilename.clear();
    modified = false;
    menu.displayMessage("Document closed.");
}

void TextEditor::newDocument()
{
    if (!confirmDiscardChanges())
    {
        menu.displayMessage("New document cancelled.");
        return;
    }

    MyString name;
    if (!menu.getString("Enter a name for the new document: ", name))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    name = FileManager::normalizeFilename(name);

    if (name.empty())
    {
        menu.displayError("Document name cannot be empty.");
        return;
    }

    if (!FileManager::isValidFilename(name))
    {
        menu.displayError(
            "Invalid document name. Avoid control characters and < > \" | ? *");
        return;
    }

    if (FileManager::fileExists(name))
    {
        menu.displayError(
            "A file with that name already exists. Use Open, or choose another name.");
        return;
    }

    document.clear();
    undoStack.clear();
    currentFilename.swap(name);
    modified = false;

    MyString message("New document \"");
    message.append(currentFilename);
    message.append("\" created. Use Save to write it to disk.");
    menu.displayMessage(message);
}

void TextEditor::openDocument()
{
    MyString filename;

    if (!menu.getString("Enter filename to open: ", filename))
    {
        menu.displayError("Input cancelled.");
        return;
    }
    if (filename.empty())
    {
        menu.displayError("Filename cannot be empty.");
        return;
    }

    try
    {
        if (!FileManager::openFile(filename, document))
        {
            menu.displayError("Could not open file. Current document preserved.");
            return;
        }

        // Only commit editor metadata after the file has loaded successfully.
        currentFilename.swap(filename);
        undoStack.clear();
        menu.displayMessage("File opened successfully.");
    }
    catch (const std::exception& error)
    {
        MyString message("Could not open file: ");
        message.append(error.what());
        menu.displayError(message);
    }
}

void TextEditor::saveDocument()
{
    if (currentFilename.empty())
    {
        saveDocumentAs();
        return;
    }

    try
    {
        if (!FileManager::saveFile(currentFilename, document))
        {
            menu.displayError("Could not save document.");
            return;
        }
        menu.displayMessage("Document saved successfully.");
    }
    catch (const std::exception& error)
    {
        MyString message("Could not save document: ");
        message.append(error.what());
        menu.displayError(message);
    }
}

void TextEditor::saveDocumentAs()
{
    MyString filename;
    if (!menu.getString("Enter filename to save as: ", filename))
    {
        menu.displayError("Input cancelled.");
        return;
    }
    if (filename.empty())
    {
        menu.displayError("Filename cannot be empty.");
        return;
    }

    try
    {
        if (!FileManager::saveFile(filename, document))
        {
            menu.displayError("Could not save document.");
            return;
        }
        currentFilename.swap(filename);
        menu.displayMessage("Document saved successfully.");
    }
    catch (const std::exception& error)
    {
        MyString message("Could not save document: ");
        message.append(error.what());
        menu.displayError(message);
    }
}

void TextEditor::exitEditor()
{
    if (modified)
    {
        bool save = false;
        if (!menu.askYesNo("You have unsaved changes. Save before exiting?", save))
        {
            running = false;
            menu.displayMessage("Input ended. Exiting without saving.");
            return;
        }

        if (save)
        {
            saveDocument();

            if (modified)
            {
                menu.displayMessage("Exit cancelled because the save did not complete.");
                return;
            }
        }
        else
        {
            bool discard = false;
            if (!menu.askYesNo("Exit and discard unsaved changes?", discard) ||
                !discard)
            {
                menu.displayMessage("Exit cancelled.");
                return;
            }
        }
    }

    running = false;
    menu.displayMessage("Exiting Text Editor.");
}

void TextEditor::insertText()
{
    int lineNumber = 0;
    int position = 0;
    MyString text;

    if (!menu.getInteger("Enter line number (1 or more, missing lines are created): ", lineNumber) ||
        !menu.getInteger("Enter character position (0 = start, past the end pads with spaces): ", position) ||
        !menu.getString("Enter text to insert: ", text))
    {
        menu.displayError("Invalid or cancelled input. Whole numbers are required.");
        return;
    }
    if (text.empty())
    {
        menu.displayError("Nothing to insert.");
        return;
    }

    MyString existing;
    if (document.getLine(lineNumber, existing) && splitsUtf8Character(existing, position))
    {
        menu.displayError("That position is inside a multi-byte character (positions are byte offsets). Choose a neighbouring position.");
        return;
    }

    const int originalLineCount = document.getLineCount();
    int linesAdded = 0;
    int insertedAt = 0;
    MyString insertedText;
    try
    {
        if (!document.insertTextExtending(lineNumber, position, text,
                                         linesAdded, insertedAt, insertedText))
        {
            menu.displayError("Invalid insertion position or padding limit exceeded.");
            return;
        }

        UndoRecord record;
        record.type = OperationType::INSERT_TEXT;
        record.lineNumber = lineNumber;
        record.position = insertedAt;
        record.newText = insertedText;
        record.extraLines = linesAdded;

        try
        {
            undoStack.push(record);
        }
        catch (...)
        {
            rollbackExtendedInsert(document, lineNumber, insertedAt,
                                  insertedText.length(), originalLineCount);
            throw;
        }
    }
    catch (const std::exception& error)
    {
        MyString message("Insert failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    modified = true;
    MyString message("Text inserted successfully.");
    const int paddingSpaces = insertedText.length() - text.length();
    if (linesAdded > 0 || paddingSpaces > 0)
    {
        message.append(" (created ");
        message.append(intToStr(linesAdded));
        message.append(" line(s), added ");
        message.append(intToStr(paddingSpaces));
        message.append(" padding space(s))");
    }
    menu.displayMessage(message);
}

void TextEditor::deleteText()
{
    int lineNumber = 0;
    int position = 0;
    int length = 0;

    if (!menu.getInteger("Enter line number: ", lineNumber) ||
        !menu.getInteger("Enter starting position (0 = start of line): ", position) ||
        !menu.getInteger("Enter length to delete: ", length))
    {
        menu.displayError("Invalid or cancelled input. Whole numbers are required.");
        return;
    }

    if (length <= 0)
    {
        menu.displayError("Length must be positive.");
        return;
    }

    MyString line;
    if (!document.getLine(lineNumber, line) ||
        position < 0 || position >= line.length())
    {
        menu.displayError("Invalid line or character position.");
        return;
    }

    const int actualLength = (length > static_cast<int>(line.size()) - position)
        ? static_cast<int>(line.size()) - position : length;

    UndoRecord record;
    record.type = OperationType::DELETE_TEXT;
    record.lineNumber = lineNumber;
    record.position = position;
    record.oldText = line.subpart(position, actualLength);

    bool pushed = false;

    try
    {
        undoStack.push(record);
        pushed = true;

        MyString deletedText;
        if (!document.deleteText(
                lineNumber, position, actualLength, deletedText))
        {
            undoStack.discardTop();
            menu.displayError("Text deletion failed.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed)
        {
            undoStack.discardTop();
        }

        MyString message("Delete failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    modified = true;
    menu.displayMessage("Text deleted successfully.");
}

void TextEditor::insertLine()
{
    int lineNumber = 0;
    MyString text;
    if (!menu.getInteger("Enter line number for insertion: ", lineNumber) ||
        !menu.getString("Enter line text: ", text))
    {
        menu.displayError("Invalid or cancelled input. A whole number is required.");
        return;
    }
    if (lineNumber < 1)
    {
        menu.displayError("Line number must be 1 or greater.");
        return;
    }

    const int originalLineCount = document.getLineCount();
    int linesAdded = 0;
    try
    {
        if (!document.insertLineExtending(lineNumber, text, linesAdded))
        {
            menu.displayError("Line number is too far beyond the end of the document.");
            return;
        }
        UndoRecord record;
        record.type = OperationType::INSERT_LINE;
        record.lineNumber = lineNumber;
        record.newText = text;
        record.extraLines = linesAdded;
        try
        {
            undoStack.push(record);
        }
        catch (...)
        {
            while (document.getLineCount() > originalLineCount)
                document.deleteLine(document.getLineCount());
            throw;
        }
    }
    catch (const std::exception& error)
    {
        MyString message("Line insertion failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }
    modified = true;
    menu.displayMessage("Line inserted successfully.");
}

void TextEditor::deleteLine()
{
    int lineNumber = 0;

    if (!menu.getInteger("Enter line number to delete: ", lineNumber))
    {
        menu.displayError("Invalid or cancelled input. A whole number is required.");
        return;
    }

    MyString oldLine;
    if (!document.getLine(lineNumber, oldLine))
    {
        menu.displayError("Invalid line number.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::DELETE_LINE;
    record.lineNumber = lineNumber;
    record.oldText = oldLine;

    bool pushed = false;

    try
    {
        undoStack.push(record);
        pushed = true;

        if (!document.deleteLine(lineNumber))
        {
            undoStack.discardTop();
            menu.displayError("Line deletion failed.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed)
        {
            undoStack.discardTop();
        }

        MyString message("Line deletion failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    modified = true;
    menu.displayMessage("Line deleted successfully.");
}

void TextEditor::searchText()
{
    MyString pattern;

    if (!menu.getString("Enter text to search: ", pattern))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    MatchArray matches;
    if (!TextAlgorithms::findAll(document, pattern, matches))
    {
        menu.displayMessage("Search failed or no searchable document exists.");
        return;
    }

    if (matches.getSize() == 0)
    {
        menu.displayMessage("No matches found.");
        return;
    }

    MyString output("Matches found: ");
    output.append(intToStr(matches.getSize()));
    output.append("\n");

    for (int i = 0; i < matches.getSize(); ++i)
    {
        MatchPosition match{};
        if (matches.get(i, match))
        {
            output.append("Line ");
            output.append(intToStr(match.lineNumber));
            output.append(", Position ");
            output.append(intToStr(match.characterPosition));
            output.append("\n");
        }
    }

    menu.displayMessage(output);
}

void TextEditor::replaceText()
{
    MyString pattern;
    MyString replacement;

    if (!menu.getString("Enter text to find: ", pattern) ||
        !menu.getString("Enter replacement text: ", replacement))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    // Locate and prepare the complete undo record before changing the document.
    MatchArray matches;
    if (!TextAlgorithms::findAll(document, pattern, matches) ||
        matches.getSize() == 0)
    {
        menu.displayMessage("No replacement made.");
        return;
    }

    MatchPosition first{};
    if (!matches.get(0, first))
    {
        menu.displayError("Could not prepare replacement.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::REPLACE_TEXT;
    record.lineNumber = first.lineNumber;
    record.position = first.characterPosition;
    record.oldText = pattern;
    record.newText = replacement;

    bool pushed = false;

    try
    {
        undoStack.push(record);
        pushed = true;

        MatchPosition changedPosition{};
        MyString originalText;

        if (!TextAlgorithms::replaceFirst(
                document, pattern, replacement,
                changedPosition, originalText))
        {
            undoStack.discardTop();
            menu.displayMessage("No replacement made.");
            return;
        }

    }
    catch (const std::exception& error)
    {
        if (pushed)
        {
            undoStack.discardTop();
        }

        MyString message("Replacement failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    modified = true;
    menu.displayMessage("First occurrence replaced.");
}

void TextEditor::replaceAllText()
{
    MyString pattern;
    MyString replacement;

    if (!menu.getString("Enter text to find: ", pattern) ||
        !menu.getString("Enter replacement text: ", replacement))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    if (pattern == replacement)
    {
        menu.displayMessage("Replacement text is identical. No replacements made.");
        return;
    }

    TextChangeArray changes;
    int replacementCount = 0;

    if (!TextAlgorithms::replaceAll(
            document, pattern, replacement, changes, replacementCount))
    {
        menu.displayMessage("No replacements made.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::REPLACE_ALL;
    record.changes = changes;

    try
    {
        undoStack.push(record);
    }
    catch (const std::exception& error)
    {
        // Best-effort rollback in reverse order. Positions stored in changes
        // refer to the original text; prior replacements shift later positions.
        bool rollbackSucceeded = true;
        try
        {
            for (int i = changes.getSize() - 1; i >= 0; --i)
            {
                TextChange change{};
                if (!changes.get(i, change))
                {
                    rollbackSucceeded = false;
                    break;
                }

                long long position = change.position;
                for (int j = 0; j < i; ++j)
                {
                    TextChange earlier{};
                    if (!changes.get(j, earlier))
                    {
                        rollbackSucceeded = false;
                        break;
                    }
                    if (earlier.lineNumber == change.lineNumber)
                    {
                        position += static_cast<long long>(earlier.newText.size()) -
                                    static_cast<long long>(earlier.oldText.size());
                    }
                }
                if (!rollbackSucceeded || position < 0 ||
                    position > static_cast<long long>(std::numeric_limits<int>::max()))
                {
                    rollbackSucceeded = false;
                    break;
                }

                MyString line;
                if (!document.getLine(change.lineNumber, line) ||
                    position + change.newText.length() > line.length())
                {
                    rollbackSucceeded = false;
                    break;
                }

                MyString restored;
                restored.append(line, 0, static_cast<int>(position));
                restored.append(change.oldText);
                restored.append(line, static_cast<int>(position + change.newText.length()),
                    line.length() - static_cast<int>(position + change.newText.length()));
                if (!document.updateLine(change.lineNumber, restored))
                {
                    rollbackSucceeded = false;
                    break;
                }
            }
        }
        catch (...)
        {
            rollbackSucceeded = false;
        }

        MyString errorMessage;
        if (rollbackSucceeded)
        {
            errorMessage = "Replacement was rolled back because undo recording failed: ";
            errorMessage.append(error.what());
        }
        else
        {
            errorMessage = "Critical error: undo recording failed and rollback was incomplete.";
        }
        menu.displayError(errorMessage);
        return;
    }

    MyString result("Total replacements: ");
    result.append(intToStr(replacementCount));
    menu.displayMessage(result);
}

void TextEditor::undo()
{
    if (undoStack.isEmpty())
    {
        menu.displayMessage("Nothing to undo.");
        return;
    }

    UndoRecord record;
    if (!undoStack.peek(record))
    {
        menu.displayError("Could not read undo record.");
        return;
    }

    bool success = false;

    try
    {
        switch (record.type)
        {
        case OperationType::INSERT_TEXT:
        {
            MyString removed;
            success = document.deleteText(
                record.lineNumber, record.position,
                record.newText.length(), removed);
            if (success)
            {
                for (int i = 0; i < record.extraLines; ++i)
                {
                    success = document.deleteLine(record.lineNumber);
                    if (!success) break;
                }
            }
            break;
        }

        case OperationType::DELETE_TEXT:
            success = document.insertText(
                record.lineNumber,
                record.position,
                record.oldText
            );
            break;

        case OperationType::REPLACE_TEXT:
        {
            MyString line;
            if (document.getLine(record.lineNumber, line) && record.position >= 0 &&
                record.position <= line.length() &&
                record.newText.length() <= line.length() - record.position)
            {
                MyString restored;
                restored.append(line, 0, record.position);
                restored.append(record.oldText);
                restored.append(line, record.position + record.newText.length(),
                    line.length() - record.position - record.newText.length());
                success = document.updateLine(record.lineNumber, restored);
            }
            break;
        }
        case OperationType::REPLACE_ALL:
        {
            success = true;
            // Reverse order is essential. The stored positions are in the
            // original text, so account for all earlier replacements' deltas.
            for (int i = record.changes.getSize() - 1; i >= 0; --i)
            {
                TextChange change{};
                if (!record.changes.get(i, change))
                {
                    success = false;
                    break;
                }

                long long position = change.position;
                for (int j = 0; j < i; ++j)
                {
                    TextChange earlier{};
                    if (!record.changes.get(j, earlier))
                    {
                        success = false;
                        break;
                    }
                    if (earlier.lineNumber == change.lineNumber)
                    {
                        position += static_cast<long long>(earlier.newText.size()) -
                                    static_cast<long long>(earlier.oldText.size());
                    }
                }
                if (!success || position < 0 ||
                    position > static_cast<long long>(std::numeric_limits<int>::max()))
                {
                    success = false;
                    break;
                }

                MyString line;
                if (!document.getLine(change.lineNumber, line) ||
                    position + change.newText.length() > line.length())
                {
                    success = false;
                    break;
                }

                MyString restored;
                restored.append(line, 0, static_cast<int>(position));
                restored.append(change.oldText);
                restored.append(line, static_cast<int>(position + change.newText.length()),
                    line.length() - static_cast<int>(position + change.newText.length()));
                if (!document.updateLine(change.lineNumber, restored))
                {
                    success = false;
                }
            }
            break;
        }
        case OperationType::INSERT_LINE:
            success = true;
            for (int i = 0; i < record.extraLines; ++i)
            {
                if (!document.deleteLine(record.lineNumber))
                {
                    success = false;
                    break;
                }
            }
            break;
        case OperationType::DELETE_LINE:
            success = document.insertLine(record.lineNumber, record.oldText);
            break;
        }
    }
    catch (const std::exception& error)
    {
        MyString message("Undo failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    if (!success)
    {
        menu.displayError("Undo failed. The undo record was retained.");
        return;
    }

    undoStack.discardTop();
    modified = true;
    menu.displayMessage("Undo completed successfully.");
}