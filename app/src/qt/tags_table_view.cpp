/*
 tags_table_view.cpp     MindForger thinking notebook

 Copyright (C) 2016-2022 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "tags_table_view.h"

namespace m8r {

using namespace std;

TagsTableView::TagsTableView(QWidget* parent)
    : QTableView(parent)
{
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    setSortingEnabled(true);

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void TagsTableView::keyPressEvent(QKeyEvent* event)
{
    if(!(event->modifiers() & Qt::AltModifier)
         &&
       !(event->modifiers() & Qt::ControlModifier)
         &&
       !(event->modifiers() & Qt::ShiftModifier))
    {
        switch(event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Right:
            emit signalShowDialogForTag();
            return;
        case Qt::Key_Down:
            QTableView::keyPressEvent(event);
            return;
        case Qt::Key_Up:
        // IMPROVE left to cancel selection
        case Qt::Key_Left:
            QTableView::keyPressEvent(event);
            return;
        case Qt::Key_Home:
            MF_DEBUG("  TagsTableView::keyPressEvent HOME" << endl);
            if(this->model()->rowCount() > 0) {
                this->selectRow(0);
            }
            return;
        case Qt::Key_End:
            MF_DEBUG("  TagsTableView::keyPressEvent END" << endl);
            if(this->model()->rowCount() > 0) {
                this->selectRow(this->model()->rowCount()-1);
            }
            return;
        case Qt::Key_PageUp: {
            MF_DEBUG("  TagsTableView::keyPressEvent PAGE_UP" << endl);
            // get currently selected row
            QModelIndexList indices = selectionModel()->selection().indexes();
            // no indexes > no row > no selection
            for(int i=0; i<indices.count(); i++) {
                // calculate row to select (based on the number of rows)
                int currentRow = indices.at(i).row();
                int newRow{currentRow - PG_UP_DOWN_STEP_SIZE};
                if(newRow < 0) {
                    newRow = 0;
                }
                // select row
                this->selectRow(newRow);
            }
            // no row selected
            return;
        }
        case Qt::Key_PageDown: {
            MF_DEBUG("  TagsTableView::keyPressEvent PAGE_DOWN" << endl);
            // get currently selected row
            QModelIndexList indices = selectionModel()->selection().indexes();
            // no indexes > no row > no selection
            for(int i=0; i<indices.count(); i++) {
                // calculate row to select (based on the number of rows)
                int currentRow = indices.at(i).row();
                int newRow{currentRow + PG_UP_DOWN_STEP_SIZE};
                if(newRow > this->model()->rowCount()-1) {
                    newRow = this->model()->rowCount()-1;
                }
                // select row
                this->selectRow(newRow);
            }
            // no row selected
            return;
            }
        }

        return;
    }

    QTableView::keyPressEvent(event);
}

void TagsTableView::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // double click to O opens it
    emit signalShowDialogForTag();
}

void TagsTableView::resizeEvent(QResizeEvent* event)
{
    MF_DEBUG("TagsTableView::resizeEvent " << event << std::endl);

    if(horizontalHeader()->length() > 0) {
        // ensure that 1st column gets the remaining space from others
        horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    }
    verticalHeader()->setDefaultSectionSize(fontMetrics().height()*1.5);

    // cardinality
    this->setColumnWidth(1, this->fontMetrics().averageCharWidth()*6);

    QTableView::resizeEvent(event);
}

} // m8r namespace
