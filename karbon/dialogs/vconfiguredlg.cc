/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>

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

#include <float.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qcombobox.h>

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <knumvalidator.h>
#include <kspell.h>

#include "karbon_view.h"
#include "karbon_part.h"
#include "karbon_factory.h"

#include "vconfiguredlg.h"


VConfigureDlg::VConfigureDlg( KarbonView* parent )
		: KDialogBase( KDialogBase::IconList, i18n( "Configure" ),
					   KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Default,
					   KDialogBase::Ok )

{
	QVBox * page = addVBoxPage(
					   i18n( "Interface" ), i18n( "Interface" ),
					   BarIcon( "misc", KIcon::SizeMedium ) );

	m_interfacePage = new VConfigInterfacePage( parent, page );

	page = addVBoxPage(
			   i18n( "Misc" ), i18n( "Misc" ),
			   BarIcon( "misc", KIcon::SizeMedium ) );

	m_miscPage = new VConfigMiscPage( parent, page );

	page = addVBoxPage(
			   i18n( "Document" ), i18n( "Document Settings" ),
			   BarIcon( "document", KIcon::SizeMedium ) );

	m_defaultDocPage = new VConfigDefaultPage( parent, page );

	connect( this, SIGNAL( okClicked() ), this, SLOT( slotApply() ) );
}

void VConfigureDlg::slotApply()
{
	m_interfacePage->apply();
	m_miscPage->apply();
	m_defaultDocPage->apply();
}

void VConfigureDlg::slotDefault()
{
	switch( activePageIndex() )
	{
		case 0:
			m_interfacePage->slotDefault();
			break;

		case 1:
			m_miscPage->slotDefault();
			break;

		case 2:
			m_defaultDocPage->slotDefault();
			break;

		default:
			break;
	}
}


VConfigInterfacePage::VConfigInterfacePage( KarbonView* view,
		QWidget* parent, char* name )
		: QWidget( parent, name )
{
	m_view = view;
	m_config = KarbonFactory::instance()->config();

	QVBoxLayout *box = new QVBoxLayout( this );
	box->setMargin( 5 );
	box->setSpacing( 10 );

	m_oldRecentFiles = 10;
	bool oldShowStatusBar = true;

	QGroupBox* tmpQGroupBox = new QGroupBox( this, "GroupBox" );
	tmpQGroupBox->setTitle( i18n( "Interface" ) );

	QVBoxLayout* lay1 = new QVBoxLayout( tmpQGroupBox );
	lay1->setMargin( 20 );
	lay1->setSpacing( 10 );

	if( m_config->hasGroup( "Interface" ) )
	{
		m_config->setGroup( "Interface" );

		m_oldRecentFiles = m_config->readNumEntry(
							   "NbRecentFile", m_oldRecentFiles );

		oldShowStatusBar = m_config->readBoolEntry(
							   "ShowStatusBar" , true );
	}

	m_showStatusBar = new QCheckBox( i18n( "Show status bar" ), tmpQGroupBox );
	m_showStatusBar->setChecked( oldShowStatusBar );
	lay1->addWidget( m_showStatusBar );

	m_recentFiles = new KIntNumInput( m_oldRecentFiles, tmpQGroupBox );
	m_recentFiles->setRange( 1, 20, 1 );
	m_recentFiles->setLabel( i18n( "Number of recent file:" ) );

	lay1->addWidget( m_recentFiles );

	box->addWidget( tmpQGroupBox );
}

void VConfigInterfacePage::apply()
{
	bool showStatusBar = m_showStatusBar->isChecked();

	KarbonPart* part = m_view->part();

	m_config->setGroup( "Interface" );


	int recent = m_recentFiles->value();

	if( recent != m_oldRecentFiles )
	{
		m_config->writeEntry( "NbRecentFile", recent );
		m_view->setNumberOfRecentFiles( recent );
		m_oldRecentFiles = recent;
	}


	bool refreshGUI = false;

	if( showStatusBar != part->showStatusBar() )
	{
		m_config->writeEntry( "ShowStatusBar", showStatusBar );
		part->setShowStatusBar( showStatusBar );
		refreshGUI = true;
	}

	if( refreshGUI )
		part->reorganizeGUI();
}

void VConfigInterfacePage::slotDefault()
{
	m_recentFiles->setValue( 10 );
	m_showStatusBar->setChecked( true );
}


VConfigMiscPage::VConfigMiscPage( KarbonView* view, QVBox* box, char* name )
		: QObject( box->parent(), name )
{
    m_view = view;
    m_config = KarbonFactory::instance()->config();

    KoUnit::Unit unit = view->part()->unit();

    QGroupBox* tmpQGroupBox = new QGroupBox( box, "GroupBox" );
    tmpQGroupBox->setTitle( i18n( "Misc" ) );

    QGridLayout* grid = new QGridLayout(
        tmpQGroupBox, 4, 2, KDialog::marginHint() + 7, KDialog::spacingHint() );

    m_oldUndoRedo = 30;

    QString unitType = KoUnit::unitName( unit );
    //#################"laurent
    //don't load unitType from config file because unit is
    //depend from kword file => unit can be different from config file

    if( m_config->hasGroup( "Misc" ) )
    {
        m_config->setGroup( "Misc" );
        m_oldUndoRedo = m_config->readNumEntry( "UndoRedo", m_oldUndoRedo );
    }

    m_undoRedo = new KIntNumInput( m_oldUndoRedo, tmpQGroupBox );
    m_undoRedo->setLabel( i18n( "Undo/redo limit:" ) );
    m_undoRedo->setRange( 10, 60, 1 );

    grid->addMultiCellWidget( m_undoRedo, 0, 0, 0, 1 );

    grid->addWidget( new QLabel(  i18n(  "Units:" ), tmpQGroupBox ), 1, 0 );

    QStringList listUnit;
    listUnit << KoUnit::unitDescription( KoUnit::U_MM );
    listUnit << KoUnit::unitDescription( KoUnit::U_CM );
    listUnit << KoUnit::unitDescription( KoUnit::U_DM );
    listUnit << KoUnit::unitDescription( KoUnit::U_INCH );
    listUnit << KoUnit::unitDescription( KoUnit::U_PT );
    listUnit << KoUnit::unitDescription( KoUnit::U_PI );
    listUnit << KoUnit::unitDescription( KoUnit::U_DD );
    listUnit << KoUnit::unitDescription( KoUnit::U_CC );

    m_unit = new QComboBox( tmpQGroupBox );
    m_unit->insertStringList( listUnit );
    grid->addWidget( m_unit, 1, 1 );
    m_oldUnit = 0;

    switch( KoUnit::unit( unitType ) )
    {

    case KoUnit::U_MM:
        m_oldUnit = 0;
        break;
    case KoUnit::U_CM:
        m_oldUnit = 1;
        break;
    case KoUnit::U_DM:
        m_oldUnit = 2;
        break;
    case KoUnit::U_INCH:
        m_oldUnit = 3;
        break;
    case KoUnit::U_PT:
        m_oldUnit = 4;
        break;
    case KoUnit::U_PI:
        m_oldUnit = 5;
        break;
    case KoUnit::U_DD:
        m_oldUnit = 6;
        break;
    case KoUnit::U_CC:
    default:
        m_oldUnit = 7;
    }

    m_unit->setCurrentItem( m_oldUnit );
}

void VConfigMiscPage::apply()
{
    KarbonPart * part = m_view->part();

    m_config->setGroup( "Misc" );

    if( m_oldUnit != m_unit->currentItem() )
    {
        QString unitName;
        m_oldUnit = m_unit->currentItem();

        switch( m_oldUnit )
        {

        case 0:
            unitName = KoUnit::unitName( KoUnit::U_MM );
            part->setUnit( KoUnit::U_MM );
            break;

        case 1:
            unitName = KoUnit::unitName( KoUnit::U_CM );
            part->setUnit( KoUnit::U_CM );
            break;

        case 2:
            unitName = KoUnit::unitName( KoUnit::U_DM );
            part->setUnit( KoUnit::U_DM );
            break;

        case 3:
            unitName = KoUnit::unitName( KoUnit::U_INCH );
            part->setUnit( KoUnit::U_INCH );
            break;

        case 4:
            unitName = KoUnit::unitName( KoUnit::U_PT );
            part->setUnit( KoUnit::U_PT );
            break;
        case 5:
            unitName = KoUnit::unitName( KoUnit::U_PI );
            part->setUnit( KoUnit::U_PI );
            break;
        case 6:
            unitName = KoUnit::unitName( KoUnit::U_DD );
            part->setUnit( KoUnit::U_DD );
            break;
        case 7:
        default:
            unitName = KoUnit::unitName( KoUnit::U_CC );
            part->setUnit( KoUnit::U_CC );
            break;
        }

        m_config->writeEntry( "Units", unitName );
    }

    int newUndo = m_undoRedo->value();

    if( newUndo != m_oldUndoRedo )
    {
        m_config->writeEntry( "UndoRedo", newUndo );
        part->setUndoRedoLimit( newUndo );
        m_oldUndoRedo = newUndo;
    }
}

void VConfigMiscPage::slotDefault()
{
    m_undoRedo->setValue( 30 );
    m_unit->setCurrentItem( 0 );
}

VConfigDefaultPage::VConfigDefaultPage( KarbonView* view,
                                        QVBox* box, char* name )
    : QObject( box->parent(), name )
{
    m_view = view;

    m_config = KarbonFactory::instance()->config();

    QVGroupBox* gbDocumentSettings = new QVGroupBox(
        i18n( "Document Settings" ), box );
    gbDocumentSettings->setMargin( 10 );
    gbDocumentSettings->setInsideSpacing( KDialog::spacingHint() );

    m_oldAutoSave = m_view->part()->defaultAutoSave() / 60;

    m_oldBackupFile = true;

    if( m_config->hasGroup( "Interface" ) )
    {
        m_config->setGroup( "Interface" );
        m_oldAutoSave = m_config->readNumEntry( "AutoSave", m_oldAutoSave );
        m_oldBackupFile = m_config->readBoolEntry( "BackupFile", m_oldBackupFile );
    }

    m_autoSave = new KIntNumInput( m_oldAutoSave, gbDocumentSettings );
    m_autoSave->setRange( 0, 60, 1 );
    m_autoSave->setLabel( i18n( "Auto save (min):" ) );
    m_autoSave->setSpecialValueText( i18n( "No auto save" ) );
    m_autoSave->setSuffix( i18n( "min" ) );

    m_createBackupFile = new QCheckBox( i18n( "Create backup file" ), gbDocumentSettings );
    m_createBackupFile->setChecked( m_oldBackupFile );
}

void VConfigDefaultPage::apply()
{
	m_config->setGroup( "Document defaults" );

	m_config->setGroup( "Interface" );

	int autoSave = m_autoSave->value();

	if( autoSave != m_oldAutoSave )
	{
		m_config->writeEntry( "AutoSave", autoSave );
		m_view->part()->setAutoSave( autoSave * 60 );
		m_oldAutoSave = autoSave;
	}

	bool state = m_createBackupFile->isChecked();

	if( state != m_oldBackupFile )
	{
		m_config->writeEntry( "BackupFile", state );
		m_view->part()->setBackupFile( state );
		m_oldBackupFile = state;
	}
}

void VConfigDefaultPage::slotDefault()
{
	m_autoSave->setValue( m_view->part()->defaultAutoSave() / 60 );
	m_createBackupFile->setChecked( true );
}

#include "vconfiguredlg.moc"

