/******************************************************************/
/* KOffice Library - (c) by Reginald Stadlbauer 1998              */
/* Version: 1.0                                                   */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Page Layout Dialog (header)                            */
/******************************************************************/

#ifndef KOPGLAYOUTDIA_H
#define KOPGLAYOUTDIA_H

#include <stdio.h>
#include <stdlib.h>

#include <qtabdlg.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qgrpbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qlined.h>

#include <kapp.h>
#include <krestrictedline.h>
#include <kspinbox.h>

// paper formats (mm)
#define PG_A3_WIDTH             297.0
#define PG_A3_HEIGHT            420.0
#define PG_A4_WIDTH             210.0
#define PG_A4_HEIGHT            297.0
#define PG_A5_WIDTH             148.0
#define PG_A5_HEIGHT            210.0
#define PG_B5_WIDTH             182.0
#define PG_B5_HEIGHT            257.0
#define PG_US_LETTER_WIDTH      216.0
#define PG_US_LETTER_HEIGHT     279.0
#define PG_US_LEGAL_WIDTH       216.0
#define PG_US_LEGAL_HEIGHT      356.0
#define PG_US_EXECUTIVE_WIDTH   191.0
#define PG_US_EXECUTIVE_HEIGHT  254.0
#define PG_SCREEN_WIDTH         240.0
#define PG_SCREEN_HEIGHT        180.0

// enums
const int FORMAT_AND_BORDERS = 1;
const int HEADER_AND_FOOTER = 2;
const int KWORD_SPECIAL = 4;

enum KoFormat {PG_DIN_A3 = 0,PG_DIN_A4 = 1,PG_DIN_A5 = 2,PG_US_LETTER = 3,PG_US_LEGAL = 4,PG_SCREEN = 5,PG_CUSTOM = 6,PG_DIN_B5 = 7,PG_US_EXECUTIVE = 8};
enum KoOrientation {PG_PORTRAIT = 0,PG_LANDSCAPE = 1};
enum KoUnit {PG_MM = 0,PG_CM = 1,PG_INCH = 2};

// structure for page layout
struct KoPageLayout
{
  KoFormat format;
  KoOrientation orientation;
  double width,height;
  double left,right,top,bottom;
  KoUnit unit;
};

// structure for header-footer
struct KoHeadFoot
{
  QString headLeft;
  QString headMid;
  QString headRight;
  QString footLeft;
  QString footMid;
  QString footRight;
};

struct KoKWord
{
  int columns;
  int columnSpacing;
};

/******************************************************************/
/* class KoPagePreview                                            */
/******************************************************************/

class KoPagePreview : public QGroupBox
{
  Q_OBJECT

public:

  // constructor - destructor
  KoPagePreview(QWidget*,const char*,KoPageLayout);
  ~KoPagePreview();
  
  // set page layout
  void setPageLayout(KoPageLayout);

protected:
  
  // paint page
  void drawContents(QPainter*);

  // vars
  int pgWidth,pgHeight,pgX,pgY,pgW,pgH;

};

/******************************************************************/
/* class KoPageLayoutDia                                          */
/******************************************************************/

class KoPageLayoutDia : public QTabDialog
{
  Q_OBJECT

public:

  // constructor - destructor
  KoPageLayoutDia(QWidget*,const char*,KoPageLayout,KoHeadFoot,int);
  KoPageLayoutDia(QWidget* parent,const char* name,KoPageLayout _layout,KoHeadFoot _hf,
		  KoKWord _kw,int tabs);
  ~KoPageLayoutDia();              

  // show page layout dialog
  static bool pageLayout(KoPageLayout&,KoHeadFoot&,int);
  static bool pageLayout(KoPageLayout&,KoHeadFoot&,KoKWord&,int);

  // get a standard page layout
  static KoPageLayout standardLayout();

  // should NOT be used by an app - just for internal use!
  KoPageLayout getLayout() {return layout;};
  KoHeadFoot getHeadFoot();
  KoKWord getKWord();

protected:

  // setup tabs
  void setupTab1();
  void setValuesTab1();
  void setupTab2();
  void setupTab3();

  // update preview
  void updatePreview(KoPageLayout);

  // dialog objects
  QGroupBox *formatFrame,*borderFrame;
  QWidget *tab1,*tab2,*tab3;
  QGridLayout *grid1,*grid3,*formatGrid,*borderGrid,*grid2;
  QLabel *lpgFormat,*lpgOrientation,*lpgUnit,*lpgWidth,*lpgHeight,*lbrLeft,*lbrRight,*lbrTop,*lbrBottom;
  QComboBox *cpgFormat,*cpgOrientation,*cpgUnit;
  KRestrictedLine *epgWidth,*epgHeight,*ebrLeft,*ebrRight,*ebrTop,*ebrBottom;
  KoPagePreview *pgPreview;
  QLabel *lHeadLeft,*lHeadMid,*lHeadRight,*lHead;
  QLineEdit *eHeadLeft,*eHeadMid,*eHeadRight;
  QLabel *lFootLeft,*lFootMid,*lFootRight,*lFoot,*lMacros1,*lMacros2;
  QLineEdit *eFootLeft,*eFootMid,*eFootRight;
  KNumericSpinBox *nColumns,*nCSpacing;
  QLabel *lColumns,*lCSpacing;

  // layout
  KoPageLayout layout;
  KoHeadFoot hf;
  KoKWord kw;

  bool retPressed;

private slots:     

  // take changes
  void Ok() {}                                  

  // combos
  void unitChanged(int);
  void formatChanged(int);
  void orientationChanged(int);

  // linedits
  void widthChanged();
  void heightChanged();
  void leftChanged();
  void rightChanged();
  void topChanged();
  void bottomChanged();
  void rPressed() {retPressed = true;}

};
#endif //KOPGLAYOUTDIA_H



