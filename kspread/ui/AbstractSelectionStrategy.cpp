/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "AbstractSelectionStrategy.h"

#include "Selection.h"
#include "Sheet.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoTool.h>

using namespace KSpread;

class AbstractSelectionStrategy::Private
{
public:
    Selection* selection;
    QPointF start;
};

AbstractSelectionStrategy::AbstractSelectionStrategy(KoTool* parent, KoCanvasBase* canvas, Selection* selection,
                                     const QPointF documentPos, Qt::KeyboardModifiers modifiers)
    : KoInteractionStrategy(parent, canvas)
    , d(new Private)
{
    Q_UNUSED(modifiers)
    d->selection = selection;
    d->start = documentPos;
}

AbstractSelectionStrategy::~AbstractSelectionStrategy()
{
    delete d;
}

void AbstractSelectionStrategy::handleMouseMove(const QPointF& documentPos, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers & Qt::ShiftModifier)
        return;
    Q_ASSERT(m_canvas->shapeManager()->selection()->count() > 0);
    KoShape* shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = documentPos - shape->position();
    // In which cell did the user click?
    double xpos;
    double ypos;
    int col = d->selection->activeSheet()->leftColumn(position.x(), xpos);
    int row = d->selection->activeSheet()->topRow(position.y(), ypos);
    // Check boundaries.
    if (col > KS_colMax || row > KS_rowMax)
    {
        kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
        return;
    }
    // Update the selection.
    d->selection->update(QPoint(col, row));
    m_parent->repaintDecorations();
}

QUndoCommand* AbstractSelectionStrategy::createCommand()
{
    return 0;
}

void AbstractSelectionStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    m_parent->repaintDecorations();
}

Selection* AbstractSelectionStrategy::selection() const
{
    return d->selection;
}

const QPointF& AbstractSelectionStrategy::startPosition() const
{
    return d->start;
}
