/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS_STRATEGY_COLORSPACE_CMYKA_H_
#define KIS_STRATEGY_COLORSPACE_CMYKA_H_

#include <qcolor.h>
#include <qmap.h>

#include <kis_pixel.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_strategy_colorspace.h>

#include "kis_strategy_colorspace_cmyk.h"

class KisStrategyColorSpaceCMYKA : public KisStrategyColorSpace {

public:
	KisStrategyColorSpaceCMYKA();
	virtual ~KisStrategyColorSpaceCMYKA();

public:

	virtual void nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile = 0);

	virtual void toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const QUANTUM *src, KisProfileSP profile = 0)
		{ return KisPixelRO (src, src + PIXEL_CMYK_ALPHA, this, profile); }
	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile = 0)
		{ return KisPixel (src, src + PIXEL_CMYK_ALPHA, this, profile); }

	virtual Q_INT8 difference(const QUANTUM* src1, const QUANTUM* src2);
	virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
	
	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
	virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

	virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

	virtual void adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const;
	
	virtual void bitBlt(Q_INT32 stride,
			    Q_UINT8 *dst,
			    Q_INT32 dststride,
			    const Q_UINT8 *src,
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    const KisCompositeOp& op);

	virtual bool valid() { return true; }

	KisCompositeOpList userVisiblecompositeOps() const;

private:
	vKisChannelInfoSP m_channels;
};

#endif // KIS_STRATEGY_COLORSPACE_CMYKA_H_
