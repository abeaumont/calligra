/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef __trader_scan_h__
#define __trader_scan_h__

#include <CORBA.h>

class Trader;

void koScanFilters( Trader *_trader, CORBA::ORB_ptr _orb );
void koScanOfficeParts( Trader *_trader, CORBA::ORB_ptr _orb );
void koScanParts( Trader *_trader, CORBA::ORB_ptr _orb );

#endif
