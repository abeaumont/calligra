/* This file is part of the KDE project
   Copyright (C) 2002, Sven L�ppken <sven@kde.org>

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

#include <svgimport.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <koUnit.h>
#include <koGlobal.h>
#include <shapes/vellipse.h>
#include <shapes/vrectangle.h>
#include <shapes/vpolygon.h>
#include <qcolor.h>
#include <qfile.h>
#include <qregexp.h>
#include <iostream>

typedef KGenericFactory<SvgImport, KoFilter> SvgImportFactory;
K_EXPORT_COMPONENT_FACTORY( libkarbonsvgimport, SvgImportFactory( "karbonsvgimport" ) );

SvgImport::SvgImport(KoFilter *, const char *, const QStringList&) :
    KoFilter(),
    outdoc( "DOC" )
{
}

SvgImport::~SvgImport()
{

}

KoFilter::ConversionStatus SvgImport::convert(const QCString& from, const QCString& to)
{
	// check for proper conversion
	if( to != "application/x-karbon" || from != "image/svg+xml" )
		return KoFilter::NotImplemented;

	QFile in( m_chain->inputFile() );
    if( !in.open( IO_ReadOnly ) )
	{
		kdError(30502) << "Unable to open input file" << endl;
		in.close();
		return KoFilter::FileNotFound;
	}

	QTextStream stream( &in );
	inpdoc.setContent( stream.device() );

	// Do the conversion!

	convert();

	KoStoreDevice* out = m_chain->storageFile( "root", KoStore::Write );
	if( !out ) 
	{
		kdError(30502) << "Unable to open output file!" << endl;
		return KoFilter::StorageCreationError;
	}
	QCString cstring = outdoc.toCString(); // utf-8 already
	out->writeBlock( cstring.data(), cstring.length() );

	in.close();
	return KoFilter::OK; // was successfull
}

void
SvgImport::convert()
{
	QDomElement docElem = inpdoc.documentElement();
	parseGroup( 0L, docElem );
}

void
SvgImport::parseStyle( VObject *obj, const QDomElement &e )
{
	VFill fill;
	VStroke stroke;
	QColor c;
	GraphicsContext *gc = new GraphicsContext;
	// set as default
	if( m_gc.current() )
		*gc = *( m_gc.current() );

	// try normal PA
	if( !e.attribute( "fill" ).isEmpty() )
	{
		c.setNamedColor( e.attribute( "fill" ) );
		VColor color( c );
		fill.setColor( color );
		gc->fill = fill;
	}

	// try style attr
	QString style = e.attribute( "style" ).simplifyWhiteSpace();
	QStringList substyles = QStringList::split( ';', style );
    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
	{
		QStringList substyle = QStringList::split( ':', (*it) );
		QString command	= substyle[0].stripWhiteSpace();
		QString params	= substyle[1].stripWhiteSpace();
		if( command == "fill" )
		{
			c.setNamedColor( params );
			VColor color( c );
			fill.setColor( color );
			gc->fill = fill;
		}
	}

	obj->setFill( gc->fill );
	obj->setStroke( gc->stroke );
	m_gc.push( gc );
}

void
SvgImport::parseGroup( VGroup *grp, const QDomElement &e )
{
	QDomElement b = e.firstChild().toElement();
	for( ; !b.isNull(); b = b.nextSibling().toElement() )
	{
		if( b.tagName() == "g" )
		{
			VGroup *group = new VGroup( grp );
			parseStyle( group, b );
			parseGroup( group, b );
			if( grp )
				grp->append( group );
			else
				m_document.append( group );
			m_gc.pop();
		}
		else if( b.tagName() == "rect" )
		{
			int x = b.attribute( "x" ).toInt();
			int y = b.attribute( "y" ).toInt();
			int width = b.attribute( "width" ).toInt();
			int height = b.attribute( "height" ).toInt();
			VObject *rect = new VRectangle( 0L, KoPoint( x, height + y ) , width, height );
			parseStyle( rect, b );
			if( grp )
				grp->append( rect );
			else
				m_document.append( rect );
			m_gc.pop();
		}
		else if( b.tagName() == "ellipse" )
		{
			//QDomElement object = b.namedItem( "gobject" ).toElement();
			//QDomElement matrix = object.namedItem( "matrix" ).toElement();
			double rx = b.attribute( "rx" ).toDouble();
			double ry = b.attribute( "ry" ).toDouble();
			double left	= b.attribute( "cx" ).toDouble() - ( rx / 2.0 );
			double top	= b.attribute( "cy" ).toDouble() + ( ry / 2.0 );
			// Append the ellipse to the document
			VObject *ellipse = new VEllipse( 0L, KoPoint( left, top ), rx * 2.0, ry * 2.0 );
			parseStyle( ellipse, b );
			if( grp )
				grp->append( ellipse );
			else
				m_document.append( ellipse );
			m_gc.pop();
		}
		else if( b.tagName() == "circle" )
		{
			double r = b.attribute( "r" ).toDouble();
			double left	= b.attribute( "cx" ).toDouble() - ( r / 2.0 );
			double top	= b.attribute( "cy" ).toDouble() + ( r / 2.0 );
			// Append the ellipse to the document
			VObject *circle = new VEllipse( 0L, KoPoint( left, top ), r * 2.0, r * 2.0 );
			parseStyle( circle, b );
			if( grp )
				grp->append( circle );
			else
				m_document.append( circle );
			m_gc.pop();
		}
		else if( b.tagName() == "polyline" || b.tagName() == "polygon" )
		{
			VComposite *path = new VComposite( &m_document );
			bool bFirst = true;

			QString points = b.attribute( "points" ).simplifyWhiteSpace();
			points.replace( QRegExp( "," ), " " );
			points.replace( QRegExp( "\r" ), "" );
		    points.replace( QRegExp( "\n" ), "" );
			QStringList pointList = QStringList::split( ' ', points );
			for( QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++ )
			{
				if( bFirst )
				{
					path->moveTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
					bFirst = false;
				}
				else
					path->lineTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
			}
			if( b.tagName() == "polygon" ) path->close();
			parseStyle( path, b );
			if( grp )
				grp->append( path );
			else
				m_document.append( path );	
			m_gc.pop();
		}
	}
	
	outdoc = m_document.saveXML();
}

#include <svgimport.moc>
