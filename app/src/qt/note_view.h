/*
 note_view.h     MindForger thinking notebook

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
#ifndef M8RUI_NOTE_VIEW_H
#define M8RUI_NOTE_VIEW_H

#include "../../lib/src/debug.h"
#include "../../lib/src/model/note.h"
#include "../../lib/src/config/configuration.h"

#include "note_view_model.h"
#include "widgets/mf_widgets.h"
#include "widgets/view_to_edit_buttons_panel.h"

#include <QtWidgets>
#ifdef MF_QT_WEB_ENGINE
  #include <QWebChannel>
  #include <QWebEngineView>
  #include "web_engine_page_link_navigation_policy.h"
#else
  #include <QWebView>
#endif
#include <QUrl>

namespace m8r {

class NoteViewerView;
class JSHelper : public QObject
{
    Q_OBJECT

    QObject* o;

public:
    explicit JSHelper(QObject *o)
        : QObject(nullptr), o(o)
    {
    }

signals:
    /*!
        This signal is emitted from the C++ side and the text displayed on the HTML client side.
    */
    void sendText(const QString &text);

public slots:
    /*!
        This slot is invoked from the HTML client side and the text displayed on the server side.
    */
    void receiveText(const QString &text);
    void exit();
};

#ifdef MF_QT_WEB_ENGINE
class NoteViewerView: public QWebEngineView
#else
class NoteViewerView: public QWebView
#endif
{
    Q_OBJECT

public:
    NoteViewerView(QWidget* parent);
    NoteViewerView(const NoteViewerView&) = delete;
    NoteViewerView(const NoteViewerView&&) = delete;
    NoteViewerView &operator=(const NoteViewerView&) = delete;
    NoteViewerView &operator=(const NoteViewerView&&) = delete;

#ifdef MF_QT_WEB_ENGINE
    QWebEnginePage* getPage() const { return page(); }

protected:
    bool event(QEvent* evt) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

    JSHelper helper;

#else
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
#endif
    virtual void keyPressEvent(QKeyEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;

signals:
    void signalMouseDoubleClickEvent();
    void signalFromViewNoteToOutlines();
    void signalReceiveText(const QString& text);
    void signalExit();
};

class NoteView : public QWidget
{
    Q_OBJECT

private:
    NoteViewModel* noteModel;

    NoteViewerView* noteViewer;
    ViewToEditEditButtonsPanel* view2EditPanel;

public:
    NoteView(QWidget* parent);
    NoteView(const NoteView&) = delete;
    NoteView(const NoteView&&) = delete;
    NoteView&operator=(const NoteView&) = delete;
    NoteView&operator=(const NoteView&&) = delete;
    virtual ~NoteView() override;

    NoteViewerView* getViever() const { return noteViewer; }
    ViewToEditEditButtonsPanel* getButtonsPanel() const { return view2EditPanel; }

    void setModel(NoteViewModel* noteModel) { this->noteModel = noteModel; }
    void setZoomFactor(qreal factor) {
        noteViewer->setZoomFactor(factor);
    }
    void setHtml(const QString& html, const QUrl& baseUrl = QUrl("file://")) {
        noteViewer->setHtml(html, baseUrl);
    }
    void giveViewerFocus() {
        QMetaObject::invokeMethod(
            noteViewer, "setFocus",
            Qt::QueuedConnection
            /*, Q_ARG(char*, text) */);
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void slotOpenEditor();
    void slotReceiveText(const QString& text);
    void slotExit();

signals:
    void signalOpenEditor();
    void signalReceiveText(const QString& text);
    void signalExit();
};

}
#endif // M8RUI_NOTE_VIEW_H
