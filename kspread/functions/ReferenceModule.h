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

#ifndef KSPREAD_REFERENCE_MODULE
#define KSPREAD_REFERENCE_MODULE

#include "FunctionModuleFactory.h"

namespace KSpread
{

class ReferenceModulePlugin : public QObject
{
    Q_OBJECT
public:
    ReferenceModulePlugin(QObject* parent, const QStringList&);
};


class ReferenceModuleFactory : public FunctionModuleFactory
{
    Q_OBJECT
public:
    ReferenceModuleFactory(QObject* parent);

    void registerFunctions();
    void removeFunctions();
};

} // namespace KSpread

#endif // KSPREAD_REFERENCE_MODULE
