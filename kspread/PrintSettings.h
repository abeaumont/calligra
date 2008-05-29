/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2003 Philipp Müller <philipp.mueller@gmx.de>
   Copyright 1998, 1999 Torben Weis <weis@kde.org>,

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

#ifndef KSPREAD_PRINT_SETTINGS
#define KSPREAD_PRINT_SETTINGS

#include <KoPageLayout.h>

#include "kspread_export.h"

namespace KSpread
{

class KSPREAD_EXPORT PrintSettings
{
public:
    enum PageOrder { TopToBottom, LeftToRight };

    /**
     * Constructor.
     */
    PrintSettings();

    /**
     * Constructor.
     */
    PrintSettings(const PrintSettings& other);

    /**
     * Destructor.
     */
    virtual ~PrintSettings();

    /**
     * \return the page layout associated with this document (margins, pageSize, etc).
     * \see KoPageLayout
     */
    const KoPageLayout& pageLayout() const;

    void setPageLayout(const KoPageLayout& pageLayout);

    void setPageFormat(KoPageFormat::Format format);
    void setPageOrientation(KoPageFormat::Orientation orientation);

    /**
     * \return the print width of the paper.
     */
    double printWidth() const;

    /**
     * \return the print height of the paper.
     */
    double printHeight() const;

    /**
     * The order in which the pages should be created.
     * Either they are created beginning at the left, continuing to the right and
     * then the next row of pages, or they are created vertically page column-wise.
     *
     * \return the page order
     */
    PageOrder pageOrder() const;
    void setPageOrder(PageOrder order);

    /**
     * Returns, if the grid shall be shown on printouts.
     */
    bool printGrid() const;

    /**
     * Sets, if the grid shall be shown on printouts.
     */
    void setPrintGrid(bool printGrid);

    /**
     * Returns, if the charts shall be shown on printouts.
     */
    bool printCharts() const;

    /**
     * Sets, if the charts shall be shown on printouts.
     */
    void setPrintCharts(bool printCharts);

    /**
     * Returns, if the objects shall be shown on printouts
     */
    bool printObjects() const;

    /**
     * Sets, if the objects shall be shown on printouts.
     */
    void setPrintObjects(bool printObjects);

    /**
     * Returns, if the graphics shall be shown on printouts.
     */
    bool printGraphics() const;

    /**
     * Sets, if the graphics shall be shown on printouts.
     */
    void setPrintGraphics(bool printGraphics);

    /**
     * Returns, if the comment rect shall be shown on printouts
     */
    bool printCommentIndicator() const;

    /**
     * Sets, if the comment rect shall be shown on printouts
     */
    void setPrintCommentIndicator(bool printCommentIndicator);

    /**
     * Returns, if the formula rect shall be shown on printouts.
     */
    bool printFormulaIndicator() const;

    /**
     * Sets, if the formula Rect shall be shown on printouts.
     */
    void setPrintFormulaIndicator(bool printFormulaIndicator);

    /**
     * Assignment operator.
     */
    void operator=(const PrintSettings& settings);

    /**
     * Equality operator.
     */
    bool operator==(const PrintSettings& other) const;
    inline bool operator!=(const PrintSettings& other) const { return !operator==(other); }

private:
    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_PRINT_SETTINGS
