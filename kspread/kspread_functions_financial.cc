/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team
                           www.koffice.org/kspread

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// built-in financial functions

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <koscript_parser.h>
#include <koscript_util.h>
#include <koscript_func.h>
#include <koscript_synext.h>

#include <kspread_doc.h>
#include <kspread_functions.h>
#include <kspread_util.h>
#include <kspread_table.h>

// prototypes
bool kspreadfunc_fv( KSContext& context );
bool kspreadfunc_compound( KSContext& context );
bool kspreadfunc_continuous( KSContext& context );
bool kspreadfunc_pv( KSContext& context );
bool kspreadfunc_pv_annuity( KSContext& context );
bool kspreadfunc_fv_annuity( KSContext& context );
bool kspreadfunc_effective( KSContext& context );
bool kspreadfunc_zero_coupon( KSContext& context );
bool kspreadfunc_level_coupon( KSContext& context );
bool kspreadfunc_nominal( KSContext& context );
bool kspreadfunc_sln( KSContext& context );
bool kspreadfunc_syd( KSContext& context );
bool kspreadfunc_db( KSContext& context );
bool kspreadfunc_euro( KSContext& context );

// registers all financial functions
void KSpreadRegisterFinancialFunctions()
{
  KSpreadFunctionRepository* repo = KSpreadFunctionRepository::self();

  repo->registerFunction( "COMPOUND", kspreadfunc_compound );
  repo->registerFunction( "CONTINUOUS", kspreadfunc_continuous );
  repo->registerFunction( "EFFECTIVE", kspreadfunc_effective );
  repo->registerFunction( "NOMINAL", kspreadfunc_nominal );
  repo->registerFunction( "FV", kspreadfunc_fv );
  repo->registerFunction( "FV_ANNUITY", kspreadfunc_fv_annuity );
  repo->registerFunction( "PV", kspreadfunc_pv );
  repo->registerFunction( "PV_ANNUITY", kspreadfunc_pv_annuity );
  repo->registerFunction( "ZERO_COUPON", kspreadfunc_zero_coupon );
  repo->registerFunction( "LEVEL_COUPON", kspreadfunc_level_coupon );
  repo->registerFunction( "SLN", kspreadfunc_sln );
  repo->registerFunction( "SYD", kspreadfunc_syd );
  repo->registerFunction( "DB", kspreadfunc_db );
  repo->registerFunction( "EURO", kspreadfunc_euro );  // KSpread-specific, Gnumeric-compatible
}

// Function: FV
/* Returns future value, given current value, interest rate and time */
bool kspreadfunc_fv( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "FV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;

  double present = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();

  context.setValue( new KSValue( present * pow(1+interest, periods)));
  return true;
}

// Function: compound
/* Returns value after compounded interest, given principal, rate, periods
per year and year */
 bool kspreadfunc_compound( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 4, "compound", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();
  double years = args[3]->doubleValue();

  context.setValue( new KSValue( principal * pow(1+(interest/periods),
periods*years)));

  return true;
}

// Function: continuous
/* Returns value after continuous compounding of interest, given prinicpal,
rate and years */
bool kspreadfunc_continuous( KSContext& context )
{
    // If you still don't understand this, let me know!  ;-)  jsinger@leeta.net
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "continuous", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double years = args[2]->doubleValue();


  context.setValue( new KSValue( principal * exp(interest * years)));
  return true;
}

// Function: PV
bool kspreadfunc_pv( KSContext& context )
{
/* Returns presnt value, given future value, interest rate and years */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "PV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double future = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();


  context.setValue( new KSValue( future / pow(1+interest, periods)));
  return true;
}

// Function: PV_annuity
bool kspreadfunc_pv_annuity( KSContext& context )
{
    /* Returns present value of an annuity or cash flow, given payment,
       interest rate,
       periods, initial amount and whether payments are made at the start (TRUE)
       or end of a period */

    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "PV_annuity", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
	return false;
    double amount = args[0]->doubleValue();
    double interest = args[1]->doubleValue();
    double periods = args[2]->doubleValue();

    double result;
    result = amount * (1 - 1/(pow( (1+interest), periods ))) / interest ;

  context.setValue( new KSValue( result ) );

  return true;
}

// Function: FV_annnuity
bool kspreadfunc_fv_annuity( KSContext& context )
{
    /* Returns future value of an annuity or cash flow, given payment, interest
       rate and periods */

    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "FV_annuity", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
	return false;
    double amount= args[0]->doubleValue();
    double interest = args[1]->doubleValue();
    double periods = args[2]->doubleValue();

    double result;

    result = amount * ((pow( (1+interest),periods))/interest - 1/interest)   ;

    context.setValue( new KSValue( result ) );

    return true;
}

// Function: effective
bool kspreadfunc_effective( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "effective", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double nominal = args[0]->doubleValue();
  double periods = args[1]->doubleValue();

  context.setValue( new KSValue(  pow( 1 + (nominal/periods), periods )- 1 ) );

  return true;
}

// Function: zero_coupon
bool kspreadfunc_zero_coupon( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "zero_coupon", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double face = args[0]->doubleValue();
  double rate = args[1]->doubleValue();
  double years = args[2]->doubleValue();

  context.setValue( new KSValue(  face / pow( (1 + rate), years )  ) );

  return true;
}

// Function: level_coupon
bool kspreadfunc_level_coupon( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 5, "level_coupon", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[4], KSValue::DoubleType, true ) )
    return false;
  double face = args[0]->doubleValue();
  double coupon_rate = args[1]->doubleValue();
  double coupon_year = args[2]->doubleValue();
  double years = args[3]->doubleValue();
  double market_rate = args[4]->doubleValue();

  double coupon = coupon_rate * face / coupon_year;
  double interest =  market_rate/coupon_year;
  double pv_annuity = (1 - 1/(pow( (1+interest), (years*coupon_year) ))) / interest ;
  context.setValue( new KSValue( coupon * pv_annuity + (face/ pow( (1+interest), (years*coupon_year) ) ) ) );

  return true;
}

// Function: nominal
bool kspreadfunc_nominal( KSContext& context )
{
/* Returns nominal interest rate given effective rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "nominal", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double effective = args[0]->doubleValue();
  double periods = args[1]->doubleValue();

  if ( periods == 0.0 ) // Check null
      return false;

  context.setValue( new KSValue( periods * (pow( (effective + 1), (1 / periods) ) -1) ) );

  return true;
}

// Function: SLN
/* straight-line depreciation for a single period */
bool kspreadfunc_sln( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "SLN", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;

  double cost = args[0]->doubleValue();
  double salvage_value = args[1]->doubleValue();
  double life = args[2]->doubleValue();

  // sentinel check
  if( life <= 0.0 ) return false;

  context.setValue( new KSValue( (cost - salvage_value) / life ) );

  return true;
}

// Function: SYD
/* sum-of-years digits depreciation */
bool kspreadfunc_syd( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 4, "SYD", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;

  double cost = args[0]->doubleValue();
  double salvage_value = args[1]->doubleValue();
  double life = args[2]->doubleValue();
  double period = args[3]->doubleValue();

  // sentinel check
  if( life <= 0.0 ) return false;

  context.setValue( new KSValue( ( ( (cost - salvage_value) * (life - period + 1) * 2) /
     (life * (life + 1.0) ) ) ) ) ;

  return true;
}

// Function: DB
/* fixed-declining depreciation */
bool kspreadfunc_db( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  double month = 12;

  if( KSUtil::checkArgumentsCount( context, 5, "DB", false ) )
  {
    if( !KSUtil::checkType( context, args[4], KSValue::DoubleType, true ) )
      return false;
    month = args[4]->doubleValue();
  }
  else
  {
    if ( !KSUtil::checkArgumentsCount( context, 4, "DB", true ) )
      return false;
  }

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;

  double cost = args[0]->doubleValue();
  double salvage = args[1]->doubleValue();
  double life = args[2]->doubleValue();
  double period = args[3]->doubleValue();

  // sentinel check
  if( cost == 0 || life <= 0.0 ) return false;
  if( salvage / cost < 0 ) return false;

  double rate = 1000 * (1 - pow( (salvage/cost), (1/life) ));
  rate = floor( rate + 0.5 )  / 1000; 

  double total = cost * rate * month / 12;

  if( period == 1 )
  {
    context.setValue( new KSValue( total ) );
    return true;
  }

  for( int i = 1; i < life; ++i )
    if( i == period-1 )
    {
      context.setValue( new KSValue( rate * (cost-total) ) );
      return true;
    }
    else total += rate * (cost-total);

  context.setValue( new KSValue( (cost-total) * rate * (12-month)/12 ) );

  return true;
}

// Function: EURO
bool kspreadfunc_euro( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "EURO", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  QString currency = args[0]->stringValue().upper();
  double result = -1;

  if( currency == "ATS" ) result = 13.7603;  // Austria
  else if( currency == "BEF" ) result = 40.3399;  // Belgium
  else if( currency == "DEM" ) result = 1.95583;  // Germany
  else if( currency == "ESP" ) result = 166.386;  // Spain
  else if( currency == "FIM" ) result = 5.94573;  // Finland
  else if( currency == "FRF" ) result = 6.55957;  // France
  else if( currency == "GRD" ) result = 340.75;   // Greece
  else if( currency == "IEP" ) result = 0.787564; // Ireland
  else if( currency == "ITL" ) result = 1936.27;  // Italy
  else if( currency == "LUX" ) result = 40.3399;  // Luxemburg
  else if( currency == "NLG" ) result = 2.20371;  // Nederland
  else if( currency == "PTE" ) result = 200.482;  // Portugal

  if( result <= 0 ) return false;

  context.setValue( new KSValue( result ) );
  return true;
}
