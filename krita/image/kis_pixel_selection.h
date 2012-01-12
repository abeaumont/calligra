/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PIXEL_SELECTION_H_
#define KIS_PIXEL_SELECTION_H_

#include <QRect>
#include <QPainterPath>

#include <kicon.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_selection_component.h"
#include <krita_export.h>

class KisSelection;

/**
 * KisPixelSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not.
 */
class KRITAIMAGE_EXPORT KisPixelSelection : public KisSelectionComponent
{

public:

    /**
     * Create a new KisPixelSelection. This selection will not have a
     * parent paint device.
     */
    KisPixelSelection(KisDefaultBoundsBaseSP defaultBounds = 0);

    /**
     * Copy the selection
     */
    KisPixelSelection(const KisPixelSelection& rhs);

    virtual ~KisPixelSelection();


    KisPaintDeviceSP paintDevice() const;

// KisSelectionComponent implementation
public:

    KisSelectionComponentSP clone(KisSelection*);

    /**
     * Fill the specified rect with the specified selectedness.
     */
    void select(const QRect & r, quint8 selectedness = MAX_SELECTED);

    /**
     * Invert the total selection. This will also invert the default value
     * of the selection paint device, from MIN_SELECTED to MAX_SELECTED or
     * back.
     */
    void invert();

    /**
     * Apply a selection to the selection using the specified selection mode
     */
    void applySelection(KisSelectionComponentSP selection, SelectionAction action);

    /// Tests if the the rect is totally outside the selection
    bool isTotallyUnselected(const QRect & r) const;

    /**
     * Rough, but fastish way of determining the area
     * of the tiles used by the selection.
     */
    QRect selectedRect() const;

    /**
     * Slow, but exact way of determining the rectangle
     * that encloses the selection.
     */
    QRect selectedExactRect() const;

    /**
     * @brief outline returns the outline of the current selection
     * @return a vector of polygons that can be used to draw the outline
     */
    QVector<QPolygon> outline() const;

    virtual void renderToProjection(KisPaintDeviceSP projection);
    virtual void renderToProjection(KisPaintDeviceSP projection, const QRect& r);


// KisPaintDevice overrides
public:

    /**
     * Set the specified rect to MIN_SELECTED.
     */
    void clear(const QRect & r);

    /**
     * Reset the entire selection. The selectedRect and selectedExactRect
     * will be empty. The selection will be completely deselected.
     */
    void clear();

    KisPaintDeviceSP createThumbnailDevice(qint32 w, qint32 h, const KisSelection * selection, QRect rect) const;


private:
    /**
     * Add a selection
     */
    void addSelection(KisPixelSelectionSP selection);

    /**
     * Subtracts a selection
     */
    void subtractSelection(KisPixelSelectionSP selection);

    /**
     * Intersects a selection using min-T-norm for this.
     */
    void intersectSelection(KisPixelSelectionSP selection);


private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_PIXEL_SELECTION_H_
