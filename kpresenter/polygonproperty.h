// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>

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
#ifndef POLYGONPROPERTY_H
#define POLYGONPROPERTY_H

#include <qwidget.h>

#include "kprcommand.h"

class PolygonPropertyUI;

class PolygonProperty : public QWidget
{
    Q_OBJECT
public:
    PolygonProperty( QWidget *parent, const char *name, PolygonSettingCmd::PolygonSettings &polygonSettings );
    ~PolygonProperty();

    int getPolygonPropertyChange() const;
    PolygonSettingCmd::PolygonSettings getPolygonSettings() const;

    void apply();

protected:
    bool isConvexConcave() const;

    PolygonPropertyUI *m_ui;

    PolygonSettingCmd::PolygonSettings m_polygonSettings;

protected slots:
    void slotReset();
    void slotTypeChanged( int pos );
};

#endif // POLYGONPROPERTY_H

