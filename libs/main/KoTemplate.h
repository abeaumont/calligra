/*
   This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>

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

#ifndef koTemplate_h
#define koTemplate_h

#include <QStringList>
#include <QPixmap>

/** @internal */
class KoTemplate
{

public:
    explicit KoTemplate(const QString &name,
                        const QString &description = QString(),
                        const QString &file = QString(),
                        const QString &picture = QString(),
                        const QString &fileName = QString(),
                        const QString &_measureSystem = QString(),
                        const QString &color = QString(),
                        const QString &swatch = QString(),
                        const QString &variantName = QString(),
                        bool wide = false,
                        bool hidden = false, bool touched = false);
    ~KoTemplate() {}

    QString name() const {
        return m_name;
    }
    QString description() const {
        return m_descr;
    }
    QString file() const {
        return m_file;
    }
    QString picture() const {
        return m_picture;
    }
    QString fileName() const {
        return m_fileName;
    }
    const QPixmap &loadPicture();

    bool isHidden() const {
        return m_hidden;
    }
    void setHidden(bool hidden = true) {
        m_hidden = hidden; m_touched = true;
    }

    bool touched() const {
        return m_touched;
    }

    QString measureSystem() const {
        return m_measureSystem;
    }
    void setMeasureSystem(const QString& system) {
        m_measureSystem = system;
    }

    QString color() const {
        return m_color;
    }
    void setColor(const QString& color) {
        m_color = color;
    }

    QString swatch() const {
        return m_swatch;
    }
    void setSwatch(const QString& swatch) {
        m_swatch = swatch;
    }

    QString variantName() const {
        return m_variantName;
    }
    void setVariantName(const QString& variantName) {
        m_variantName = variantName;
    }

    QString thumbnail() const {
        return m_thumbnail;
    }
    void setThumbnail(const QString& thumbnail) {
        m_thumbnail = thumbnail;
    }

    bool wide() const {
        return m_wide;
    }
    void setWide(bool wide) {
        m_wide = wide;
    }
private:
    QString m_name, m_descr, m_file, m_picture, m_fileName, m_color, m_swatch, m_variantName, m_thumbnail;
    bool m_wide;
    bool m_hidden;
    mutable bool m_touched;
    bool m_cached;
    QPixmap m_pixmap;
    QString m_measureSystem;
};


#endif
