/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_FILTER_H_
#define _KIS_FILTER_H_

#include <qobject.h>
#include <qwidget.h>

#include <ksharedptr.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_filter_registry.h"
#include "kis_id.h"
#include "koffice_export.h"

class KisPreviewDialog;

/**
 * Convenience function that creates an instance of a filter and adds it to the
 * filter registry of the specified view.
 */
template<class F>
KisFilterSP createFilter(KisView* view)
{
       KisFilterSP kfi;
       if( view->filterRegistry()->exists( F::id() ) )
       {
               kfi = view->filterRegistry()->get( F::id() );
      } else {
               kfi = new F(view);
	       Q_CHECK_PTR(kfi);
               view->filterRegistry()->add(kfi);
       }
       return kfi;
}



/**
 * Empty interface for passing filter configuration data
 * from the configuration widget to the filter.
 */
class KisFilterConfiguration {
};



/**
 * Basic interface of a Krita filter.
 */
class KRITACORE_EXPORT KisFilter : public KisProgressSubject, public KShared {
	Q_OBJECT
public:

	KisFilter(const KisID& id, KisView * view);
	virtual ~KisFilter() {}

public:

	virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&) = 0;

public:
	virtual KisFilterConfiguration* configuration(QWidget*);

	// This filter can be used in painting tools as a paint operation
	virtual bool supportsPainting() = 0;

	// Can this filter work incrementally when painting, or do we need to work
	// on the state as it was before painting started. The former is faster.
	virtual bool supportsIncrementalPainting() { return true; };
	
	// This filter supports cutting up the work area and filtering
	// each chunk in a separate thread. Filters that need access to the
	// whole area for correct computations should return false.
	
	virtual bool supportsThreading() { return true; };
	
	// Used when threading is used -- the overlap margin is passed to the
	// filter to use to compute pixels, but the margin is not pasted into the
	// resulting image.
	virtual int  overlapMarginNeeded() { return 0; };

	virtual void enableProgress();
	virtual void disableProgress();
	virtual void setAutoUpdate(bool set);

	inline const KisID id() const { return m_id; };

	virtual QWidget* createConfigurationWidget(QWidget* parent);

	virtual void cancel() { m_cancelRequested = true; }

	inline KisView* view();

public slots:

	void slotActivated();

private slots:

	void refreshPreview();

protected:

  	KisStrategyColorSpaceSP colorStrategy();

	bool progressEnabled() const { return m_progressEnabled; }
	bool cancelRequested() const { return m_progressEnabled && m_cancelRequested; }

	// Convenience functions for progress display.
	void setProgressTotalSteps(Q_INT32 totalSteps);
	void setProgress(Q_INT32 progress);
	void incProgress();
	void setProgressStage(const QString& stage, Q_INT32 progress);
	void setProgressDone();

private:
	bool m_cancelRequested;
	bool m_progressEnabled;

protected:
	Q_INT32 m_progressTotalSteps;
	Q_INT32 m_lastProgressPerCent;
	Q_INT32 m_progressSteps;
	
	KisID m_id;
	KisView * m_view;
	QWidget* m_widget;
	KisPreviewDialog* m_dialog;
};

inline KisView* KisFilter::view()
{
	return m_view;
}


inline KisStrategyColorSpaceSP KisFilter::colorStrategy()
{
	if (!m_view) return 0;
	KisImageSP img = m_view -> currentImg();

	if (!img) return 0;
	KisLayerSP layer = img -> activeLayer();

	if (!layer) return 0;
	return layer -> colorStrategy();
}




#endif
