/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007-2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kra/kis_kra_save_visitor.h"
#include "kra/kis_kra_tags.h"

#include <colorprofiles/KoIccColorProfile.h>
#include <KoStore.h>
#include <KoColorSpace.h>

#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_shape_layer.h"
#include "kis_clone_layer.h"
#include "kis_mask.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"
#include "kis_selection_mask.h"
#include "kis_selection_component.h"
#include "flake/kis_shape_selection.h"
#include <kis_pixel_selection.h>

using namespace KRA;

KisKraSaveVisitor::KisKraSaveVisitor(KisImageSP img, KoStore *store, quint32 &count, const QString & name, QMap<const KisNode*, QString> nodeFileNames)
    : KisNodeVisitor()
    , m_img( img )
    , m_store( store )
    , m_external( false )
    , m_count(count)
    , m_name( name )
    , m_nodeFileNames( nodeFileNames )
{
}

void KisKraSaveVisitor::setExternalUri(const QString &uri)
{
    m_external = true;
    m_uri = uri;
}

bool KisKraSaveVisitor::visit(KisExternalLayer * layer)
{
    bool result = false;
    if (KisShapeLayer* shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {
        m_store->pushDirectory();
        m_store->enterDirectory( getLocation( layer, DOT_SHAPE_LAYER )) ;
        result = shapeLayer->saveLayer(m_store);
        m_store->popDirectory();
    }
    m_count++;
    return result || visitAllInverse(layer);;
}

bool KisKraSaveVisitor::visit(KisPaintLayer *layer)
{
    if (!savePaintDevice(layer)) return false;
    if (!saveAnnotations(layer)) return false;
    m_count++;
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisGroupLayer *layer)
{
    m_count++;
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisAdjustmentLayer* layer)
{
    if (!saveSelection( layer )) return false;
    if (!saveFilterConfiguration( layer ) ) return false;
    m_count++;
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisGeneratorLayer * layer)
{
    if (!saveSelection(layer)) return false;
    if (!saveAnnotations( layer )) return false; // generator layers can have a profile because they have their own pixel data
    if (!saveFilterConfiguration( layer ) ) return false;
    m_count++;
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisCloneLayer *layer)
{
    // Clone layers do not have a profile
    m_count++;
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisFilterMask *mask)
{
    if (!saveSelection( mask )) return false;
    if (!saveFilterConfiguration( mask ) ) return false;
    m_count++;
    return true;
}

bool KisKraSaveVisitor::visit(KisTransparencyMask *mask)
{
    if (!saveSelection( mask )) return false;
    m_count++;
    return true;
}

bool KisKraSaveVisitor::visit(KisTransformationMask *mask)
{
    if (!saveSelection( mask )) return false;
    m_count++;
    return true;
}

bool KisKraSaveVisitor::visit(KisSelectionMask *mask)
{
    if (!saveSelection( mask )) return false;
    m_count++;
    return true;
}


bool KisKraSaveVisitor::savePaintDevice(KisNode * node)
{

    //connect(*node->paintDevice(), SIGNAL(ioProgress(qint8)), m_img, SLOT(slotIOProgress(qint8)));
    // Layer data
    if (m_store->open(getLocation( node ))) {
        if (!node->paintDevice()->write(m_store)) {
            node->paintDevice()->disconnect();
            m_store->close();
            //IODone();
            return false;
        }

        m_store->close();
    }
    return true;
}

bool KisKraSaveVisitor::saveAnnotations(KisLayer* layer)
{
    if ( !layer ) return false;
    if ( !layer->paintDevice() ) return false;
    if ( !layer->paintDevice()->colorSpace() ) return false;

    if (layer->paintDevice()->colorSpace()->profile()) {
        const KoColorProfile *profile = layer->paintDevice()->colorSpace()->profile();
        KisAnnotationSP annotation;
        if (profile) {
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile && !iccprofile->rawData().isEmpty())
                annotation = new KisAnnotation(ICC, iccprofile->name(), iccprofile->rawData());
        }

        if (annotation) {
            // save layer profile
            if (m_store->open(getLocation( layer, DOT_ICC ))) {
                m_store->write(annotation->annotation());
                m_store->close();
            }
            else {
                return false;
            }
        }
    }
    return true;

}
bool KisKraSaveVisitor::saveSelection(KisNode* node)
{
    KisSelectionSP selection;
    if (node->inherits("KisMask")) {
        selection = static_cast<KisMask*>(node)->selection();
    }
    else if (node->inherits( "KisAdjustmentLayer" )) {
        selection = static_cast<KisAdjustmentLayer*>(node)->selection();
    }
    else {
        return false;
    }

    if ( selection->hasPixelSelection() ) {
        KisPaintDeviceSP dev = selection->pixelSelection();
        //connect(*dev, SIGNAL(ioProgress(qint8)), m_img, SLOT(slotIOProgress(qint8)));
        // Layer data
        if (m_store->open(getLocation( node, DOT_PIXEL_SELECTION ))) {
            if (!dev->write(m_store)) {
                dev->disconnect();
                m_store->close();
                return false;
            }
            m_store->close();
        }
    }
    if ( selection->hasShapeSelection() ) {
        m_store->pushDirectory();

        m_store->enterDirectory( getLocation( node, DOT_SHAPE_SELECTION ) );
        KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>( selection->shapeSelection() );
        if ( !shapeSelection ) {
            m_store->popDirectory();
            return false;
        }

        if ( !shapeSelection->saveOdf(m_store) ) {
            m_store->popDirectory();
            return false;
        }
        m_store->popDirectory();
    }
    return true;
}

bool KisKraSaveVisitor::saveFilterConfiguration(KisNode* node)
{
    if (node->inherits( "KisNodeFilterInterface" )) {
	KisNodeFilterInterface* filterInterface = dynamic_cast<KisNodeFilterInterface*>(node);
	if (!filterInterface) return false;
        KisFilterConfiguration* filter = filterInterface->filter();
        if (filter) {
            if (m_store->open(getLocation( node, DOT_FILTERCONFIG ))) {
                QString s = filter->toLegacyXML();
                m_store->write(s.toUtf8(), qstrlen(s.toUtf8()));
                m_store->close();
            }
        }

    }
    return false;
}

QString KisKraSaveVisitor::getLocation( KisNode* node, const QString& suffix )
{

    QString location = m_external ? QString::null : m_uri;
    Q_ASSERT( m_nodeFileNames.contains( node ) );
    location += m_name + LAYER_PATH + m_nodeFileNames[node] + suffix;
    return location;
}
