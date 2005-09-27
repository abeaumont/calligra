/* libppt - library to read PowerPoint presentation
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>

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
   Boston, MA 02111-1307, US
*/

#ifndef LIBPPT_SLIDE
#define LIBPPT_SLIDE

#include "ustring.h"


namespace Libppt
{

class Presentation;
class GroupObject;
class TextObject;

class Slide
{
public:
  Slide( Presentation* presentation );
  ~Slide();
  void clear();
  UString title() const;
  void setTitle( const UString& title );
  GroupObject* rootObject();
  void setRootObject( GroupObject *);
  TextObject* textObject( unsigned placeId );
private:
  // no copy or assign
  Slide( const Slide& );
  Slide& operator=( const Slide& );
  
  class Private;
  Private* d;
};

};

#endif /* LIBPPT_SLIDE */
