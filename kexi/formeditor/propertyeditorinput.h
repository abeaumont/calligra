/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PROPERTYEDITORINPUT_H
#define PROPERTYEDITORINPUT_H

#include "propertyeditoreditor.h"

class QLineEdit;
class QSpinBox;

class KFORMEDITOR_EXPORT PropertyEditorInput : public PropertyEditorEditor
{
	Q_OBJECT

	public:
		PropertyEditorInput(QWidget *parent, QVariant::Type type, QVariant vlaue, const char *name=0);
		~PropertyEditorInput() {;}

		virtual QVariant	getValue();

	protected slots:
		void			slotTextChanged(const QString &text);

	protected:
		QLineEdit		*m_lineedit;
};

//INT

class KFORMEDITOR_EXPORT PropertyEditorSpin : public PropertyEditorEditor
{
	Q_OBJECT

	public:
		PropertyEditorSpin(QWidget *parent, QVariant::Type type, QVariant vlaue, const char *name=0);
		~PropertyEditorSpin() {;}

		virtual QVariant	getValue();

	protected:
		QSpinBox		*m_spinBox;
};


#endif
