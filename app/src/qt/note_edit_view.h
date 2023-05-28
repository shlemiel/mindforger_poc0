/*
 note_edit_view.h     MindForger thinking notebook

 Copyright (C) 2016-2022 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef M8R_NOTE_EDIT_VIEW_H
#define M8R_NOTE_EDIT_VIEW_H

#include <QtWidgets>

#include "../../lib/src/model/note.h"

#include "note_editor_view.h"
#include "widgets/edit_name_panel.h"
#include "widgets/edit_buttons_panel.h"
#include "status_bar_view.h"

namespace m8r {

class NoteEditView : public QWidget
{
    Q_OBJECT

private:
    Note* currentNote;

    EditNamePanel* topNamePanel;
    NoteEditorView* noteEditor;
    EditButtonsPanel* bottomButtonsPanel;

public:
    explicit NoteEditView(QWidget* parent);
    NoteEditView(const NoteEditView&) = delete;
    NoteEditView(const NoteEditView&&) = delete;
    NoteEditView &operator=(const NoteEditView&) = delete;
    NoteEditView &operator=(const NoteEditView&&) = delete;
    ~NoteEditView();

    virtual void keyPressEvent(QKeyEvent* event) override;

    void setNoteEditDialog(NoteEditDialog* noteEditDialog) {
        bottomButtonsPanel->setNoteEditDialog(noteEditDialog);
    }
    void setNote(Note* note, std::string mdDescription) {
        currentNote = note;
        topNamePanel->setNote(note);
        bottomButtonsPanel->setNote(note);
        // TODO configuration
        noteEditor->enableSpellCheck(true);
        noteEditor->setPlainText(QString::fromStdString(mdDescription));
        if(note->getType()->getName() == "Diagram") {
            noteEditor->setEnabled(false);
        }
        else {
            noteEditor->setEnabled(true);
        }
    }
    void setDescription(const QString& description) {
        noteEditor->setPlainText(description);
    }
    void setEditorShowLineNumbers(bool show) { noteEditor->setShowLineNumbers(show); }
    void setStatusBar(const StatusBarView* statusBar) { noteEditor->setStatusBar(statusBar); }

    void focusName() const { topNamePanel->focusName(); }
    QString getName() const { return topNamePanel->getName(); }
    QString getDescription() const { return noteEditor->toPlainText(); }
    bool isDescriptionEmpty() const { return noteEditor->toPlainText().isEmpty(); }
    QString getSelectedText() const { return noteEditor->getSelectedText(); }
    NoteEditorView* getNoteEditor() const { return noteEditor; }
    EditButtonsPanel* getButtonsPanel() const { return bottomButtonsPanel; }

    void giveEditorFocus() { noteEditor->setFocus(); }

private slots:
    void slotOpenNotePropertiesEditor();
    void slotSaveNote();
    void slotCloseEditor();
    void slotSaveAndCloseEditor();

signals:
    void signalSaveAndCloseEditor();
    void signalCloseEditor();
    void signalSaveNote();
};

}
#endif // M8R_NOTE_EDIT_VIEW_H
