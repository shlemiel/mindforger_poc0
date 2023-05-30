/*
 note_view_presenter.cpp     MindForger thinking notebook

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
#include "note_view_presenter.h"

#include "look_n_feel.h"

using namespace std;

namespace m8r {

NoteViewPresenter::NoteViewPresenter(NoteView* view, OrlojPresenter* orloj)
    : config(Configuration::getInstance())
{
    this->mind = orloj->getMind();

    // IMPORTANT: pre-allocate string using reserve() to ensure good append performance
    html = string{};
    html.reserve(10000);

    this->view = view;
    this->orloj = orloj;
    this->model = new NoteViewModel();
    this->view->setModel(this->model);

    connect(view, &NoteView::signalReceiveText, this, &NoteViewPresenter::slotReceiveText);
    connect(view, &NoteView::signalExit, this, &NoteViewPresenter::slotExit);

    this->markdownRepresentation
        = orloj->getMainPresenter()->getMarkdownRepresentation();
    this->htmlRepresentation
        = orloj->getMainPresenter()->getHtmlRepresentation();

    this->currentNote = nullptr;

#ifdef MF_QT_WEB_ENGINE
    QObject::connect(
        view->getViever()->getPage(), SIGNAL(signalLinkClicked(QUrl)),
        this, SLOT(slotLinkClicked(QUrl)));
#else
    QObject::connect(
        view->getViever(), SIGNAL(linkClicked(QUrl)),
        this, SLOT(slotLinkClicked(QUrl)));
#endif
    QObject::connect(
        view->getViever(), SIGNAL(signalMouseDoubleClickEvent()),
        this, SLOT(slotEditNoteDoubleClick()));
    QObject::connect(
        view, SIGNAL(signalOpenEditor()),
        this, SLOT(slotEditNote()));
    QObject::connect(
        view->getViever(), SIGNAL(signalFromViewNoteToOutlines()),
        orloj, SLOT(slotShowOutlines()));
}

NoteViewPresenter::~NoteViewPresenter()
{
    if(markdownRepresentation) delete markdownRepresentation;
    if(htmlRepresentation) delete htmlRepresentation;
}

void NoteViewPresenter::slotReceiveText(const QString& text)
{
    orloj->getNoteEdit()->getView()->setDescription(text);
}

void NoteViewPresenter::slotExit()
{
    orloj->getNoteEdit()->slotSaveAndCloseEditor();
}

void NoteViewPresenter::refreshLivePreview()
{
    MF_DEBUG("Refreshing N HTML preview from editor: " << this->currentNote->getName() << endl);

    // N w/ current editor text w/o saving it
    Note auxNote{currentNote->getType(), currentNote->getOutline()};
    auxNote.setName(orloj->getNoteEdit()->getView()->getName().toStdString());

    QString description = orloj->getNoteEdit()->getView()->getDescription();

    if(auxNote.getType()->getName() == "Diagram") {
        QString diagramText =
            "<!DOCTYPE html>"
            "<html>"
            "<body>"
            "<script src='qrc:///qtwebchannel/qwebchannel.js'></script>"
            "<iframe src='qrc:///drawio/resources/deps/drawio/src/main/webapp/index.html?embed=1&ui=dark&spin=0&proto=json' style='position:fixed; top:0; left:0; bottom:0; right:0; width:100%; height:100%; border:none; margin:0; padding:0; overflow:hidden; z-index:999999;'></iframe>"
            "<script>"
            "var jshelper = null;"
            "var xmlData = atob('$$xmlData$$');"
            "new QWebChannel(qt.webChannelTransport, function (channel) {"
            "jshelper = channel.objects.jshelper;"
            "});"
            "var iframe = document.querySelector('iframe');"
            "var drawIoWindow = iframe.contentWindow;"
            "window.onbeforeunload = () => { iframe.parentNode.removeChild(iframe); };"
            "var receive = function(evt) {"
            "  if (evt.data.length > 0 && evt.source == drawIoWindow) {"
            "    var msg = JSON.parse(evt.data);"
            "    if (msg.event == 'init') {"
            "      drawIoWindow.postMessage(JSON.stringify({action: 'load', xmlpng: xmlData}), '*');"
            "    }"
            "    else if (msg.event == 'save') {"
            "      drawIoWindow.postMessage(JSON.stringify({action: 'export', format: 'xmlpng', spinKey: 'saving'}), '*');"
            "    }"
            "    else if (msg.event == 'export') {"
            "      xmlData = msg.data;"
            "      jshelper.receiveText(xmlData);"
            "    }"
            "    else if (msg.event == 'exit') {"
            "      jshelper.exit();"
            "    }"
            "  }"
            "};"
            "window.addEventListener('message', receive);"
            "</script>"
            "</body>"
            "</html>";

        diagramText.replace("$$xmlData$$", description.toUtf8().toBase64());
        view->setHtml(diagramText);
    }
    else {
        string s{description.toStdString()};
        vector<string*> d{};
        orloj->getMainPresenter()->getMarkdownRepresentation()->description(&s, d);
        auxNote.setDescription(d);

        double yScrollPct{0};
        QScrollBar* scrollbar = orloj->getNoteEdit()->getView()->getNoteEditor()->verticalScrollBar();
#if defined(_WIN32) || defined(__APPLE__)
        // WebEngine: scroll to same pct view
        if(scrollbar) {
            if(scrollbar->maximum()) {
                // scroll: QWebEngine API for scrolling is not available - JavaScript must be used instead (via signal)
                yScrollPct =
                    static_cast<double>(scrollbar->value())
                    /
                    (static_cast<double>(scrollbar->maximum())/100.0);
            }
        }
#endif

        // refresh N HTML view (autolinking intentionally disabled)
        htmlRepresentation->to(
            &auxNote,
            &html,
            false,
            static_cast<int>(yScrollPct)
            );

        view->setHtml(QString::fromStdString(html));


// IMPROVE share code between O header and N
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(MF_QT_WEB_ENGINE)
        // WebView: scroll to same pct view
        if(scrollbar) {
            if(scrollbar->maximum()) {
                yScrollPct =
                    static_cast<double>(scrollbar->value())
                    /
                    (static_cast<double>(scrollbar->maximum())/100.0);
                // scroll
                QWebFrame* webFrame=view->getViever()->page()->mainFrame();
                webFrame->setScrollPosition(QPoint(
                    0,
                    static_cast<int>((webFrame->scrollBarMaximum(Qt::Orientation::Vertical)/100.0)*yScrollPct)));
            }
        }
#endif
    }
}

// IMPROVE first decorate MD with HTML colors > then MD to HTML conversion
void NoteViewPresenter::refresh(Note* note)
{
    note->makeRead();
    this->currentNote = note;

    if(this->currentNote->getType()->getName() == "Diagram") {
        QString diagramText =
            "<!DOCTYPE html>"
            "<html>"
            "<body>"
            "<script src='qrc:///qtwebchannel/qwebchannel.js'></script>"
            "<iframe src='qrc:///drawio/resources/deps/drawio/src/main/webapp/index.html?embed=1&lightbox=1&ui=dark&spin=0&proto=json&noExitBtn=1' style='position:fixed; top:0; left:0; bottom:0; right:0; width:100%; height:100%; border:none; margin:0; padding:0; overflow:hidden; z-index:999999;'></iframe>"
            "<script>"
            "var jshelper = null;"
            "var xmlData = atob('$$xmlData$$');"
            "new QWebChannel(qt.webChannelTransport, function (channel) {"
            "jshelper = channel.objects.jshelper;"
            "});"
            "var iframe = document.querySelector('iframe');"
            "var drawIoWindow = iframe.contentWindow;"
            "window.onbeforeunload = () => { iframe.parentNode.removeChild(iframe); };"
            "var receive = function(evt) {"
            "  if (evt.data.length > 0 && evt.source == drawIoWindow) {"
            "    var msg = JSON.parse(evt.data);"
            "    if (msg.event == 'init') {"
            "      drawIoWindow.postMessage(JSON.stringify({action: 'load', xmlpng: xmlData}), '*');"
            "    }"
            "    else if (msg.event == 'save') {"
            "      drawIoWindow.postMessage(JSON.stringify({action: 'export', format: 'xmlpng', spinKey: 'saving'}), '*');"
            "    }"
            "    else if (msg.event == 'export') {"
            "      xmlData = msg.data;"
            "      jshelper.receiveText(xmlData);"
            "    }"
            "  }"
            "};"
            "window.addEventListener('message', receive);"
            "</script>"
            "</body>"
            "</html>";

        diagramText.replace("$$xmlData$$", QString::fromStdString(this->currentNote->getDescriptionAsString()).toUtf8().toBase64());
        view->setHtml(diagramText);
    }
    else {
        // HTML
        htmlRepresentation->to(note, &html, Configuration::getInstance().isAutolinking());
        view->setHtml(QString::fromStdString(html));
    }

    // leaderboard
    mind->associate();
}

void NoteViewPresenter::slotLinkClicked(const QUrl& url)
{
    orloj->getMainPresenter()->handleNoteViewLinkClicked(url);
}

void NoteViewPresenter::slotEditNote()
{    
    orloj->showFacetNoteEdit(this->currentNote);
}

void NoteViewPresenter::slotEditNoteDoubleClick()
{
    if(orloj->getMainPresenter()->getConfiguration().isUiDoubleClickNoteViewToEdit()) {
        slotEditNote();
    }
}

void NoteViewPresenter::slotRefreshLeaderboardByValue(AssociatedNotes* associations)
{
    if(orloj->isFacetActive(OrlojPresenterFacets::FACET_VIEW_NOTE)
         ||
       orloj->isFacetActive(OrlojPresenterFacets::FACET_EDIT_NOTE)) // leaderboard for edit @ view
    {
        orloj->getOutlineView()->getAssocLeaderboard()->refresh(associations);
        delete associations;
    }
}

} // m8r namespace
