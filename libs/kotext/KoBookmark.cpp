/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoTextInlineRdf.h>
#include <KoTextRangeManager.h>
#include <KoXmlNS.h>

#include <QTextDocument>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>
#include <QWeakPointer>
#include <KDebug>

class KoBookmark::Private
{
public:
    Private(const QTextDocument *doc)
        : document(doc),
          posInDocument(0) { }
    const QTextDocument *document;
    int posInDocument;
    QString name;
};

KoBookmark::KoBookmark(const QTextCursor &cursor)
    : KoTextRange(cursor),
      d(new Private(cursor.block().document()))
{
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoBookmark::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &)
{
    // nothing to paint.
}

void KoBookmark::setName(const QString &name)
{
    d->name = name;
}

QString KoBookmark::name() const
{
    return d->name;
}

bool KoBookmark::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);

    QString bookmarkName = element.attribute("name");
    const QString localName(element.localName());

    if (manager()) {
        // For cut and paste, make sure that the name is unique.
        d->name = createUniqueBookmarkName(manager()->bookmarkManager(), bookmarkName, false);

        if (localName == "bookmark") {
            setPositionOnlyMode(true);
        }
        else if (localName == "bookmark-start") {
            setPositionOnlyMode(false);

            // Add inline Rdf to the bookmark.
            if (element.hasAttributeNS(KoXmlNS::xhtml, "property") || element.hasAttribute("id")) {
                KoTextInlineRdf* inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument*>(d->document), this);
                if (inlineRdf->loadOdf(element)) {
                    setInlineRdf(inlineRdf);
                }
                else {
                    delete inlineRdf;
                    inlineRdf = 0;
                }
            }
        }
        else {
            // something pretty weird going on...
            return false;
        }
        return true;
    }
    return false;
}

void KoBookmark::saveOdf(KoShapeSavingContext &context, int position) const
{
    KoXmlWriter *writer = &context.xmlWriter();

    if (!hasSelection()) {
        writer->startElement("text:bookmark", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        writer->endElement();
    } else if (position == selectionStart()) {
        writer->startElement("text:bookmark-start", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, writer);
        }
        writer->endElement();
    } else if (position == selectionEnd()) {
        writer->startElement("text:bookmark-end", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        writer->endElement();
    }
    // else nothing
}

QString KoBookmark::createUniqueBookmarkName(const KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->retrieveBookmark(ret)) {
            ret = QString("%1_%2").arg(bookmarkName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = bookmarkName;
                else
                    ret = QString("%1_%2").arg(bookmarkName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}

