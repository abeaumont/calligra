/* This file is part of the Calligra project
 * Copyright (c) 2008, 2012 Dag Andersen <danders@get2net.dk>
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

#include "kptnodechartmodel.h"

#include "kptlocale.h"
#include "kptnode.h"
#include "kptproject.h"
#include "kptschedule.h"
#include "kptresource.h"
#include "kptdebug.h"

#include <KLocalizedString>

#include <QVariant>
#include <QPen>

#include <KChartGlobal>
#include <KChartPalette>


namespace KPlato
{

ChartItemModel::ChartItemModel( QObject *parent )
    : ItemModelBase( parent ),
      m_localizeValues( false )
{
}

QModelIndex ChartItemModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

const QMetaEnum ChartItemModel::columnMap() const
{
    return metaObject()->enumerator( metaObject()->indexOfEnumerator("Properties") );
}

int ChartItemModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return columnMap().keyCount();
}

int ChartItemModel::rowCount( const QModelIndex &parent ) const
{
    return parent.isValid() ? 0 : startDate().daysTo( endDate() ) + 1;
}

QModelIndex ChartItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( m_project == 0 || row < 0 || column < 0 ) {
        //debugPlan<<"No project"<<m_project<<" or illegal row, column"<<row<<column;
        return QModelIndex();
    }
    if ( parent.isValid() ) {
        return QModelIndex();
    }
    return createIndex( row, column );
}

double ChartItemModel::bcwsEffort( int day ) const
{
    return m_bcws.hoursTo( startDate().addDays( day ) );
}

double ChartItemModel::bcwpEffort( int day ) const
{
    double res = 0.0;
    QDate date = startDate().addDays( day );
    if ( m_bcws.days().contains( date ) ) {
        res = m_bcws.bcwpEffort( date );
    } else if ( date > m_bcws.endDate() ) {
        res = m_bcws.bcwpEffort( date );
    }
    return res;
}

double ChartItemModel::acwpEffort( int day ) const
{
    return m_acwp.hoursTo( startDate().addDays( day ) );
}

double ChartItemModel::bcwsCost( int day ) const
{
    return m_bcws.costTo( startDate().addDays( day ) );
}

double ChartItemModel::bcwpCost( int day ) const
{
    double res = 0.0;
    QDate date = startDate().addDays( day );
    if ( m_bcws.days().contains( date ) ) {
        res = m_bcws.bcwpCost( date );
    } else if ( date > m_bcws.endDate() ) {
        res = m_bcws.bcwpCost( m_bcws.endDate() );
    }
    return res;
}

double ChartItemModel::acwpCost( int day ) const
{
    return m_acwp.costTo( startDate().addDays( day ) );
}

double ChartItemModel::spiEffort( int day ) const
{
    double p = bcwpEffort( day );
    double s = bcwsEffort( day );
    return s == 0.0 ? 0.0 : p / s;
}

double ChartItemModel::spiCost( int day ) const
{
    double p = bcwpCost( day );
    double s = bcwsCost( day );
    return s == 0.0 ? 0.0 : p / s;
}

double ChartItemModel::cpiEffort( int day ) const
{
    double p = bcwpEffort( day );
    double a = acwpEffort( day );
    return a == 0.0 ? 0.0 : p / a;
}

double ChartItemModel::cpiCost( int day ) const
{
    double p = bcwpCost( day );
    double a = acwpCost( day );
    return a == 0.0 ? 0.0 : p / a;
}

QVariant ChartItemModel::data( const QModelIndex &index, int role ) const
{
    QVariant result;
    if ( role == Qt::DisplayRole ) {
        if ( ! m_localizeValues ) {
            return data( index, Qt::EditRole );
        } else {
            QLocale locale;
            // TODO: temporary workaround while KLocale/money logic still used
            Locale *planLocale;
            Locale *tmpPlanLocale = 0;
            if (project()) {
                planLocale = project()->locale();
            } else {
                tmpPlanLocale = new Locale();
                planLocale = tmpPlanLocale;
            }
            switch ( index.column() ) {
            case BCWSCost: result = planLocale->formatMoney( bcwsCost( index.row() ), QString(), 0 ); break;
            case BCWPCost: result = planLocale->formatMoney( bcwpCost( index.row() ), QString(), 0 ); break;
            case ACWPCost: result = planLocale->formatMoney( acwpCost( index.row() ), QString(), 0 ); break;
            case BCWSEffort: result = locale.toString( bcwsEffort( index.row() ), 'f', 0 ); break;
            case BCWPEffort: result = locale.toString( bcwpEffort( index.row() ), 'f', 0 ); break;
            case ACWPEffort: result = locale.toString( acwpEffort( index.row() ), 'f', 0 ); break;
            case SPICost: result = locale.toString( spiCost( index.row() ), 'f', 2 ); break;
            case CPICost: result = locale.toString( cpiCost( index.row() ), 'f', 2 ); break;
            case SPIEffort: result = locale.toString( spiEffort( index.row() ), 'f', 2 ); break;
            case CPIEffort: result = locale.toString( cpiEffort( index.row() ), 'f', 2 ); break;
            default: break;
            }
            delete tmpPlanLocale;
        }
        //debugPlan<<index<<role<<result;
        return result;
    } else if ( role == Qt::EditRole ) {
        switch ( index.column() ) {
        case BCWSCost: result = bcwsCost( index.row() ); break;
        case BCWPCost: result = bcwpCost( index.row() ); break;
        case ACWPCost: result = acwpCost( index.row() ); break;
        case BCWSEffort: result = bcwsEffort( index.row() ); break;
        case BCWPEffort: result = bcwpEffort( index.row() ); break;
        case ACWPEffort: result = acwpEffort( index.row() ); break;
        case SPICost: result = spiCost( index.row() ); break;
        case CPICost: result = cpiCost( index.row() ); break;
        case SPIEffort: result = spiEffort( index.row() ); break;
        case CPIEffort: result = cpiEffort( index.row() ); break;
        default: break;
        }
        //debugPlan<<index<<role<<result;
        return result;
    } else if ( role == Qt::ForegroundRole ) {
        double v = 0.0;
        switch ( index.column() ) {
        case SPICost: v = spiCost( index.row() ); break;
        case CPICost: v = cpiCost( index.row() ); break;
        case SPIEffort: v = spiEffort( index.row() ); break;
        case CPIEffort: v = cpiEffort( index.row() ); break;
        default: break;
        }
        if ( v > 0.0 && v < 1.0 ) {
            result = QBrush( Qt::red );
        }
        return result;
    }  else if ( role == KChart::DatasetBrushRole ) {
        return headerData( index.column(), Qt::Horizontal, role );
    }  else if ( role == KChart::DatasetPenRole ) {
        return headerData( index.column(), Qt::Horizontal, role );
    }
    //debugPlan<<index<<role<<result;
    return result;
}

QVariant ChartItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    QVariant result;
    if ( role == Qt::DisplayRole ) {
        if ( orientation == Qt::Horizontal ) {
            switch ( section ) {
                case BCWSCost: return i18nc( "Cost based Budgeted Cost of Work Scheduled", "BCWS Cost" );
                case BCWPCost: return i18nc( "Cost based Budgeted Cost of Work Performed", "BCWP Cost" );
                case ACWPCost: return i18nc( "Cost based Actual Cost of Work Performed", "ACWP Cost" );
                case BCWSEffort: return i18nc( "Effort based Budgeted Cost of Work Scheduled", "BCWS Effort" );
                case BCWPEffort: return i18nc( "Effort based Budgeted Cost of Work Performed", "BCWP Effort" );
                case ACWPEffort: return i18nc( "Effort based Actual Cost of Work Performed", "ACWP Effort" );
                case SPICost: return i18nc( "Cost based Schedule Performance Index", "SPI Cost" );
                case CPICost: return i18nc( "Cost based Cost Performance Index", "CPI Cost" );
                case SPIEffort: return i18nc( "Effort based Schedule Performance Index", "SPI Effort" );
                case CPIEffort: return i18nc( "Effort based Cost Performance Index", "CPI Effort" );
                default: return QVariant();
            }
        } else {
            return startDate().addDays( section ).toString( i18nc( "Date format used as chart axis labels. Must follow QDate specification.", "MM.dd" ) );
        }
    } else if ( role == Qt::ToolTipRole ) {
        if ( orientation == Qt::Horizontal ) {
            switch ( section ) {
                case BCWSCost: return xi18nc( "@info:tooltip", "Cost based Budgeted Cost of Work Scheduled" );
                case BCWPCost: return xi18nc( "@info:tooltip", "Cost based Budgeted Cost of Work Performed" );
                case ACWPCost: return xi18nc( "@info:tooltip", "Cost based Actual Cost of Work Performed" );
                case BCWSEffort: return xi18nc( "@info:tooltip", "Effort based Budgeted Cost of Work Scheduled" );
                case BCWPEffort: return xi18nc( "@info:tooltip", "Effort based Budgeted Cost of Work Performed" );
                case ACWPEffort: return xi18nc( "@info:tooltip", "Effort based Actual Cost of Work Performed" );
                case SPICost: return xi18nc( "@info:tooltip", "Cost based Schedule Performance Index (BCWP/BCWS)" );
                case CPICost: return xi18nc( "@info:tooltip", "Cost based Cost Performance Index (BCWP/ACWS)" );
                case SPIEffort: return xi18nc( "@info:tooltip", "Effort based Schedule Performance Index (BCWP/BCWS)" );
                case CPIEffort: return xi18nc( "@info:tooltip", "Effort based Cost Performance Index (BCWP/ACWS)" );
                default: return QVariant();
            }
        } else {
            return QLocale().toString( startDate().addDays( section ), QLocale::ShortFormat );
        }
    } else if ( role == Qt::EditRole ) {
        if ( orientation == Qt::Horizontal ) {
            switch ( section ) {
                case BCWSCost: return "BCWS Cost";
                case BCWPCost: return "BCWP Cost";
                case ACWPCost: return "ACWP Cost";
                case BCWSEffort: return "BCWS Effort";
                case BCWPEffort: return "BCWP Effort";
                case ACWPEffort: return "ACWP Effort";
                case SPICost: return "SPI Cost";
                case CPICost: return "CPI Cost";
                case SPIEffort: return "SPI Effort";
                case CPIEffort: return "CPI Effort";
                default: return QVariant();
            }
        } else {
            return startDate().addDays( section );
        }
#ifdef PLAN_CHART_DEBUG
    } else if ( role == Qt::BackgroundRole ) {
        if ( orientation == Qt::Vertical ) {
            if ( startDate().addDays( section ) == QDate::currentDate() ) {
                return QBrush( Qt::red );
            }
        }
#endif
    }  else if ( role == KChart::DatasetBrushRole ) {
        if ( orientation == Qt::Horizontal ) {
            return KChart::Palette::defaultPalette().getBrush( section );
        }
    }  else if ( role == KChart::DatasetPenRole ) {
        QPen p;
        p.setBrush( headerData( section, orientation, KChart::DatasetBrushRole ).value<QBrush>() );
        result = p;
        //debugPlan<<section<<"DatasetPenRole"<<result;
        return result;
    }
    return ItemModelBase::headerData(section, orientation, role);
}

void ChartItemModel::setProject( Project *project )
{
    beginResetModel();
    m_bcws.clear();
    m_acwp.clear();
    if ( m_project ) {
        disconnect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));
        disconnect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(setScheduleManager(ScheduleManager*)) );
        disconnect( m_project, SIGNAL(nodeRemoved(Node*)), this, SLOT(slotNodeRemoved(Node*)) );
        disconnect( m_project, SIGNAL(nodeChanged(Node*)), this, SLOT(slotNodeChanged(Node*)) );
        disconnect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceChanged(const Resource*)) );
        disconnect( m_project, SIGNAL(resourceChanged(Resource*)), this, SLOT(slotResourceChanged(Resource*)) );
    }
    m_project = project;
    if ( m_project ) {
        connect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));
        connect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(setScheduleManager(ScheduleManager*)) );
        connect( m_project, SIGNAL(nodeRemoved(Node*)), this, SLOT(slotNodeRemoved(Node*)) );
        connect( m_project, SIGNAL(nodeChanged(Node*)), this, SLOT(slotNodeChanged(Node*)) );
        connect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceChanged(const Resource*)) );
        connect( m_project, SIGNAL(resourceChanged(Resource*)), this, SLOT(slotResourceChanged(Resource*)) );
    }
    endResetModel();
}

void ChartItemModel::setScheduleManager( ScheduleManager *sm )
{
    beginResetModel();
    m_manager = sm;
    calculate();
    endResetModel();
}

void ChartItemModel::setNodes( const QList<Node*> &nodes )
{
    beginResetModel();
    debugPlan<<nodes;
    m_nodes = nodes;
    calculate();
    endResetModel();
}

void ChartItemModel::addNode( Node *node )
{
    beginResetModel();
    m_nodes.append( node );
    calculate();
    endResetModel();
}

void ChartItemModel::clearNodes()
{
    beginResetModel();
    m_nodes.clear();
    calculate();
    endResetModel();
}

void ChartItemModel::slotNodeRemoved( Node *node )
{
    if ( m_nodes.contains( node ) ) {
        beginResetModel();
        m_nodes.removeAt( m_nodes.indexOf( node ) );
        calculate();
        endResetModel();
    }
}

void ChartItemModel::slotNodeChanged( Node *node )
{
    //debugPlan<<this<<node;
    if ( m_nodes.contains( node ) ) {
        beginResetModel();
        calculate();
        endResetModel();
        return;
    }
    foreach ( Node *n, m_nodes ) {
        if ( node->isChildOf( n ) ) {
            beginResetModel();
            calculate();
            endResetModel();
            return;
        }
    }
}

void ChartItemModel::slotResourceChanged( Resource* )
{
    beginResetModel();
    calculate();
    endResetModel();
}

void ChartItemModel::slotResourceChanged( const Resource* )
{
    beginResetModel();
    calculate();
    endResetModel();
}

QDate ChartItemModel::startDate() const
{
    QDate d = m_bcws.startDate();
    if ( m_acwp.startDate().isValid() ) {
        if ( ! d.isValid() || d > m_acwp.startDate() ) {
            d = m_acwp.startDate();
        }
    }
    return d;
}

QDate ChartItemModel::endDate() const
{
    return qMax( m_bcws.endDate(), m_acwp.endDate() );
}

void ChartItemModel::calculate()
{
    //debugPlan<<m_project<<m_manager<<m_nodes;
    m_bcws.clear();
    m_acwp.clear();
    if ( m_manager ) {
        if ( m_project ) {
            foreach ( Node *n, m_nodes ) {
                bool skip = false;
                foreach ( Node *p, m_nodes ) {
                    if ( n->isChildOf( p ) ) {
                        skip = true;
                        break;
                    }
                }
                if ( ! skip ) {
                    m_bcws += n->bcwpPrDay( m_manager->scheduleId(), ECCT_EffortWork );
                    m_acwp += n->acwp( m_manager->scheduleId() );
                }
            }
        }
    }
    //debugPlan<<"bcwp"<<m_bcws;
    //debugPlan<<"acwp"<<m_acwp;
}

void ChartItemModel::setLocalizeValues( bool on )
{
    m_localizeValues = on;
}

//-------------------------
PerformanceDataCurrentDateModel::PerformanceDataCurrentDateModel( QObject *parent )
    : ChartItemModel( parent )
{
    setLocalizeValues( true );
}


int PerformanceDataCurrentDateModel::rowCount( const QModelIndex &parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    return 2;
}

int PerformanceDataCurrentDateModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return 5;
}

QModelIndex PerformanceDataCurrentDateModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( parent.isValid() ) {
        return QModelIndex();
    }
    return createIndex( row, column );
}

QVariant PerformanceDataCurrentDateModel::data(const QModelIndex &idx, int role) const
{
    return ChartItemModel::data( mapIndex( idx ), role );
}

QVariant PerformanceDataCurrentDateModel::headerData( int section, Qt::Orientation o, int role ) const
{
    if ( role == Qt::DisplayRole ) {
        if ( o == Qt::Horizontal ) {
            switch ( section ) {
            case 0: return xi18nc( "@title:column Budgeted Cost of Work Scheduled", "BCWS" );
            case 1: return xi18nc( "@title:column Budgeted Cost of Work Performed", "BCWP" );
            case 2: return xi18nc( "@title:column Actual Cost of Work Performed", "ACWP" );
            case 3: return xi18nc( "@title:column Schedule Performance Index", "SPI" );
            case 4: return xi18nc( "@title:column Cost Performance Index", "CPI" );
            default: break;
            }
        } else {
            switch ( section ) {
            case 0: return xi18nc( "@title:column", "Cost:" );
            case 1: return xi18nc( "@title:column", "Effort:" );
            default: break;
            }
        }
    } else if ( role == Qt::ToolTipRole ) {
        if ( o == Qt::Horizontal ) {
            switch ( section ) {
            case 0: return xi18nc( "@info:tooltip", "Budgeted Cost of Work Scheduled" );
            case 1: return xi18nc( "@info:tooltip", "Budgeted Cost of Work Performed" );
            case 2: return xi18nc( "@info:tooltip", "Actual Cost of Work Performed" );
            case 3: return xi18nc( "@info:tooltip", "Schedule Performance Index" );
            case 4: return xi18nc( "@info:tooltip", "Cost Performance Index" );
            default: break;
            }
        } else {
            switch ( section ) {
            case 0: return xi18nc( "@info:tooltip", "Performance indicators based on cost" );
            case 1: return xi18nc( "@info:tooltip", "Performance indicators based on effort" );
            default: break;
            }
        }
    }
    return QVariant();
}

QModelIndex PerformanceDataCurrentDateModel::mapIndex( const QModelIndex &idx ) const
{
    if ( ! startDate().isValid() ) {
        return QModelIndex();
    }
    int row = startDate().daysTo( QDate::currentDate() );
    if ( row < 0 ) {
        return QModelIndex();
    }
    int column = -1;
    switch ( idx.column() ) {
        case 0: column = idx.row() == 0 ? BCWSCost : BCWSEffort; break; // BCWS
        case 1: column = idx.row() == 0 ? BCWPCost : BCWPEffort; break; // BCWP
        case 2: column = idx.row() == 0 ? ACWPCost : ACWPEffort; break; // ACWP
        case 3: column = idx.row() == 0 ? SPICost : SPIEffort; break; // SPI
        case 4: column = idx.row() == 0 ? CPICost : CPIEffort; break; // CPI
        default: break;
    }
    if ( column < 0 ) {
        return QModelIndex();
    }
    return ChartItemModel::index( row, column );
}

} //namespace KPlato
