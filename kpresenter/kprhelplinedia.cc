/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <knuminput.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <koUnit.h>
#include <klineedit.h>
#include <knumvalidator.h>

#include "kprhelplinedia.h"
#include "kpresenter_doc.h"


KPrMoveHelpLineDia::KPrMoveHelpLineDia( QWidget *parent, double value, double limitTop, double limitBottom, KPresenterDoc *_doc, const char *name)
    : KDialogBase( parent, name , true, "", Ok | Cancel | User1, Ok, true )
{
    m_doc=_doc;
    m_bRemoveLine = false;

    setButtonText( KDialogBase::User1, i18n("Remove") );
    setCaption( i18n("Change Help Line position") );
    QVBox *page = makeVBoxMainWidget();
    QLabel *lab=new QLabel(i18n("Position:(%1)").arg(m_doc->getUnitName()), page);
    position = new KLineEdit(page);
    position->setText( KoUnit::userValue( QMAX(0.00, value), m_doc->getUnit() ) );
    position->setValidator(new KFloatValidator( KoUnit::ptFromUnit(QMAX(0.00, limitTop), m_doc->getUnit() ),KoUnit::ptFromUnit(QMAX(0.00, limitBottom), m_doc->getUnit() ),true,position ) );
    connect( this, SIGNAL( user1Clicked() ), this ,SLOT( slotRemoveHelpLine() ));
    resize( 300,100 );
}

void KPrMoveHelpLineDia::slotRemoveHelpLine()
{
    m_bRemoveLine = true;
    KDialogBase::slotOk();
}

double KPrMoveHelpLineDia::newPosition()
{
    return KoUnit::fromUserValue( position->text(), m_doc->getUnit() );
}


KPrInsertHelpLineDia::KPrInsertHelpLineDia( QWidget *parent, const KoRect & _pageRect , KPresenterDoc *_doc, const char *name)
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{
    limitOfPage=_pageRect;
    m_doc=_doc;
    setCaption( i18n("Add new Help Line") );
    QVBox *page = makeVBoxMainWidget();
    QButtonGroup *group = new QButtonGroup( 1, QGroupBox::Horizontal,i18n("Orientation"), page );
    group->setRadioButtonExclusive( TRUE );
    group->layout();
    m_rbHoriz = new QRadioButton( i18n("Horizontal"), group );
    m_rbVert = new QRadioButton( i18n("Vertical"), group );

    connect( group , SIGNAL( clicked( int) ), this, SLOT( slotRadioButtonClicked() ));

    QLabel *lab=new QLabel(i18n("Position:(%1)").arg(m_doc->getUnitName()), page);
    position = new KLineEdit(page);
    position->setText( KoUnit::userValue( 0.00, m_doc->getUnit() ) );
    floatValidator = new KFloatValidator( KoUnit::ptFromUnit(limitOfPage.top(),m_doc->getUnit() ) , KoUnit::ptFromUnit(limitOfPage.bottom(),m_doc->getUnit() ) ,true,position );
    position->setValidator( floatValidator );

    m_rbHoriz->setChecked( true );
    resize( 300,100 );
}

double KPrInsertHelpLineDia::newPosition()
{
    return KoUnit::fromUserValue( position->text(), m_doc->getUnit() );

}

bool KPrInsertHelpLineDia::addHorizontalHelpLine()
{
    return m_rbHoriz->isChecked();
}

void KPrInsertHelpLineDia::slotRadioButtonClicked()
{
    if ( m_rbHoriz->isChecked() )
    {
        floatValidator->setRange( limitOfPage.top(), limitOfPage.bottom());
        floatValidator->setRange( KoUnit::ptFromUnit(limitOfPage.top(),m_doc->getUnit() ) , KoUnit::ptFromUnit(limitOfPage.bottom(),m_doc->getUnit() ));
    }
    else if ( m_rbVert->isChecked() )
    {
        floatValidator->setRange( KoUnit::ptFromUnit(limitOfPage.left(),m_doc->getUnit() ) , KoUnit::ptFromUnit(limitOfPage.right(),m_doc->getUnit() ));
    }
}


KPrInsertHelpPointDia::KPrInsertHelpPointDia( QWidget *parent, const KoRect & _pageRect , KPresenterDoc *_doc, double posX, double posY, const char *name)
    : KDialogBase( parent, name , true, "", Ok|Cancel| User1, Ok, true ),
      m_bRemovePoint( false )
{
    limitOfPage=_pageRect;
    m_doc=_doc;
    setButtonText( KDialogBase::User1, i18n("Remove") );
    setCaption( i18n("Add new Help Point") );
    QVBox *page = makeVBoxMainWidget();
    QLabel *lab=new QLabel(i18n("Position (x): (%1)").arg(m_doc->getUnitName()), page);
    positionX = new KLineEdit(page);
    positionX->setText( KoUnit::userValue( posX, m_doc->getUnit() ) );
    positionX->setValidator( new KFloatValidator( limitOfPage.left(), limitOfPage.right() ,true,positionX ) );


    lab=new QLabel(i18n("Position (y): (%1)").arg(m_doc->getUnitName()), page);
    positionY = new KLineEdit(page);
    positionY->setText( KoUnit::userValue( posY, m_doc->getUnit() ) );
    positionY->setValidator( new KFloatValidator( limitOfPage.top(), limitOfPage.bottom() ,true,positionY ) );

    showButton( KDialogBase::User1, (posX!=0.0 || posY!=0.0) );

    connect( this, SIGNAL( user1Clicked() ), this ,SLOT( slotRemoveHelpPoint() ));

    resize( 300,100 );
}

KoPoint KPrInsertHelpPointDia::newPosition() const
{
    return KoPoint( KoUnit::fromUserValue( positionX->text(), m_doc->getUnit() ), KoUnit::fromUserValue( positionY->text(), m_doc->getUnit() ));
}

void KPrInsertHelpPointDia::slotRemoveHelpPoint()
{
    m_bRemovePoint = true;
    KDialogBase::slotOk();
}

#include "kprhelplinedia.moc"
