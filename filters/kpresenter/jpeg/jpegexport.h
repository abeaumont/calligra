/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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

#ifndef __XPMEXPORT_H__
#define __XPMEXPORT_H__

#include "imageexport.h"

class JpegExport : public ImageExport
{
    Q_OBJECT

public:
    JpegExport(QObject* parent, const QStringList&);
    virtual ~JpegExport();
    virtual bool saveImage( const QString& fileName);
    virtual bool extraImageAttribute();
    virtual const char * exportFormat();
};

#endif // __XPMEXPORT_H__

