#include "KSpreadDocIface.h"
#include <KoDocumentIface.h>

#include "kspread_doc.h"
#include "kspread_map.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <qcolor.h>
#include <kdebug.h>

KSpreadDocIface::KSpreadDocIface( KSpreadDoc* _doc )
    : KoDocumentIface( _doc )
{
    doc=_doc;
}

DCOPRef KSpreadDocIface::map()
{
    return DCOPRef( kapp->dcopClient()->appId(),
                    doc->map()->dcopObject()->objId() );
}

bool KSpreadDocIface::save()
{
    // TODO
  return false;
}

bool KSpreadDocIface::saveAs( const QString& )
{
    // TODO
  return false;
}


bool KSpreadDocIface::showColHeader()const
{
    return doc->getShowColHeader();
}

bool KSpreadDocIface::showRowHeader()const
{
    return doc->getShowRowHeader();
}

int KSpreadDocIface::indentValue()const
{
    return doc->getIndentValue();
}

void KSpreadDocIface::setIndentValue(int _val)
{
    doc->setIndentValue(_val);
}

void KSpreadDocIface::changeDefaultGridPenColor( const QColor &_col)
{
    doc->changeDefaultGridPenColor(_col);
}

QColor KSpreadDocIface::pageBorderColor() const
{
    return doc->pageBorderColor();
}


bool KSpreadDocIface::showCommentIndicator()const
{
    return doc->getShowCommentIndicator();
}

bool KSpreadDocIface::showFormulaBar()const
{
    return doc->getShowFormulaBar();
}

bool KSpreadDocIface::showStatusBar()const
{
    return doc->getShowStatusBar();
}

bool KSpreadDocIface::showTabBar()const
{
    return doc->getShowTabBar();
}

void KSpreadDocIface::setShowVerticalScrollBar(bool _show)
{
    doc->setShowVerticalScrollBar(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowHorizontalScrollBar(bool _show)
{
    doc->setShowHorizontalScrollBar(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowColHeader(bool _show)
{
    doc->setShowColHeader(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowRowHeader(bool _show)
{
    doc->setShowRowHeader(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowTabBar(bool _show)
{
    doc->setShowTabBar(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowCommentIndicator(bool _show)
{
    doc->setShowCommentIndicator(_show);
    doc->refreshInterface();
}

void KSpreadDocIface::setShowMessageError(bool _show)
{
    doc->setShowMessageError(_show);
}

bool KSpreadDocIface::showMessageError()const
{
    return doc->getShowMessageError();
}

void KSpreadDocIface::changePageBorderColor( const QColor & _color)
{
    doc->changePageBorderColor( _color);
    doc->refreshInterface();
}

void KSpreadDocIface::addIgnoreWordAll( const QString &word)
{
    doc->addIgnoreWordAll( word );
}

void KSpreadDocIface::clearIgnoreWordAll( )
{
    doc->clearIgnoreWordAll();
}

QStringList KSpreadDocIface::spellListIgnoreAll() const
{
    return doc->spellListIgnoreAll();
}

void KSpreadDocIface::addStringCompletion(const QString & stringCompletion)
{
    doc->addStringCompletion( stringCompletion );
}

int KSpreadDocIface::zoom() const
{
    return doc->zoom();
}


QString KSpreadDocIface::moveToValue()const
{
    switch(doc->getMoveToValue())
    {
    case KSpread::Bottom:
        return QString("bottom");
        break;
    case KSpread::Left:
        return QString("left");
        break;
    case KSpread::Top:
        return QString("top");
        break;
    case KSpread::Right:
        return QString("right");
        break;
    }
    return QString::null;
}

void KSpreadDocIface::setMoveToValue(const QString & move)
{
    if ( move.lower()=="bottom")
        doc->setMoveToValue(KSpread::Bottom);
    else if ( move.lower()=="top")
        doc->setMoveToValue(KSpread::Top);
    else if ( move.lower()=="left")
        doc->setMoveToValue(KSpread::Left);
    else if ( move.lower()=="right")
        doc->setMoveToValue(KSpread::Right);
}

void KSpreadDocIface::setTypeOfCalc( const QString & calc )
{
    if ( calc.lower()=="sum")
        doc->setTypeOfCalc(SumOfNumber );
    else if ( calc.lower()=="min")
        doc->setTypeOfCalc( Min );
    else if ( calc.lower()=="max")
        doc->setTypeOfCalc(Max );
    else if ( calc.lower()=="average")
        doc->setTypeOfCalc(Average );
    else if ( calc.lower()=="count")
        doc->setTypeOfCalc(Count );
    else if ( calc.lower()=="none")
        doc->setTypeOfCalc(NoneCalc );
    else
        kdDebug()<<"Error in setTypeOfCalc( const QString & calc ) :"<<calc<<endl;
    doc->refreshInterface();
}

QString KSpreadDocIface::typeOfCalc() const
{
    switch( doc->getTypeOfCalc() )
    {
    case SumOfNumber:
        return QString("sum");
        break;
    case Min:
        return QString("min");
        break;
    case Max:
        return QString("max");
        break;
    case Average:
        return QString("average");
        break;
    case Count:
        return QString("count");
        break;
    case NoneCalc:
    default:
       return QString("none");
       break;
    }
}

void KSpreadDocIface::setBackupFile( bool _b )
{
    doc->setBackupFile( _b);
}

bool KSpreadDocIface::backupFile()const
{
    return doc->backupFile();
}

