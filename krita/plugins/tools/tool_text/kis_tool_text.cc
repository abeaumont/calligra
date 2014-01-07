/*
 *
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_text.h"

#include <KoShapeRegistry.h>
#include <KoShapeController.h>
#include <KoColorBackground.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShape.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>
#include <KoSelection.h>

#include "kis_canvas2.h"

#include "kis_cursor.h"


KisToolText::KisToolText(KoCanvasBase * canvas)
    : KisToolRectangleBase(canvas, KisToolRectangleBase::PAINT, KisCursor::load("tool_rectangle_cursor.png", 6, 6))
{
    setObjectName("tool_text");
}

KisToolText::~KisToolText()
{
}

void KisToolText::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoord(event);
    m_dragStart = m_dragCenter = m_dragEnd = pos;
    event->accept();
}

void KisToolText::continuePrimaryAction(KoPointerEvent *event)
{
    QPointF pos = convertToPixelCoord(event);

    if (event->modifiers() & Qt::AltModifier) {
        QPointF trans = pos - m_dragEnd;
        m_dragStart += trans;
        m_dragEnd += trans;
    } else {
        QPointF diag = pos - (event->modifiers() & Qt::ControlModifier
                              ? m_dragCenter : m_dragStart);
        // square?
        if (event->modifiers() & Qt::ShiftModifier) {
            double size = qMax(fabs(diag.x()), fabs(diag.y()));
            double w = diag.x() < 0 ? -size : size;
            double h = diag.y() < 0 ? -size : size;
            diag = QPointF(w, h);
        }

        // resize around center point?
        if (event->modifiers() & Qt::ControlModifier) {
            m_dragStart = m_dragCenter - diag;
            m_dragEnd = m_dragCenter + diag;
        } else {
            m_dragEnd = m_dragStart + diag;
        }
    }
    updateArea();

    m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                           (m_dragStart.y() + m_dragEnd.y()) / 2);
    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolText::endPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::HOVER_MODE);

    updateArea();

    finishRect(QRectF(m_dragStart, m_dragEnd).normalized());
    event->accept();
}

void KisToolText::finishRect(const QRectF &rect)
{
    if (rect.isNull())
        return;

    QRectF r = convertToPt(rect);
    QString shapeString = (m_optionWidget->mode() == KisTextToolOptionWidget::MODE_ARTISTIC) ? "ArtisticText" : "TextShapeID";
    KoShapeFactoryBase* textFactory = KoShapeRegistry::instance()->value(shapeString);
    if (textFactory) {
        KoShape* shape = textFactory->createDefaultShape(canvas()->shapeController()->resourceManager());
        shape->setPosition(r.topLeft());
        // If the shape is an artistic shape we keep the aspect ratio so the text isn't stretched
        if (shapeString == "ArtisticText") {
            qreal ratio = rect.height()/shape->size().height();
            r.setWidth(shape->size().width()*ratio);
        }
        shape->setSize(r.size());
        addShape(shape);

        KisCanvas2* kiscanvas = dynamic_cast<KisCanvas2 *>(canvas());
        kiscanvas->shapeManager()->selection()->deselectAll();
        kiscanvas->shapeManager()->selection()->select(shape);

        // Selection uses QTimer singleShot to activate the default tool
        // Here we have to use it too, to switch to the text tool after the other switch is done
        QTimer::singleShot(0, this, SLOT(slotActivateTextTool()));
    }
}

QList< QWidget* > KisToolText::createOptionWidgets()
{
    m_optionWidget = new KisTextToolOptionWidget();
    QList< QWidget* > widgets;
    widgets.append(m_optionWidget);
    return widgets;
}

KisPainter::FillStyle KisToolText::fillStyle()
{
    if(m_optionWidget->mode() == KisTextToolOptionWidget::MODE_MULTILINE)
        return KisPainter::FillStyleNone;
    return m_optionWidget->style();
}

void KisToolText::slotActivateTextTool()
{
    KisCanvas2* kiscanvas = dynamic_cast<KisCanvas2 *>(canvas());
    QString tool = KoToolManager::instance()->preferredToolForSelection(kiscanvas->shapeManager()->selection()->selectedShapes());
    KoToolManager::instance()->switchToolRequested(tool);
}


#include "kis_tool_text.moc"
