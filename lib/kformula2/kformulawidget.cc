/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpainter.h>

#include "formulacursor.h"
#include "kformulacontainer.h"
#include "kformulawidget.h"


KFormulaWidget::KFormulaWidget(KFormulaContainer* doc, QWidget* parent, const char* name, WFlags f)
    : QWidget(parent, name, f), document(doc)
{
    cursor = document->createCursor();

    connect(document, SIGNAL(formulaChanged()), this, SLOT(formulaChanged()));
}

KFormulaWidget::~KFormulaWidget()
{
    document->destroyCursor(cursor);
}


void KFormulaWidget::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    document->draw(painter);
    cursor->draw(painter);
    painter.end();
}

void KFormulaWidget::keyPressEvent(QKeyEvent* event)
{
    QPainter painter;
    painter.begin(this);
    cursor->draw(painter);

    QChar ch = event->text().at(0);
    if (ch.isPrint()) {
        int latin1 = ch.latin1();
        switch (latin1) {
        case '(':
            document->addBracket(cursor, '(', ')');
            break;
        case '[':
            document->addBracket(cursor, '[', ']');
            break;
        case '{':
            break;
        case '|':
            document->addBracket(cursor, '|', '|');
            break;
        case '/':
            document->addFraction(cursor);
            break;
        case '#':
            // here we need a dialog!
            document->addMatrix(cursor, 4, 5);
            break;
        case '\\':
            document->addRoot(cursor);
            break;
        case '+':
        case '-':
        case '*':
        case '=':
        case '<':
        case '>':
            document->addOperator(cursor, ch);
            break;
        case '^':
            document->addUpperRightIndex(cursor);
            break;
        case '_':
            document->addLowerRightIndex(cursor);
            break;
        case ' ':
            break;
        default:
            document->addText(cursor, ch);
        }
    }
    else {
        int action = event->key();
        int state = event->state();
	int flag=0;

	if(state & Qt::ControlButton)
	  flag+=FormulaCursor::WordMovement;

	if(state & Qt::ShiftButton)
	  flag+=FormulaCursor::SelectMovement;

	switch (action) {
	case Qt::Key_Left:
            cursor->moveLeft(flag);
            break;
        case Qt::Key_Right:
            cursor->moveRight(flag);
            break;
        case Qt::Key_Up:
            cursor->moveUp(flag);
            break;
        case Qt::Key_Down:
            cursor->moveDown(flag);
            break;
        case Qt::Key_BackSpace:
            document->removeSelection(cursor, BasicElement::beforeCursor);
            break;
        case Qt::Key_Delete:
            document->removeSelection(cursor, BasicElement::afterCursor);
            break;
        case Qt::Key_Home:
            cursor->moveHome(flag);
            break;
        case Qt::Key_End:
            cursor->moveEnd(flag);
            break;
        case Qt::Key_F1:
            document->addSymbol(cursor, Artwork::product);
            break;
        case Qt::Key_F2:
            document->addSymbol(cursor, Artwork::sum);
            break;
        case Qt::Key_F3:
            document->addSymbol(cursor, Artwork::integral);
            break;
        case Qt::Key_F4:
            document->save("test");
            break;
        case Qt::Key_F7:
            document->undo(cursor);
            break;
        case Qt::Key_F8:
            document->redo(cursor);
            break;

	}
    }

    //Is this necessary here ?
    
    document->testDirty();

    cursor->draw(painter);
    painter.end();
}


void KFormulaWidget::formulaChanged()
{
    update();
}
