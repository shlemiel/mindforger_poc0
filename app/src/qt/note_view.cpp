/*
 note_view.cpp     MindForger thinking notebook

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
#include "note_view.h"
namespace m8r {

using namespace std;

void JSHelper::exit()
{
    emit dynamic_cast<NoteViewerView*>(o)->signalExit();
}

void JSHelper::receiveText(const QString& text)
{
    emit dynamic_cast<NoteViewerView*>(o)->signalReceiveText(text);
}

NoteViewerView::NoteViewerView(QWidget *parent)
#ifdef MF_QT_WEB_ENGINE
    : QWebEngineView(parent), helper(this)
#else
    : QWebView(parent)
#endif
{
#ifdef MF_QT_WEB_ENGINE
    // ensure that link clicks are not handled, but delegated to MF using linkClicked signal
    WebEnginePageLinkNavigationPolicy* p = new WebEnginePageLinkNavigationPolicy{this};
    setPage(p);

    QWebChannel* channel = new QWebChannel(p);
    p->setWebChannel(channel);
    channel->registerObject(QStringLiteral("jshelper"), &this->helper);
#else
    // ensure that link clicks are not handled, but delegated to MF using linkClicked signal
    page()->setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy::DelegateAllLinks);
#endif

    // zoom
    setZoomFactor(static_cast<qreal>(Configuration::getInstance().getUiHtmlZoomFactor()));
}

#ifdef MF_QT_WEB_ENGINE

bool NoteViewerView::event(QEvent* event)
{
    // INSTALL event filter to every child - child polished event is received 1+x for every child
    if (event->type() == QEvent::ChildPolished) {
        QChildEvent* childEvent = static_cast<QChildEvent*>(event);
        QObject* childObj = childEvent->child();
        if (childObj) {
            childObj->installEventFilter(this);
        }
    }

    return QWebEngineView::event(event);
}

bool NoteViewerView::eventFilter(QObject* obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonDblClick) {
        // double click to Note view opens Note editor
        emit signalMouseDoubleClickEvent();
        event->accept();
        return true;
    }

    return QWebEngineView::eventFilter(obj, event);
}

void NoteViewerView::keyPressEvent(QKeyEvent* event)
{
    if(event->modifiers() & Qt::AltModifier){
        if(event->key()==Qt::Key_Left) {
            emit signalFromViewNoteToOutlines();
        }
    }

    QWebEngineView::keyPressEvent(event);
}

void NoteViewerView::wheelEvent(QWheelEvent* event)
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
        if(!event->angleDelta().isNull()) {
            if(event->angleDelta().ry()>0) {
                Configuration::getInstance().incUiHtmlZoom();
            } else {
                Configuration::getInstance().decUiHtmlZoom();
            }
            setZoomFactor(static_cast<qreal>(Configuration::getInstance().getUiHtmlZoomFactor()));
            event->accept();
            return;
        }
    }

    // do NOT forward event to the parent as it would cause loop
}

#else

void NoteViewerView::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // double click to Note view opens Note editor
    emit signalMouseDoubleClickEvent();
}

void NoteViewerView::keyPressEvent(QKeyEvent* event)
{
    if(event->modifiers() & Qt::AltModifier){
        if(event->key()==Qt::Key_Left) {
            emit signalFromViewNoteToOutlines();
        }
    }

    QWebView::keyPressEvent(event);
}

void NoteViewerView::wheelEvent(QWheelEvent* event)
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
        if(!event->angleDelta().isNull()) {
            if(event->angleDelta().ry()>0) {
                Configuration::getInstance().incUiHtmlZoom();
            } else {
                Configuration::getInstance().decUiHtmlZoom();
            }
            setZoomFactor(Configuration::getInstance().getUiHtmlZoomFactor());
            return;
        }
    }

    QWebView::wheelEvent(event);
}

#endif

NoteView::NoteView(QWidget* parent)
    : QWidget(parent)
{
    // widgets
    noteViewer = new NoteViewerView{this};
    connect(noteViewer, &NoteViewerView::signalReceiveText, this, &NoteView::slotReceiveText);
    connect(noteViewer, &NoteViewerView::signalExit, this, &NoteView::slotExit);

    view2EditPanel = new ViewToEditEditButtonsPanel{MfWidgetMode::NOTE_MODE, this};

    // assembly
    QVBoxLayout* layout = new QVBoxLayout{this};
    // ensure that wont be extra space around member widgets
    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->addWidget(noteViewer);
    view2EditPanel->setFixedHeight(2*view2EditPanel->getEditButton()->height());
    layout->addWidget(view2EditPanel);
    setLayout(layout);

    // signals
    QObject::connect(
        view2EditPanel->getEditButton(), SIGNAL(clicked()),
        this, SLOT(slotOpenEditor())
    );
}

NoteView::~NoteView()
{
}

void NoteView::keyPressEvent(QKeyEvent* event)
{
    if(event->modifiers() & Qt::ControlModifier){
        if(event->key()==Qt::Key_E) {
            emit signalOpenEditor();
        }
    }

    QWidget::keyPressEvent(event);
}

void NoteView::slotOpenEditor()
{
    emit signalOpenEditor();
}

void NoteView::slotReceiveText(const QString& text)
{
    emit signalReceiveText(text);
}

void NoteView::slotExit()
{
    emit signalExit();
}

} // m8r namespace
