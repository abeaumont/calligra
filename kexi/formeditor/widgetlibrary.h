/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KFORMDESIGNERWIDGETLIBRARY_H
#define KFORMDESIGNERWIDGETLIBRARY_H

#include <qobject.h>
#include <qmap.h>
#include <qdict.h>

#include "widgetfactory.h"

template<class type> class QPtrList;
template<class type> class QValueVector;
class KActionCollection;
class KAction;
class QWidget;
class QPopupMenu;
class QVariant;
class QDomDocument;
class QDomElement;

namespace KFormDesigner {

class Container;
class ObjectTreeItem;
class WidgetLibraryPrivate;

typedef QPtrList<KAction> ActionList;

/**
 * This class searches for factories and provides KActions for widget creation.
 * Every widget can be located using this library.
 * You call WidgetLibrary functions instead of calling directly factories.
 * See WidgetFactory for a description of the functions.
 */
class KFORMEDITOR_EXPORT WidgetLibrary : public QObject
{
	Q_OBJECT

	public:
		/*! Constructs WidgetLibrary object.
		 In \a supportedFactoryGroups you can provide
		 factory group list to be supported. Factory groups are defined by
		 "X-KFormDesigner-FactoryGroup" field in every factory serviece's .desktop file.
		 By default (when supportedFactoryGroups is empty) only factories having empty
		 "X-KFormDesigner-FactoryGroup" field will be loaded.
		 Factory group names are case-insensitive. */
		WidgetLibrary(QObject *parent=0, const QStringList& supportedFactoryGroups = QStringList());

		~WidgetLibrary();

		/**
		 * creates actions for widget creating
		 */
		ActionList addCreateWidgetActions(KActionCollection *parent, QObject *receiver, const char *slot);

		void addCustomWidgetActions(KActionCollection *parent);

		/**
		 * creates the XML for widget actions
		 */
		QString createXML();

		/**
		 * searches the right factory and creates a widget.
		 * @returns the widget or 0 if something falid
		 */
		QWidget *createWidget(const QCString &classname, QWidget *parent, const char *name, Container *c);

		bool createMenuActions(const QCString &c, QWidget *w, QPopupMenu *menu,
			KFormDesigner::Container *container);

		QString displayName(const QCString &classname);
		QString namePrefix(const QCString &classname);
		QString textForWidgetName(const QCString &name, const QCString &className);

		/*! Checks if the \a classname is an alternate classname,
		 and returns the good classname.
		 If \a classname is not alternate, it is returned. */
		QCString checkAlternateName(const QCString &classname);
		QString icon(const QCString &classname);
		QString includeFileName(const QCString &classname);
		QString savingName(const QCString &classname);

		bool startEditing(const QCString &classname, QWidget *w, Container *container);
		bool previewWidget(const QCString &classname, QWidget *widget, Container *container);
		bool clearWidgetContent(const QCString &classname, QWidget *w);

		bool saveSpecialProperty(const QCString &classname, const QString &name,
			const QVariant &value, QWidget *w, QDomElement &parentNode, QDomDocument &parent);
		bool readSpecialProperty(const QCString &classname, QDomElement &node, QWidget *w,
			ObjectTreeItem *item);
		bool isPropertyVisible(const QCString &classname, QWidget *w,
			const QCString &property, bool multiple);

		QValueList<QCString> autoSaveProperties(const QCString &classname);

		WidgetInfo* widgetInfoForClassName(const char* classname);

		WidgetFactory* factoryForClassName(const char* className);

		/*! \return true if advanced properties like "mouseTracking" should 
		 be user-visible. True by default (in KFD), but Kexi set's this to false. 
		 See WidgetLibraryPrivate class implementation for complete list 
		 of advanced properties. */
		bool advancedPropertiesVisible() const;

		void setAdvancedPropertiesVisible(bool set);

	signals:
		void prepareInsert(const QCString &c);

	protected:
		/**
		 * Adds a factory to the library, creates actions for widgets in the added factory.
		 * This function is not called directly but by the factory locater.
		 */
		void loadFactoryWidgets(WidgetFactory *f);

#if 0 //UNIMPLEMENTED
		/**
		 * you can restrict the loaded factories by setting the filter to a pattern
		 * like 'kexi|containers' in that case only factory containing 'kexi' or containers will be loaded.
		 * this is useful if you want to embedd formeditor and provide e.g. a LineEdit with special features
		 * but don't want to confuse the user... are you confused now?
		 * NB: not implemented yet
		 */
		void setFilter(const QRegExp &expr);
#endif

		/**
		 * Lookups widget factories list (note that this function get called once in ctor)
		 */
		void lookupFactories();

		/**
		 * Loads widget factories found in lookupFactories(). This is called once.
		 */
		void loadFactories();

		WidgetLibraryPrivate *d;
};

}
#endif
