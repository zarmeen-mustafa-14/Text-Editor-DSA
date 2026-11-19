#include "TextEditor.h"

#include <exception>
#include <iostream>
#include <limits>

namespace
{
    // Replaces a range in a line using only MyString operations.
    bool replaceRange(
        const MyString& line,
        int position,
        int removeLength,
        const MyString& insertedText,
        MyString& result)
    {
        if (position < 0 || removeLength < 0 ||
            position > line.length() ||
            removeLength > line.length() - position)
        {
            return false;
        }

        MyString rebuilt;
        rebuilt.append(line.subpart(0, position));
        rebuilt.append(insertedText);
        rebuilt.append(line.subpart(position + removeLength));

        result.swap(rebuilt);
        return true;
    }

    // Calculates the position of a replacement after earlier replacements
    // on the same line have changed its length.
    bool adjustedPosition(
        const TextChangeArray& changes,
        int changeIndex,
        const TextChange& change,
        long long& position)
    {
        position = change.position;

        for (int i = 0; i < changeIndex; ++i)
        {
            TextChange earlier;
            if (!changes.get(i, earlier))
            {
                return false;
            }

            if (earlier.lineNumber == change.lineNumber)
            {
                position += static_cast<long long>(earlier.newText.length()) -
                            static_cast<long long>(earlier.oldText.length());
            }
        }

        return position >= 0 &&
               position <= static_cast<long long>(std::numeric_limits<int>::max());
    }
}

TextEditor::TextEditor()
    : document(),
      undoStack(),
      menu(),
      currentFilename(),
      running(true)
{
}

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

void TextEditor::handleChoice(int choice)
{
    switch (choice)
    {
    case 1: newDocument(); break;
    case 2: openDocument(); break;
    case 3: saveDocument(); break;
    case 4: saveDocumentAs(); break;
    case 5: insertText(); break;
    case 6: deleteText(); break;
    case 7: insertLine(); break;
    case 8: deleteLine(); break;
    case 9: searchText(); break;
    case 10: replaceText(); break;
    case 11: replaceAllText(); break;
    case 12: undo(); break;
    case 13:
    {
        MyString output;

        if (document.isEmpty())
        {
            output = "Document is empty.\n";
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
            document.getLineCount(),
            document.getCharacterCount()
        );
        break;
    }
    case 14: exitEditor(); break;
    default: menu.displayError("Invalid menu choice."); break;
    }
}

void TextEditor::newDocument()
{
    document.clear();
    undoStack.clear();
    currentFilename.clear();
    menu.displayMessage("New document created.");
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

    if (!FileManager::openFile(filename, document))
    {
        menu.displayError("Could not open file. Current document preserved.");
        return;
    }

    currentFilename.swap(filename);
    undoStack.clear();
    menu.displayMessage("File opened successfully.");
}

void TextEditor::saveDocument()
{
    if (currentFilename.empty())
    {
        saveDocumentAs();
        return;
    }

    if (!FileManager::saveFile(currentFilename, document))
    {
        menu.displayError("Could not save document.");
        return;
    }

    menu.displayMessage("Document saved successfully.");
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

    if (!FileManager::saveFile(filename, document))
    {
        menu.displayError("Could not save document.");
        return;
    }

    currentFilename.swap(filename);
    menu.displayMessage("Document saved successfully.");
}

void TextEditor::exitEditor()
{
    running = false;
    menu.displayMessage("Exiting Text Editor.");
}

void TextEditor::insertText()
{
    int lineNumber = 0;
    int position = 0;
    MyString text;

    if (!menu.getInteger("Enter line number: ", lineNumber) ||
        !menu.getInteger("Enter character position: ", position) ||
        !menu.getString("Enter text to insert: ", text))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (text.empty())
    {
        menu.displayError("Nothing to insert.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::INSERT_TEXT;
    record.lineNumber = lineNumber;
    record.position = position;
    record.newText = text;

    bool pushed = false;

    try
    {
        undoStack.push(record);
        pushed = true;

        if (!document.insertText(lineNumber, position, text))
        {
            undoStack.discardTop();
            menu.displayError("Invalid line or character position.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed)
        {
            undoStack.discardTop();
        }

        MyString message("Insert failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    menu.displayMessage("Text inserted successfully.");
}

void TextEditor::deleteText()
{
    int lineNumber = 0;
    int position = 0;
    int length = 0;

    if (!menu.getInteger("Enter line number: ", lineNumber) ||
        !menu.getInteger("Enter starting position: ", position) ||
        !menu.getInteger("Enter length to delete: ", length))
    {
        menu.displayError("Input cancelled.");
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

    const int availableLength = line.length() - position;
    const int actualLength = (length > availableLength)
        ? availableLength : length;

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

    menu.displayMessage("Text deleted successfully.");
}

void TextEditor::insertLine()
{
    int lineNumber = 0;
    MyString text;

    if (!menu.getInteger("Enter line number for insertion: ", lineNumber) ||
        !menu.getString("Enter line text: ", text))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::INSERT_LINE;
    record.lineNumber = lineNumber;

    bool pushed = false;

    try
    {
        undoStack.push(record);
        pushed = true;

        if (!document.insertLine(lineNumber, text))
        {
            undoStack.discardTop();
            menu.displayError(
                "Invalid line number. Valid range is 1 through line count + 1."
            );
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed)
        {
            undoStack.discardTop();
        }

        MyString message("Line insertion failed: ");
        message.append(error.what());
        menu.displayError(message);
        return;
    }

    menu.displayMessage("Line inserted successfully.");
}

void TextEditor::deleteLine()
{
    int lineNumber = 0;

    if (!menu.getInteger("Enter line number to delete: ", lineNumber))
    {
        menu.displayError("Input cancelled.");
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
        // Restore in reverse order because the saved positions refer to the
        // original line, before earlier replacements shifted its contents.
        bool rollbackSucceeded = true;

        for (int i = changes.getSize() - 1; i >= 0; --i)
        {
            TextChange change;
            if (!changes.get(i, change))
            {
                rollbackSucceeded = false;
                break;
            }

            long long position = 0;
            if (!adjustedPosition(changes, i, change, position))
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
            if (!replaceRange(
                    line, static_cast<int>(position),
                    change.newText.length(), change.oldText, restored) ||
                !document.updateLine(change.lineNumber, restored))
            {
                rollbackSucceeded = false;
                break;
            }
        }

        if (rollbackSucceeded)
        {
            MyString message(
                "Replacement was rolled back because undo recording failed: "
            );
            message.append(error.what());
            menu.displayError(message);
        }
        else
        {
            menu.displayError(
                "Critical error: undo recording failed and rollback was incomplete."
            );
        }

        return;
    }

    MyString message("Total replacements: ");
    message.append(intToStr(replacementCount));
    menu.displayMessage(message);
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
                record.lineNumber,
                record.position,
                record.newText.length(),
                removed
            );
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
            if (document.getLine(record.lineNumber, line))
            {
                MyString restored;
                success = replaceRange(
                    line,
                    record.position,
                    record.newText.length(),
                    record.oldText,
                    restored
                );

                if (success)
                {
                    success = document.updateLine(record.lineNumber, restored);
                }
            }
            break;
        }

        case OperationType::REPLACE_ALL:
        {
            success = true;

            for (int i = record.changes.getSize() - 1; i >= 0; --i)
            {
                TextChange change;
                if (!record.changes.get(i, change))
                {
                    success = false;
                    break;
                }

                long long position = 0;
                if (!adjustedPosition(
                        record.changes, i, change, position))
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
                if (!replaceRange(
                        line,
                        static_cast<int>(position),
                        change.newText.length(),
                        change.oldText,
                        restored) ||
                    !document.updateLine(change.lineNumber, restored))
                {
                    success = false;
                    break;
                }
            }
            break;
        }

        case OperationType::INSERT_LINE:
            success = document.deleteLine(record.lineNumber);
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
    menu.displayMessage("Undo completed successfully.");
}
