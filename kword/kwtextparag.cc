/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kwtextparag.h"
#include "kwtextdocument.h"
#include "kwutils.h"
#include "kwstyle.h"
#include "kwdoc.h"
#include "kwformat.h"
#include "kwanchor.h"
#include "kwtextimage.h"
#include "kwtextframeset.h"
#include "counter.h"
#include <kdebug.h>
#include <qdom.h>
#include <qtl.h>

KWTextParag::KWTextParag( QTextDocument *d, QTextParag *pr, QTextParag *nx, bool updateIds)
    : QTextParag( d, pr, nx, updateIds )
{
    //kdDebug() << "KWTextParag::KWTextParag " << this << endl;
    m_item = 0L;
}

KWTextParag::~KWTextParag()
{
    if ( !textDocument()->isDestroying() )
        invalidateCounters();
    //kdDebug() << "KWTextParag::~KWTextParag " << this << endl;
    delete m_item;
}

KWTextDocument * KWTextParag::textDocument() const
{
    return static_cast<KWTextDocument *>( document() );
}

// There is one QStyleSheetItems per paragraph, created on demand,
// in order to set the DisplayMode for counters.
void KWTextParag::checkItem( QStyleSheetItem * & item, const char * name )
{
    if ( !item )
    {
        item = new QStyleSheetItem( 0, QString::fromLatin1(name) /* For debugging purposes only */ );
        QVector<QStyleSheetItem> vec = styleSheetItems();
        vec.resize( vec.size() + 1 );
        vec.insert( vec.size() - 1, item );
        //kdDebug() << "KWTextParag::checkItem inserting QStyleSheetItem " << name << " at position " << vec.size()-1 << endl;
        setStyleSheetItems( vec );
    }
}

// Return the counter associated with this paragraph.
Counter *KWTextParag::counter()
{
    if ( !m_layout.counter )
        return m_layout.counter;

    // Garbage collect unnneeded counters.
    if ( m_layout.counter->numbering() == Counter::NUM_NONE )
        setNoCounter();
    return m_layout.counter;
}

void KWTextParag::setMargin( QStyleSheetItem::Margin m, double _i )
{
    //kdDebug() << "KWTextParag::setMargin " << m << " margin " << _i.pt() << endl;
    m_layout.margins[m] = _i;
    if ( m == QStyleSheetItem::MarginTop && prev() )
        prev()->invalidate(0);     // for top margin
    invalidate(0);
}

void KWTextParag::setMargins( const double * margins )
{
    for ( int i = 0 ; i < 5 ; ++i )
        m_layout.margins[i] = margins[i];
    if ( prev() )
        prev()->invalidate(0);     // for top margin
    invalidate(0);
}

void KWTextParag::setAlign( int align )
{
    setAlignment( align );
    m_layout.alignment = align;
}

void KWTextParag::setLineSpacing( double _i )
{
    m_layout.lineSpacing = _i;
    invalidate(0);
}

void KWTextParag::setLinesTogether( bool b )
{
    m_layout.linesTogether = b;
    invalidate(0);
}

void KWTextParag::setTopBorder( const Border & _brd )
{
    m_layout.topBorder = _brd;
    invalidate(0);
}

void KWTextParag::setBottomBorder( const Border & _brd )
{
    m_layout.bottomBorder = _brd;
    invalidate(0);
}

void KWTextParag::setNoCounter()
{
    delete m_layout.counter;
    m_layout.counter = 0L;
    invalidateCounters();
}

void KWTextParag::setCounter( const Counter & counter )
{
    // Garbage collect unnneeded counters.
    if ( counter.numbering() == Counter::NUM_NONE )
    {
        setNoCounter();
    }
    else
    {
        delete m_layout.counter;
        m_layout.counter = new Counter( counter );

        checkItem( m_item, "m_item" );
        // Set the display mode (in order for drawLabel to get called by QTextParag)
        m_item->setDisplayMode( QStyleSheetItem::DisplayListItem );
        //m_item->setSelfNesting( FALSE ); // Not sure why this is necessary.. to be investigated

        // Invalidate the counters
        invalidateCounters();
    }
}

void KWTextParag::invalidateCounters()
{
    // Invalidate this paragraph and all the following ones
    // (Numbering may have changed)
    invalidate( 0 );
    if ( m_layout.counter )
        m_layout.counter->invalidate();
    KWTextParag *s = static_cast<KWTextParag *>( next() );
    while ( s ) {
        if ( s->m_layout.counter )
            s->m_layout.counter->invalidate();
        s->invalidate( 0 );
        s = static_cast<KWTextParag *>( s->next() );
    }
}

int KWTextParag::counterWidth() const
{
    if ( !m_layout.counter )
        return 0;

    return m_layout.counter->width( this );
}

// Draw the counter/bullet for a paragraph
void KWTextParag::drawLabel( QPainter* p, int x, int y, int /*w*/, int h, int base, const QColorGroup& /*cg*/ )
{
    if ( !m_layout.counter ) // shouldn't happen
        return;

    if ( m_layout.counter->numbering() == Counter::NUM_NONE )
    {   // Garbage collect unnneeded counter.
        delete m_layout.counter;
        m_layout.counter = 0L;
        return;
    }

    int size = m_layout.counter->width( this );

    // Draw the complete label.
    QTextFormat *format = at( 0 )->format(); // paragFormat();
    p->save();
    QPen tmpPen=p->pen();

    QColor newColor(format->color());
    QFont newFont(format->font());

    p->setFont( newFont );
    tmpPen.setColor( newColor);
    p->setPen(tmpPen);

    // Now draw any bullet that is required over the space left for it.
    if ( m_layout.counter->isBullet() )
    {
        // Modify x offset.
        for ( unsigned int i = 0; i < m_layout.counter->suffix().length(); i++ )
            x -= format->width( m_layout.counter->suffix(), i );
        int width = format->width( ' ' );
        int height = format->height();
        QRect er( x - width, y - h + height / 2 - width / 2, width, width );
        // Draw the bullet.
        switch ( m_layout.counter->style() )
        {
            case Counter::STYLE_DISCBULLET:
                p->setBrush( QBrush(newColor) );
                p->drawEllipse( er );
                p->setBrush( Qt::NoBrush );
                break;
            case Counter::STYLE_SQUAREBULLET:
                p->fillRect( er , QBrush(newColor) );
                break;
            case Counter::STYLE_CIRCLEBULLET:
                p->drawEllipse( er );
                break;
            case Counter::STYLE_CUSTOMBULLET:
                // The user has selected a symbol from a special font. Override the paragraph
                // font with the given family. This conserves the right size etc.
                if ( !m_layout.counter->customBulletFont().isEmpty() )
                {
                    newFont.setFamily( m_layout.counter->customBulletFont() );
                    p->setFont( newFont );
                }
                p->drawText( x - width, y - h + base, m_layout.counter->customBulletCharacter() );
                break;
            default:
                break;
        }
    }
    else
    {
        // There are no bullets...any parent bullets have already been suppressed.
        // Just draw the text! Note: one space is always appended.
        QString counterText = m_layout.counter->text( this );
        if ( !counterText.isEmpty() )
            p->drawText( x - size, y - h + base, counterText + ' ' );
    }
    p->restore();
}

int KWTextParag::topMargin() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItY(
        m_layout.margins[ QStyleSheetItem::MarginTop ]
        + m_layout.topBorder.ptWidth ) );
}

int KWTextParag::bottomMargin() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItY(
        m_layout.margins[ QStyleSheetItem::MarginBottom ]
        + m_layout.bottomBorder.ptWidth ) );
}

int KWTextParag::leftMargin() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItX(
        m_layout.margins[ QStyleSheetItem::MarginLeft ]
        + m_layout.leftBorder.ptWidth )
        + counterWidth() /* shouldn't be zoomed, it depends on the font sizes */);
}

int KWTextParag::rightMargin() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItX(
        m_layout.margins[ QStyleSheetItem::MarginRight ]
        + m_layout.rightBorder.ptWidth ) );
}

int KWTextParag::firstLineMargin() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItX(
        m_layout.margins[ QStyleSheetItem::MarginFirstLine ] ) );
}

int KWTextParag::lineSpacing() const
{
    return static_cast<int>( textDocument()->textFrameSet()->kWordDocument()->zoomItY(
        m_layout.lineSpacing ) );
}

// Reimplemented from QTextParag
void KWTextParag::paint( QPainter &painter, const QColorGroup &cg, QTextCursor *cursor, bool drawSelections,
                         int clipx, int clipy, int clipw, int cliph )
{
    //qDebug("KWTextParag::paint %p", this);
    QTextParag::paint( painter, cg, cursor, drawSelections, clipx, clipy, clipw, cliph );

    if ( m_layout.topBorder.ptWidth > 0
         || m_layout.bottomBorder.ptWidth > 0
         || m_layout.leftBorder.ptWidth > 0
         || m_layout.rightBorder.ptWidth > 0 )
    {
        KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
        int leftX = 0;
        int rightX = documentWidth()-1;
        int topY = lineY( 0 ); // Maybe this is always 0. Not sure.
        int bottomY = static_cast<int>( lineY( lines() -1 ) + lineHeight( lines() -1 ) - m_layout.lineSpacing );
        //kdDebug() << "KWTextParag::paint bottomY=" << bottomY << endl;
        if ( m_layout.topBorder.ptWidth > 0 )
        {
            int width = QMAX( 1, (int)(doc->zoomItY( m_layout.topBorder.ptWidth ) + 0.5) );
            painter.setPen( Border::borderPen( m_layout.topBorder, width ) );
            painter.drawLine( leftX, topY, rightX, topY );
        }
        if ( m_layout.bottomBorder.ptWidth > 0 )
        {
            int width = QMAX( 1, (int)(doc->zoomItY( m_layout.bottomBorder.ptWidth ) + 0.5) );
            painter.setPen( Border::borderPen( m_layout.bottomBorder, width ) );
            painter.drawLine( leftX, bottomY, rightX, bottomY );
        }
        if ( m_layout.leftBorder.ptWidth > 0 )
        {
            int width = QMAX( 1, (int)(doc->zoomItX( m_layout.leftBorder.ptWidth ) + 0.5) );
            painter.setPen( Border::borderPen( m_layout.leftBorder, width ) );
            painter.drawLine( leftX, topY, leftX, bottomY );
        }
        if ( m_layout.rightBorder.ptWidth > 0 )
        {
            int width = QMAX( 1, (int)(doc->zoomItX( m_layout.rightBorder.ptWidth ) + 0.5) );
            painter.setPen( Border::borderPen( m_layout.rightBorder, width ) );
            painter.drawLine( rightX, topY, rightX, bottomY );
        }
    }
}

void KWTextParag::copyParagData( QTextParag *_parag )
{
    KWTextParag * parag = static_cast<KWTextParag *>(_parag);
    KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
    // Style of the previous paragraph
    KWStyle * style = doc->findStyle( parag->styleName(), true );
    // Obey "following style" setting
    bool styleApplied = false;
    if ( style && !style->followingStyle().isEmpty() )
    {
        KWStyle * newStyle = doc->findStyle( style->followingStyle(), true );
        if (!newStyle)
            kdWarning() << "Following style " << style->followingStyle() << " not found" << endl;
        else if ( style != newStyle ) // if same style, keep paragraph-specific changes as usual
        {
            setParagLayout( newStyle->paragLayout() );
            QTextFormat * format = document()->formatCollection()->format( &newStyle->format() );
            setFormat( format );
            string()->setFormat( 0, format, true ); // prepare format for text insertion
            styleApplied = true;
        }
    }
    else if (!style)
        kdWarning() << "Paragraph style '" << parag->styleName() << "' not found" << endl;

    // No "following style" setting, or same style -> copy layout & format of previous paragraph
    if (!styleApplied)
    {
        setParagLayout( parag->paragLayout() );
        setFormat( parag->paragFormat() );
        // QTextCursor::splitAndInsertEmptyParag takes care of setting the format
        // for the chars in the new parag
    }

    // Note: we don't call QTextParag::copyParagData on purpose.
    // We don't want setListStyle to get called - it ruins our stylesheetitems
    // And we don't care about copying the stylesheetitems directly,
    // applying the parag layout will create them
}

void KWTextParag::setCustomItem( int index, QTextCustomItem * custom, QTextFormat * currentFormat )
{
    if ( currentFormat )
        setFormat( index, 1, currentFormat );
    at( index )->setCustomItem( custom );
    addCustomItem();
    document()->registerCustomItem( custom, this );
    invalidate( 0 );
}

void KWTextParag::removeCustomItem( int index )
{
    ASSERT( at( index )->isCustom() );
    QTextCustomItem * item = at( index )->customItem();
    at( index )->loseCustomItem();
    remove( index, 1 );
    QTextParag::removeCustomItem();
    document()->unregisterCustomItem( item, this );
}

int KWTextParag::findCustomItem( QTextCustomItem * custom ) const
{
    int len = string()->length();
    for ( int i = 0; i < len; ++i )
    {
        QTextStringChar & ch = string()->at(i);
        if ( ch.isCustom() && ch.customItem() == custom )
            return i;
    }
    kdDebug() << "KWTextParag::findCustomItem custom item " << custom
              << " not found in paragraph " << this << " " << paragId() << endl;
    return 0;
}

void KWTextParag::setTabList( const KoTabulatorList &tabList )
{
    // TODO support for centered tabs, right-aligned tabs etc.
    m_layout.setTabList( tabList );
    KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
    KoTabulatorList::ConstIterator it = tabList.begin();
    QValueList<int> sortedTabs;
    for ( int i = 0; it != tabList.end() ; ++it, ++i )
        sortedTabs.append( (int)doc->zoomItX( (*it).ptPos ) );
    qHeapSort( sortedTabs );
    int * tabs = new int( tabList.count() ); // will be deleted by ~QTextParag
    QValueList<int>::Iterator it2 = sortedTabs.begin();
    for ( int i = 0; it2 != sortedTabs.end() ; ++it2, ++i )
        tabs[i] = *it2;
    delete [] tabArray();
    setTabArray( tabs );
    invalidate( 0 );
}

//static
QDomElement KWTextParag::saveFormat( QDomDocument & doc, QTextFormat * curFormat, QTextFormat * refFormat, int pos, int len )
{
    QDomElement formatElem = doc.createElement( "FORMAT" );
    formatElem.setAttribute( "id", 1 ); // text format
    if ( len ) // 0 when saving the format of a style
    {
        formatElem.setAttribute( "pos", pos );
        formatElem.setAttribute( "len", len );
    }
    QDomElement elem;
    if( !refFormat || curFormat->font().weight() != refFormat->font().weight() )
    {
        elem = doc.createElement( "WEIGHT" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", curFormat->font().weight() );
    }
    if( !refFormat || curFormat->color() != refFormat->color() )
    {
        elem = doc.createElement( "COLOR" );
        formatElem.appendChild( elem );
        elem.setAttribute( "red", curFormat->color().red() );
        elem.setAttribute( "green", curFormat->color().green() );
        elem.setAttribute( "blue", curFormat->color().blue() );
    }
    if( !refFormat || curFormat->font().family() != refFormat->font().family() )
    {
        elem = doc.createElement( "FONT" );
        formatElem.appendChild( elem );
        elem.setAttribute( "name", curFormat->font().family() );
    }
    if( !refFormat || curFormat->font().pointSize() != refFormat->font().pointSize() )
    {
        elem = doc.createElement( "SIZE" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", curFormat->font().pointSize() );
    }
    if( !refFormat || curFormat->font().italic() != refFormat->font().italic() )
    {
        elem = doc.createElement( "ITALIC" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", static_cast<int>(curFormat->font().italic()) );
    }
    if( !refFormat || curFormat->font().underline() != refFormat->font().underline() )
    {
        elem = doc.createElement( "UNDERLINE" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", static_cast<int>(curFormat->font().underline()) );
    }
    if( !refFormat || curFormat->font().strikeOut() != refFormat->font().strikeOut() )
    {
        elem = doc.createElement( "STRIKEOUT" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", static_cast<int>(curFormat->font().strikeOut()) );
    }
    if( !refFormat || curFormat->vAlign() != refFormat->vAlign() )
    {
        elem = doc.createElement( "VERTALIGN" );
        formatElem.appendChild( elem );
        elem.setAttribute( "value", static_cast<int>(curFormat->vAlign()) );
    }
    return formatElem;
}

void KWTextParag::save( QDomElement &parentElem, int from /* default 0 */, int to /* default -1 */ )
{
    QDomDocument doc = parentElem.ownerDocument();
    QDomElement paragElem = doc.createElement( "PARAGRAPH" );
    parentElem.appendChild( paragElem );
    QDomElement textElem = doc.createElement( "TEXT" );
    paragElem.appendChild( textElem );
    QString text = string()->toString();
    ASSERT( text.right(1)[0] == ' ' );
    if ( to == -1 )
        to = string()->length()-2; // 'to' is inclusive, and length-1 is the trailing space
    textElem.appendChild( doc.createTextNode( text.mid( from, to - from + 1 ) ) );

    QDomElement formatsElem = doc.createElement( "FORMATS" );
    int startPos = -1;
    int index = 0; // Usually same as 'i' but if from>0, 'i' indexes the parag's text and this one indexes the output
    QTextFormat *curFormat = paragFormat();
    for ( int i = from; i <= to; ++i, ++index )
    {
        QTextStringChar & ch = string()->at(i);
        if ( ch.isCustom() )
        {
            if ( startPos > -1 && curFormat) { // Save former format
                QDomElement formatElem = saveFormat( doc, curFormat, paragFormat(), startPos, index-startPos );
                if ( !formatElem.firstChild().isNull() ) // Don't save an empty format tag
                    formatsElem.appendChild( formatElem );
            }

            QDomElement formatElem = doc.createElement( "FORMAT" );
            formatsElem.appendChild( formatElem );
            formatElem.setAttribute( "pos", index );
            formatElem.setAttribute( "len", 1 );
            saveCustomItem( formatElem, ch.customItem() );
            startPos = -1;
        }
        else
        {
            QTextFormat * newFormat = ch.format();
            if ( newFormat != curFormat )
            {
                // Format changed.
                if ( startPos > -1 && curFormat) { // Save former format
                    QDomElement formatElem = saveFormat( doc, curFormat, paragFormat(), startPos, index-startPos );
                    if ( !formatElem.firstChild().isNull() ) // Don't save an empty format tag
                        formatsElem.appendChild( formatElem );
                }

                // Format different from paragraph's format ?
                if( newFormat != paragFormat() )
                {
                    startPos = index;
                    curFormat = newFormat;
                }
                else
                {
                    startPos = -1;
                    curFormat = paragFormat();
                }
            }
        }
    }
    if ( startPos > -1 && index > startPos && curFormat) { // Save last format
        QDomElement formatElem = saveFormat( doc, curFormat, paragFormat(), startPos, index-startPos );
        if ( !formatElem.firstChild().isNull() ) // Don't save an empty format tag
            formatsElem.appendChild( formatElem );
    }

    if (!formatsElem.firstChild().isNull()) // Do we have formats to save ?
        paragElem.appendChild( formatsElem );


    QDomElement layoutElem = doc.createElement( "LAYOUT" );
    paragElem.appendChild( layoutElem );

    m_layout.save( layoutElem );

    // Paragraph's format
    // ## Maybe we should have a "default format" somewhere and
    // pass it instead of 0L, to only save the non-default attributes
    QDomElement paragFormatElement = saveFormat( doc, paragFormat(), 0L, 0, to - from + 1 );
    layoutElem.appendChild( paragFormatElement );
}

void KWTextParag::saveCustomItem( QDomElement & formatElem, QTextCustomItem * item )
{
    // Is it an image ?
    KWTextImage * ti = dynamic_cast<KWTextImage *>( item );
    if ( ti )
    {
        formatElem.setAttribute( "id", 2 ); // code for a picture
        QDomElement imageElem = formatElem.ownerDocument().createElement( "IMAGE" );
        formatElem.appendChild( imageElem );
        QDomElement elem = formatElem.ownerDocument().createElement( "FILENAME" );
        imageElem.appendChild( elem );
        elem.setAttribute( "value", ti->image().key() );
        return;
    }

    // Is it an anchor ?
    KWAnchor * anchor = dynamic_cast<KWAnchor *>( item );
    if ( anchor )
    {
        formatElem.setAttribute( "id", 6 ); // code for an anchor
        QDomElement anchorElem = formatElem.ownerDocument().createElement( "ANCHOR" );
        formatElem.appendChild( anchorElem );
        anchorElem.setAttribute( "type", "frameset" ); // the only possible value currently
        KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
        // ## TODO save the frame number as well ? Only the first frame ? to be determined
        // ## or maybe use len=<number of frames>. Difficult :}
        int num = doc->getFrameSetNum( anchor->frame()->getFrameSet() );
        anchorElem.setAttribute( "instance", num );
        return;
    }
    // else ....
}

//static
QTextFormat KWTextParag::loadFormat( QDomElement &formatElem, QTextFormat * refFormat, const QFont & defaultFont )
{
    QTextFormat format;
    if ( refFormat )
        format = *refFormat;
    QFont font = format.font();
    QDomElement elem;
    elem = formatElem.namedItem( "FONT" ).toElement();
    if ( !elem.isNull() )
    {
        font.setFamily( elem.attribute("name") );
    }
    else if ( !refFormat )
    {   // No reference format and no FONT tag -> use default font
        font = defaultFont;
    }
    elem = formatElem.namedItem( "WEIGHT" ).toElement();
    if ( !elem.isNull() )
        font.setWeight( elem.attribute( "value" ).toInt() );
    elem = formatElem.namedItem( "SIZE" ).toElement();
    if ( !elem.isNull() )
        font.setPointSize( elem.attribute("value").toInt() );
    elem = formatElem.namedItem( "ITALIC" ).toElement();
    if ( !elem.isNull() )
        font.setItalic( elem.attribute("value").toInt() == 1 );
    elem = formatElem.namedItem( "UNDERLINE" ).toElement();
    if ( !elem.isNull() )
        font.setUnderline( elem.attribute("value").toInt() == 1 );
    elem = formatElem.namedItem( "STRIKEOUT" ).toElement();
    if ( !elem.isNull() )
        font.setStrikeOut( elem.attribute("value").toInt() == 1 );

    format.setFont( font );

    elem = formatElem.namedItem( "VERTALIGN" ).toElement();
    if ( !elem.isNull() )
        format.setVAlign( static_cast<QTextFormat::VerticalAlignment>( elem.attribute("value").toInt() ) );
    elem = formatElem.namedItem( "COLOR" ).toElement();
    if ( !elem.isNull() )
    {
        QColor col( elem.attribute("red").toInt(),
                    elem.attribute("green").toInt(),
                    elem.attribute("blue").toInt() );
        format.setColor( col );
    }
    //kdDebug() << "KWTextParag::loadLayout format=" << format.key() << endl;
    return format;
}

void KWTextParag::loadLayout( QDomElement & attributes )
{
    QDomElement layout = attributes.namedItem( "LAYOUT" ).toElement();
    if ( !layout.isNull() )
    {
        KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
        KWParagLayout paragLayout( layout, doc );
        setParagLayout( paragLayout );

        // Load default format from style.
        KWStyle *existingStyle = doc->findStyle( m_layout.styleName(), true );
        if ( !existingStyle )
            kdDebug() << "KWTextParag::loadLayout no style named " << m_layout.styleName() << " found!" << endl;
        QTextFormat *defaultFormat = existingStyle ? &existingStyle->format() : 0L;
        QDomElement formatElem = layout.namedItem( "FORMAT" ).toElement();
        if ( !formatElem.isNull() )
        {
            // Load paragraph format
            QTextFormat f = loadFormat( formatElem, defaultFormat, doc->defaultFont() );
            setFormat( document()->formatCollection()->format( &f ) );
        }
        else // No paragraph format
        {
            if ( defaultFormat ) // -> use the one from the style
                setFormat( document()->formatCollection()->format( defaultFormat ) );
        }
    }
    else
    {
        // Even the simplest import filter should do <LAYOUT><NAME value="Standard"/></LAYOUT>
        kdWarning(32001) << "No LAYOUT tag in PARAGRAPH, dunno what layout to apply" << endl;
    }
}

void KWTextParag::load( QDomElement &attributes )
{
    loadLayout( attributes );

    // Set the text after setting the paragraph format - so that the format applies
    QDomElement element = attributes.namedItem( "TEXT" ).toElement();
    if ( !element.isNull() )
    {
        append( element.text() );
        // Apply default format - this should be automatic !!
        setFormat( 0, string()->length(), paragFormat(), TRUE );
    }

    loadFormatting( attributes );

    setChanged( true );
    invalidate( 0 );
}

void KWTextParag::loadFormatting( QDomElement &attributes, int offset )
{
    KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
    QDomElement formatsElem = attributes.namedItem( "FORMATS" ).toElement();
    if ( !formatsElem.isNull() )
    {
        QDomElement formatElem = formatsElem.firstChild().toElement();
        for ( ; !formatElem.isNull() ; formatElem = formatElem.nextSibling().toElement() )
        {
            if ( formatElem.tagName() == "FORMAT" )
            {
                int index = formatElem.attribute( "pos" ).toInt() + offset;
                int len = formatElem.attribute( "len" ).toInt();

                int id = formatElem.attribute( "id" ).toInt();
                switch( id ) {
                case 1: // Normal text
                {
                    QTextFormat f = loadFormat( formatElem, paragFormat(), doc->defaultFont() );
                    //kdDebug(32002) << "KWTextParag::loadFormatting applying formatting from " << index << " to " << index+len << endl;
                    setFormat( index, len, document()->formatCollection()->format( &f ) );
                    break;
                }
                case 2: // Picture
                {
                    ASSERT( len == 1 );
                    KWTextImage * custom = new KWTextImage( textDocument(), QString::null );
                    kdDebug() << "KWTextParag::loadFormatting insertCustomItem" << endl;
                    setCustomItem( index, custom, paragFormat() );
                    // <IMAGE>
                    QDomElement image = formatElem.namedItem( "IMAGE" ).toElement();
                    if ( !image.isNull() ) {
                        // <FILENAME>
                        QDomElement filenameElement = image.namedItem( "FILENAME" ).toElement();
                        if ( !filenameElement.isNull() )
                        {
                            QString filename = filenameElement.attribute( "value" );
                            doc->addImageRequest( filename, custom );
                        }
                        else
                            kdError(32001) << "Missing FILENAME tag in IMAGE" << endl;
                    } else
                        kdError(32001) << "Missing IMAGE tag in FORMAT wth id=2" << endl;

                    break;
                }
                case 6:
                {
                    ASSERT( len == 1 );
                    QDomElement anchorElem = formatElem.namedItem( "ANCHOR" ).toElement();
                    if ( !anchorElem.isNull() ) {
                        QString type = anchorElem.attribute( "type" );
                        if ( type == "grpMgr" ) // old syntax
                            // TODO, set table floating
                            kdWarning() << "floating tables not implemented yet" << endl;
                        else if ( type == "frameset" )
                        {
                            int num = anchorElem.attribute( "instance" ).toInt();
                            KWAnchorPosition pos;
                            pos.textfs = textDocument()->textFrameSet();
                            pos.parag = this;
                            pos.index = index;
                            doc->addAnchorRequest( num, pos );
                        }
                        else
                            kdWarning() << "Anchor type not supported: " << type << endl;
                    }
                    else
                        kdWarning() << "Missing ANCHOR tag" << endl;
                    break;
                }
                default:
                    kdWarning() << "KWTextParag::loadFormat id=" << id << " not supported" << endl;
                    break;
                }
            }
        }
    }
}

void KWTextParag::setParagLayout( const KWParagLayout & layout )
{
    setAlign( layout.alignment );
    setMargins( layout.margins );
    setLinesTogether( layout.linesTogether );
    setLineSpacing( layout.lineSpacing );
    setLeftBorder( layout.leftBorder );
    setRightBorder( layout.rightBorder );
    setTopBorder( layout.topBorder );
    setBottomBorder( layout.bottomBorder );
    setCounter( layout.counter );

    setTabList( layout.tabList() );
    // Don't call setStyle from here, it would overwrite any paragraph-specific settings
    setStyleName( layout.styleName() );
}

#ifndef NDEBUG
void KWTextParag::printRTDebug( int info )
{
    kdDebug() << "Paragraph " << this << "   (" << paragId() << ") ------------------ " << endl;
    if ( prev() && prev()->paragId() + 1 != paragId() )
        kdWarning() << "Previous paragraph " << prev() << " has ID " << prev()->paragId() << endl;
    /*
      static const char * dm[] = { "DisplayBlock", "DisplayInline", "DisplayListItem", "DisplayNone" };
      QVector<QStyleSheetItem> vec = styleSheetItems();
      for ( uint i = 0 ; i < vec.size() ; ++i )
      {
      QStyleSheetItem * item = vec[i];
      kdDebug() << "  StyleSheet Item " << item << " '" << item->name() << "'" << endl;
      kdDebug() << "        italic=" << item->fontItalic() << " underline=" << item->fontUnderline() << " fontSize=" << item->fontSize() << endl;
      kdDebug() << "        align=" << item->alignment() << " leftMargin=" << item->margin(QStyleSheetItem::MarginLeft) << " rightMargin=" << item->margin(QStyleSheetItem::MarginRight) << " topMargin=" << item->margin(QStyleSheetItem::MarginTop) << " bottomMargin=" << item->margin(QStyleSheetItem::MarginBottom) << endl;
      kdDebug() << "        displaymode=" << dm[item->displayMode()] << endl;
      }*/
    kdDebug() << "  Style: " << styleName() << endl;
    kdDebug() << "  Text: '" << string()->toString() << "'" << endl;
    if ( info == 0 ) // paragraph info
    {
        if ( counter() )
            kdDebug() << "  Counter style=" << counter()->style() << " depth=" << counter()->depth() << " text=" << m_layout.counter->text( this ) << " width=" << m_layout.counter->width( this ) << endl;
        kdDebug() << "rect() : " << DEBUGRECT( rect() ) << endl;
    } else if ( info == 1 ) // formatting info
    {
        kdDebug() << "  Paragraph format=" << paragFormat() << " " << paragFormat()->key()
                  << " fontsize:" << dynamic_cast<KWTextFormat *>(paragFormat())->pointSizeFloat() << endl;

        QTextString * s = string();
        for ( int i = 0 ; i < s->length() ; ++i )
            kdDebug() << i << ": '" << QString(s->at(i).c) << "'" << s->at(i).format()
                      << " " << s->at(i).format()->key()
                //<< " fontsize:" << dynamic_cast<KWTextFormat *>(s->at(i).format())->pointSizeFloat()
                      << endl;
    }

    /*KoTabulatorList tabList = m_layout.tabList();
      KoTabulatorList::Iterator it = tabList.begin();
      for ( ; it != tabList.end() ; it++ )
      kdDebug() << "Tab at: " << (*it).ptPos << endl;*/
}
#endif

//////////

// Create a default KWParagLayout.
KWParagLayout::KWParagLayout()
{
    initialise();
}

void KWParagLayout::operator=( const KWParagLayout &layout )
{
    alignment = layout.alignment;
    for ( int i = 0 ; i < 5 ; ++i )
        margins[i] = layout.margins[i];
    linesTogether = layout.linesTogether;
    leftBorder = layout.leftBorder;
    rightBorder = layout.rightBorder;
    topBorder = layout.topBorder;
    bottomBorder = layout.bottomBorder;
    if ( layout.counter )
        counter = new Counter( *layout.counter );
    else
        counter = 0L;
    lineSpacing = layout.lineSpacing;
    setStyleName( layout.styleName() );
    setTabList( layout.tabList() );
}

// Create a KWParagLayout from XML.
//
// If a document is supplied, default values are taken from the style in the
// document named by the layout. This allows for simplified import filters,
// and also looks to the day that redundant data can be eliminated from the
// saved XML.
KWParagLayout::KWParagLayout( QDomElement & parentElem, KWDocument *doc )
{
    initialise();

    // Name of the style. If there is no style, then we do not supply
    // any default!
    QDomElement element = parentElem.namedItem( "NAME" ).toElement();
    if ( !element.isNull() )
    {
        m_styleName = element.attribute( "value" );
        if ( doc )
        {
            // Default all the layout stuff from the style.
            KWStyle *existingStyle = doc->findStyle( m_styleName );
            if (existingStyle)
            {
                *this = existingStyle->paragLayout();
            }
            else
            {
                kdError(32001) << "Cannot find style \"" << m_styleName << "\"" << endl;
            }
        }
    }
    else
    {
        kdError(32001) << "Missing NAME tag in LAYOUT" << endl;
    }

    // Load the paragraph tabs - forget about the ones from the style first.
    // We can't apply the 'default comes from the style' in this case, because
    // there is no way to differenciate between "I want no tabs in the parag"
    // and "use default from style".
    m_tabList.clear();
    QDomNodeList listTabs = parentElem.elementsByTagName ( "TABULATOR" );
    unsigned int count = listTabs.count();
    for (unsigned int item = 0; item < count; item++)
    {
        element = listTabs.item( item ).toElement();
        KoTabulator tab;
        tab.type = static_cast<KoTabulators>( KWDocument::getAttribute( element, "type", T_LEFT ) );
        tab.ptPos = KWDocument::getAttribute( element, "ptpos", 0.0 );
        m_tabList.append( tab );
    }
    alignment = Qt::AlignLeft;
    element = parentElem.namedItem( "FLOW" ).toElement(); // Flow is what is now called alignment internally
    if ( !element.isNull() )
    {
        QString flow = element.attribute( "value" ); // KWord-0.8
        if ( !flow.isEmpty() )
        {
            static const int flow2align[] = { Qt::AlignLeft, Qt::AlignRight, Qt::AlignCenter, Qt3::AlignJustify };
            alignment = flow2align[flow.toInt()];
        } else {
            flow = element.attribute( "align" ); // KWord-1.0 DTD
            alignment = flow=="right" ? (int)Qt::AlignRight : flow=="center" ? (int)Qt::AlignCenter : flow=="justify" ? (int)Qt3::AlignJustify : (int)Qt::AlignLeft;
        }
    }

    element = parentElem.namedItem( "OHEAD" ).toElement(); // used by KWord-0.8
    if ( !element.isNull() )
        margins[QStyleSheetItem::MarginTop] = KWDocument::getAttribute( element, "pt", 0.0 );

    element = parentElem.namedItem( "OFOOT" ).toElement(); // used by KWord-0.8
    if ( !element.isNull() )
        margins[QStyleSheetItem::MarginBottom] = KWDocument::getAttribute( element, "pt", 0.0 );

    element = parentElem.namedItem( "IFIRST" ).toElement(); // used by KWord-0.8
    if ( !element.isNull() )
        margins[QStyleSheetItem::MarginFirstLine] = KWDocument::getAttribute( element, "pt", 0.0 );

    element = parentElem.namedItem( "ILEFT" ).toElement(); // used by KWord-0.8
    if ( !element.isNull() )
        margins[QStyleSheetItem::MarginLeft] = KWDocument::getAttribute( element, "pt", 0.0 );

    // KWord-1.0 DTD
    element = parentElem.namedItem( "INDENTS" ).toElement();
    if ( !element.isNull() )
    {
        margins[QStyleSheetItem::MarginFirstLine] = KWDocument::getAttribute( element, "first", 0.0 );
        margins[QStyleSheetItem::MarginLeft] = KWDocument::getAttribute( element, "left", 0.0 );
        margins[QStyleSheetItem::MarginRight] = KWDocument::getAttribute( element, "right", 0.0 );
    }
    element = parentElem.namedItem( "OFFSETS" ).toElement();
    if ( !element.isNull() )
    {
        margins[QStyleSheetItem::MarginTop] = KWDocument::getAttribute( element, "before", 0.0 );
        margins[QStyleSheetItem::MarginBottom] = KWDocument::getAttribute( element, "after", 0.0 );
    }

    element = parentElem.namedItem( "LINESPACE" ).toElement(); // used by KWord-0.8
    if ( !element.isNull() )
        lineSpacing = KWDocument::getAttribute( element, "pt", 0.0 );

    element = parentElem.namedItem( "LINESPACING" ).toElement(); // KWord-1.0 DTD
    if ( !element.isNull() )
        lineSpacing = KWDocument::getAttribute( element, "value", 0.0 );


    element = parentElem.namedItem( "PAGEBREAKING" ).toElement();
    if ( !element.isNull() )
        linesTogether = element.attribute( "linesTogether" ) == "true";


    element = parentElem.namedItem( "LEFTBORDER" ).toElement();
    if ( !element.isNull() )
        leftBorder = Border::loadBorder( element );
    else
        leftBorder.ptWidth = 0;

    element = parentElem.namedItem( "RIGHTBORDER" ).toElement();
    if ( !element.isNull() )
        rightBorder = Border::loadBorder( element );
    else
        rightBorder.ptWidth = 0;

    element = parentElem.namedItem( "TOPBORDER" ).toElement();
    if ( !element.isNull() )
        topBorder = Border::loadBorder( element );
    else
        topBorder.ptWidth = 0;

    element = parentElem.namedItem( "BOTTOMBORDER" ).toElement();
    if ( !element.isNull() )
        bottomBorder = Border::loadBorder( element );
    else
        bottomBorder.ptWidth = 0;

    element = parentElem.namedItem( "COUNTER" ).toElement();
    if ( !element.isNull() )
    {
        counter = new Counter;
        counter->load( element );
    }
}

void KWParagLayout::initialise()
{
    alignment = Qt::AlignLeft;
    for ( int i = 0 ; i < 5 ; ++i ) // use memset ?
        margins[i] = 0;
    lineSpacing = 0;
    counter = 0L;
    leftBorder.ptWidth = 0;
    rightBorder.ptWidth = 0;
    topBorder.ptWidth = 0;
    bottomBorder.ptWidth = 0;
    linesTogether = false;
    m_tabList.clear();
}

KWParagLayout::~KWParagLayout()
{
    delete counter;
}

void KWParagLayout::save( QDomElement & parentElem )
{
    QDomDocument doc = parentElem.ownerDocument();
    QDomElement element = doc.createElement( "NAME" );
    parentElem.appendChild( element );
    element.setAttribute( "value", m_styleName );

    element = doc.createElement( "FLOW" );
    parentElem.appendChild( element );
    int a = alignment;
    element.setAttribute( "align", a==Qt::AlignRight ? "right" : a==Qt::AlignCenter ? "center" : a==Qt3::AlignJustify ? "justify" : "left" );
    if ( margins[QStyleSheetItem::MarginFirstLine] != 0 ||
         margins[QStyleSheetItem::MarginLeft] != 0 ||
         margins[QStyleSheetItem::MarginRight] != 0 )
    {
        element = doc.createElement( "INDENTS" );
        parentElem.appendChild( element );
        if ( margins[QStyleSheetItem::MarginFirstLine] != 0 )
            element.setAttribute( "first", margins[QStyleSheetItem::MarginFirstLine] );
        if ( margins[QStyleSheetItem::MarginLeft] != 0 )
            element.setAttribute( "left", margins[QStyleSheetItem::MarginLeft] );
        if ( margins[QStyleSheetItem::MarginRight] != 0 )
            element.setAttribute( "right", margins[QStyleSheetItem::MarginRight] );
    }

    if ( margins[QStyleSheetItem::MarginTop] != 0 ||
         margins[QStyleSheetItem::MarginBottom] != 0 )
    {
        element = doc.createElement( "OFFSETS" );
        parentElem.appendChild( element );
        if ( margins[QStyleSheetItem::MarginTop] != 0 )
            element.setAttribute( "before", margins[QStyleSheetItem::MarginTop] );
        if ( margins[QStyleSheetItem::MarginBottom] != 0 )
            element.setAttribute( "after", margins[QStyleSheetItem::MarginBottom] );
    }

    if ( lineSpacing != 0 )
    {
        element = doc.createElement( "LINESPACING" );
        parentElem.appendChild( element );
        element.setAttribute( "value", lineSpacing );
    }

    if ( linesTogether )
    {
        element = doc.createElement( "PAGEBREAKING" );
        parentElem.appendChild( element );
        element.setAttribute( "linesTogether", linesTogether ? "true" : "false" );
    }

    if ( leftBorder.ptWidth > 0 )
    {
        element = doc.createElement( "LEFTBORDER" );
        parentElem.appendChild( element );
        leftBorder.save( element );
    }
    if ( rightBorder.ptWidth > 0 )
    {
        element = doc.createElement( "RIGHTBORDER" );
        parentElem.appendChild( element );
        rightBorder.save( element );
    }
    if ( topBorder.ptWidth > 0 )
    {
        element = doc.createElement( "TOPBORDER" );
        parentElem.appendChild( element );
        topBorder.save( element );
    }
    if ( bottomBorder.ptWidth > 0 )
    {
        element = doc.createElement( "BOTTOMBORDER" );
        parentElem.appendChild( element );
        bottomBorder.save( element );
    }
    if ( counter && counter->numbering() != Counter::NUM_NONE )
    {
        element = doc.createElement( "COUNTER" );
        parentElem.appendChild( element );
        counter->save( element );
    }
}

void KWParagLayout::setStyleName( const QString &styleName )
{
    m_styleName = styleName;
}
