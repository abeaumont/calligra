/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson

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
 * Boston, MA 02110-1301, USA.
*/
#include "KPrPen.h"

#include <KoTextZoomHandler.h>

KPrPen::KPrPen()
 : QPen()
{
  m_pointWidth = 1.0;
}

KPrPen::KPrPen(const QColor& _color, double _pointWidth, Qt::PenStyle _style)
 : QPen()
{
  setColor(_color);
  setPointWidth(_pointWidth);
  setStyle(_style);
}

KPrPen::KPrPen(const QColor& _color)
 : QPen(_color)
{
  m_pointWidth = 1.0;
}

KPrPen::~KPrPen()
{
}

void KPrPen::setPointWidth(double w)
{
  m_pointWidth = w;
}

QPen KPrPen::zoomedPen(KoZoomHandler* zoomHandler)
{
  QPen pen = *this;
  pen.setWidth(zoomHandler->zoomItY(m_pointWidth));

  return pen;
}
