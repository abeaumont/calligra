/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "PresentationImpl.h"

#include <stage/part/KPrPart.h>
#include <stage/part/KPrDocument.h>

using namespace Calligra::Components;

class PresentationImpl::Private
{
public:
    Private() : part{nullptr}, document{nullptr}
    { }

    KPrPart* part;
    KPrDocument* document;
};

PresentationImpl::PresentationImpl(QObject* parent)
    : DocumentImpl{parent}, d{new Private}
{

}

PresentationImpl::~PresentationImpl()
{
    delete d;
}

Global::DocumentType PresentationImpl::documentType() const
{
    return Global::PresentationType;
}

bool PresentationImpl::load(const QUrl& url)
{
    if(d->part) {
        delete d->part;
        delete d->document;
    }

    d->part = new KPrPart{this};
    d->document = new KPrDocument{d->part};
    d->part->setDocument(d->document);

    return d->document->openUrl(url);
}

KoFindBase* PresentationImpl::finder() const
{
    return nullptr;
}
