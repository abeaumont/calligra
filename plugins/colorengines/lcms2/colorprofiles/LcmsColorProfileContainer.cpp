/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "LcmsColorProfileContainer.h"

#include <cfloat>
#include <cmath>

#include "DebugPigment.h"

class LcmsColorProfileContainer::Private
{
public:
    Private()
        : valid(false)
        , suitableForOutput(false) { }

    cmsHPROFILE profile;
    cmsColorSpaceSignature colorSpaceSignature;
    cmsProfileClassSignature deviceClass;
    QString productDescription;
    QString manufacturer;
    QString name;
    IccColorProfile::Data * data;
    bool valid;
    bool suitableForOutput;
    bool hasColorants;
    cmsCIEXYZ mediaWhitePoint;
    cmsCIExyY whitePoint;
    cmsCIEXYZTRIPLE colorants;
};

LcmsColorProfileContainer::LcmsColorProfileContainer()
    : d(new Private())
{
    d->profile = 0;
}

LcmsColorProfileContainer::LcmsColorProfileContainer(IccColorProfile::Data * data)
    : d(new Private())
{
    d->data = data;
    d->profile = 0;
    init();
}

QByteArray LcmsColorProfileContainer::lcmsProfileToByteArray(const cmsHPROFILE profile)
{
    cmsUInt32Number  bytesNeeded = 0;
    // Make a raw data image ready for saving
    cmsSaveProfileToMem(profile, 0, &bytesNeeded); // calc size
    QByteArray rawData;
    rawData.resize(bytesNeeded);
    if (rawData.size() >= (int)bytesNeeded) {
        cmsSaveProfileToMem(profile, rawData.data(), &bytesNeeded); // fill buffer
    } else {
        errorPigment << "Couldn't resize the profile buffer, system is probably running out of memory.";
        rawData.resize(0);
    }
    return rawData;
}

IccColorProfile* LcmsColorProfileContainer::createFromLcmsProfile(const cmsHPROFILE profile)
{
    IccColorProfile* iccprofile = new IccColorProfile(lcmsProfileToByteArray(profile));
    cmsCloseProfile(profile);
    return iccprofile;
}

LcmsColorProfileContainer::~LcmsColorProfileContainer()
{
    cmsCloseProfile(d->profile);
    delete d;
}

#define _BUFFER_SIZE_ 1000

bool LcmsColorProfileContainer::init()
{
    if (d->profile) cmsCloseProfile(d->profile);

    d->profile = cmsOpenProfileFromMem((void*)d->data->rawData().constData(), d->data->rawData().size());

#ifndef NDEBUG
    if (d->data->rawData().size() == 4096) {
        warnPigment << "Profile has a size of 4096, which is suspicious and indicates a possible misuse of QIODevice::read(int), check your code.";
    }
#endif

    if (d->profile) {
        wchar_t buffer[_BUFFER_SIZE_];
        d->colorSpaceSignature = cmsGetColorSpace(d->profile);
        d->deviceClass = cmsGetDeviceClass(d->profile);
        cmsGetProfileInfo(d->profile, cmsInfoDescription, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->name = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoModel, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->productDescription = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoManufacturer, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->manufacturer = QString::fromWCharArray(buffer);
        
        cmsProfileClassSignature profile_class;
        profile_class = cmsGetDeviceClass(d->profile);
        d->valid = (profile_class != cmsSigNamedColorClass);
        
        if (cmsIsTag(d->profile, cmsSigMediaWhitePointTag)) {
            d->mediaWhitePoint = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigMediaWhitePointTag));
            cmsXYZ2xyY(&d->whitePoint, &d->mediaWhitePoint);
            //qDebug()<<d->name<<" Whitepoint: "<<WhitePoint.x<<","<<WhitePoint.y<<","<<WhitePoint.Y;
        }
        if (cmsIsTag(d->profile, cmsSigRedColorantTag)) {
            cmsCIEXYZTRIPLE tempColorants = { -1 };
            tempColorants.Red = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigRedColorantTag));
            tempColorants.Green = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigGreenColorantTag));
            tempColorants.Blue = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigBlueColorantTag));
            d->colorants = tempColorants;
            d->hasColorants = true;
            //TODO: convert to d65, this is useless.
            //cmsAdaptMatrixFromD50(&tempColorants, &WhitePoint);
            //cmsXYZ2xyY(&(d->Primaries.Green), &tempColor);
            //qDebug()<<d->name<<": "<<tempColorants.Red.Y<<","<<tempColorants.Green.Y<<","<<tempColorants.Blue.Y;
        } else {
        //qDebug()<<d->name<<": has no colorants";
        d->hasColorants = false;
        }
        
        
        
        // Check if the profile can convert (something->this)
        d->suitableForOutput = cmsIsMatrixShaper(d->profile)
                || ( cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_INPUT) &&
                     cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_OUTPUT) );

        dbgPigment << "Loaded ICC Profile"
                   << "\n\tSignature:" << d->colorSpaceSignature
                   << "\n\tDevice class:" << d->deviceClass
                   << "\n\tDescription:" << d->productDescription
                   << "\n\tValid:" << d->valid
                   << "\n\tName:" << d->name
                   << "\n\tManufacturer:" << d->manufacturer
                   << "\n\tSuitable for output:" << d->suitableForOutput
                   << "\n\tClass" << QString::number(profile_class, 0, 16);

        return true;
    }

    return false;
}

cmsHPROFILE LcmsColorProfileContainer::lcmsProfile() const
{
    return d->profile;
}

cmsColorSpaceSignature LcmsColorProfileContainer::colorSpaceSignature() const
{
    return d->colorSpaceSignature;
}

cmsProfileClassSignature LcmsColorProfileContainer::deviceClass() const
{
    return d->deviceClass;
}

QString LcmsColorProfileContainer::manufacturer() const
{
    return d->manufacturer;
}

bool LcmsColorProfileContainer::valid() const
{
    return d->valid;
}

bool LcmsColorProfileContainer::isSuitableForOutput() const
{
    return d->suitableForOutput;
}

bool LcmsColorProfileContainer::isSuitableForPrinting() const
{
    return deviceClass() == cmsSigOutputClass;
}

bool LcmsColorProfileContainer::isSuitableForDisplay() const
{
    return deviceClass() == cmsSigDisplayClass;
}
bool LcmsColorProfileContainer::hasColorants() const
{
    return d->hasColorants;
}
QVector <double> LcmsColorProfileContainer::getColorantsXYZ() const
{
    QVector <double> colorants(9);
    colorants[0] = d->colorants.Red.X;
    colorants[1] = d->colorants.Red.Y;
    colorants[2] = d->colorants.Red.Z;
    colorants[3] = d->colorants.Green.X;
    colorants[4] = d->colorants.Green.Y;
    colorants[5] = d->colorants.Green.Z;
    colorants[6] = d->colorants.Blue.X;
    colorants[7] = d->colorants.Blue.Y;
    colorants[8] = d->colorants.Blue.Z;
    return colorants;
}

QVector <double> LcmsColorProfileContainer::getColorantsxyY() const
{
    cmsCIEXYZ temp1;
    cmsCIExyY temp2;
    QVector <double> colorants;
    
    temp1.X = d->colorants.Red.X;
    temp1.Y = d->colorants.Red.Y;
    temp1.Z = d->colorants.Red.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[0] = temp2.x;
    colorants[1] = temp2.y;
    colorants[2] = temp2.Y;
    
    temp1.X = d->colorants.Green.X;
    temp1.Y = d->colorants.Green.Y;
    temp1.Z = d->colorants.Green.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[3] = temp2.x;
    colorants[4] = temp2.y;
    colorants[5] = temp2.Y;
    
    temp1.X = d->colorants.Blue.X;
    temp1.Y = d->colorants.Blue.Y;
    temp1.Z = d->colorants.Blue.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[6] = temp2.x;
    colorants[7] = temp2.y;
    colorants[8] = temp2.Y;
    
    return colorants;
}

QVector <double> LcmsColorProfileContainer::getWhitePointXYZ() const
{
    QVector <double> tempWhitePoint(3);
    
    tempWhitePoint[0] = d->mediaWhitePoint.X;
    tempWhitePoint[1] = d->mediaWhitePoint.Y;
    tempWhitePoint[2] = d->mediaWhitePoint.Z;
    
    return tempWhitePoint;
}

QVector <double> LcmsColorProfileContainer::getWhitePointxyY() const
{
    QVector <double> tempWhitePoint(3);
    
    tempWhitePoint[0] = d->whitePoint.x;
    tempWhitePoint[1] = d->whitePoint.y;
    tempWhitePoint[2] = d->whitePoint.Y;
    
    return tempWhitePoint;
}

QString LcmsColorProfileContainer::name() const
{
    return d->name;
}

QString LcmsColorProfileContainer::info() const
{
    return d->productDescription;
}
