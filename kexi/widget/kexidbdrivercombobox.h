/* This file is part of the KDE project
   Copyright (C) 2005-2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDBDRIVERCOMBOBOX_H
#define KEXIDBDRIVERCOMBOBOX_H

#include "kexiextwidgets_export.h"

#include <KDbDriver>

#include <QHash>

//! \brief Combo box widget for selecting a database driver
/*! This widget provides a combobox for selecting a database driver.


\b Usage: \n
\code
  KDbDriverManager manager;
  KDbDriver::InfoHash drvs = manager.driversInfo();

  KexiDBDriverComboBox* combo = new KexiDBDriverComboBox(drvs, true, 0);
\endcode

A more complete example can be found in
<a href="http://lxr.kde.org/source/calligra/kexi/tests/widgets/kexidbdrivercombotest.cpp">calligra/kexi/tests/widgets/</a>.
*/
class KEXIEXTWIDGETS_EXPORT KexiDBDriverComboBox : public KComboBox
{
    Q_OBJECT

public:
    enum Options {
        ShowFileDrivers = 1,
        ShowServerDrivers = 2,
        ShowAll = ShowFileDrivers | ShowServerDrivers
    };

    /*! Constructs a KexiDBDriverComboBox object.

        The combobox is populated with the names of the drivers in
        \a driversInfo.  A suitable value for \a driversInfo can be obtained
        from KDbDriverManager::driversInfo().

        If \a includeFileBasedDrivers is set to false, then only those drivers
        that are for database servers (those which have X-Kexi-DriverType=Network
        in their .desktop file) are shown. */
    KexiDBDriverComboBox(QWidget* parent, const KDbDriver::InfoHash& driversInfo,
                         Options options = ShowAll);

    ~KexiDBDriverComboBox();

    /*! Gets a list of the names of all drivers.

        Note that this returns just the names of those drivers that are in the
        combobox: if the includeFileBasedDrivers argument to the constructor
        was false, this won't include the file based drivers either.

        \return a list of names of drivers that were found */
    QStringList driverNames() const;

    /*! Get the name of the currrently selected driver.  If the combobox is empty,
        QString() will be returned.

        \return the name of the currently selected driver */
    QString selectedDriverName() const;

    /*! Set the currrently selected driver.

        The combobox entry for \a driverName is selected.  If \a driverName
        is not listed in the combobox then there is no change.  The search
        is case insensitive.

        */
    void setDriverName(const QString& driverName);

private:
    class Private;
    Private * const d;
};

#endif
