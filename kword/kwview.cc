/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   Boston, MA 02111-1307, USA
*/

#include "kwcanvas.h"
#undef Unsorted
#include <qpainter.h>
#include <qstring.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qevent.h>
#include <kmessagebox.h>
#include <qclipboard.h>
#include <qsplitter.h>
#include <kaction.h>
#include <qregexp.h>
#include <qobjectlist.h>
#include <qpaintdevicemetrics.h>
#include <qtl.h>


#include "kwview.h"
#include "kwviewmode.h"
#include "kwtextframeset.h"
#include "kwtableframeset.h"
#include "kwdoc.h"
#include "kwframe.h"
#include "kwstyle.h"
#include "kwformat.h"
#include "defs.h"
#include "paragdia.h"
#include "searchdia.h"
#include "stylist.h"
#include "tabledia.h"
#include "insdia.h"
#include "deldia.h"
#include "docstruct.h"
#include "variable.h"
#include "footnotedia.h"
#include "autoformatdia.h"
#include "variabledlgs.h"
#include "serialletter.h"
#include "kwconfig.h"
#include "kcharselectdia.h"
#include "kwcommand.h"
#include "fontdia.h"
#include "counter.h"
#include "kwchangecasedia.h"
#include "kweditpersonnalexpressiondia.h"
#include "splitcellsdia.h"

#include <koMainWindow.h>
#include <koDocument.h>
#include <koRuler.h>
#include <koTabChooser.h>
#include <koPartSelectDia.h>
#include <koTemplateCreateDia.h>
#include <koFrame.h>

#include <ktempfile.h>
#include <kapp.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kstdaction.h>
#include <klocale.h>
#include <qrect.h>
#undef Bool
#include <kspell.h>
#include <kcolordlg.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kcoloractions.h>
#include <tkcoloractions.h>
#include <kstddirs.h>
#include <kparts/event.h>
#include <kformuladocument.h>

#include <stdlib.h>

#include "preview.h"

KWView::KWView( QWidget *_parent, const char *_name, KWDocument* _doc )
    : KoView( _doc, _parent, _name )
{
    m_doc = _doc;
    m_gui = 0;
    m_kspell = 0;
    m_border.left.color = white;
    m_border.left.style = Border::SOLID;
    m_border.left.ptWidth = 0;
    m_border.right = m_border.left;
    m_border.top = m_border.left;
    m_border.bottom = m_border.left;
    m_border.common.color = black;
    m_border.common.style = Border::SOLID;
    m_border.common.ptWidth = 1;
    m_currentPage = 0;
    m_actionList.setAutoDelete( true );
    searchEntry = 0L;
    replaceEntry = 0L;
    backColor = QBrush( white );
    // Default values.
    m_zoomViewModeNormal = m_doc->zoom();
    m_zoomViewModePreview = 33;

    setInstance( KWFactory::global() );
    setXMLFile( "kword.rc" );

    QObject::connect( this, SIGNAL( embeddImage( const QString & ) ),
                      this, SLOT( insertPicture( const QString & ) ) );

    setKeyCompression( TRUE );
    setAcceptDrops( TRUE );
    createKWGUI();
    initConfig();

    connect( m_doc, SIGNAL( pageNumChanged() ),
             this, SLOT( pageNumChanged()) );

    connect( m_doc, SIGNAL( docStructureChanged(int) ),
             this, SLOT( docStructChanged(int)) );

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
             this, SLOT( clipboardDataChanged() ) );

    connect( m_gui->canvasWidget(), SIGNAL(currentFrameSetEditChanged()),
             this, SLOT(slotFrameSetEditChanged()) );

    connect( m_gui->canvasWidget(), SIGNAL(currentMouseModeChanged(MouseMode)),
             this, SLOT(setTool(MouseMode)));

    // Cut and copy are directly connected to the selectionChanged signal
    if ( m_doc->isReadWrite() )
        connect( m_gui->canvasWidget(), SIGNAL(selectionChanged(bool)),
                 actionEditCut, SLOT(setEnabled(bool)) );
    else
        actionEditCut->setEnabled( false );

    connect( m_gui->canvasWidget(), SIGNAL(selectionChanged(bool)),
             actionEditCopy, SLOT(setEnabled(bool)) );

    connect( m_gui->canvasWidget(), SIGNAL(frameSelectedChanged()),
             this, SLOT(frameSelectedChanged()));

    connect( m_gui->canvasWidget(), SIGNAL(docStructChanged(int)),
             this, SLOT(docStructChanged(int)));

    connect( m_gui->canvasWidget(), SIGNAL(updateRuler()),
             this, SLOT(slotUpdateRuler()));

    if(shell())
        connect( shell(), SIGNAL( documentSaved()), m_doc,SLOT(slotDocumentInfoModifed() ) );

    m_gui->canvasWidget()->updateCurrentFormat();
    setFocusProxy( m_gui->canvasWidget() );

    KStatusBar * sb = statusBar();
    ASSERT(sb);
    m_sbPageLabel = sb ? new KStatusBarLabel( QString::null, 0, sb ) : 0;
    addStatusBarItem( m_sbPageLabel, 0 );
    m_sbFramesLabel = 0L; // Only added when frames are selected
}

KWView::~KWView()
{
    // Delete gui while we still exist ( it needs documentDeleted() )
    delete m_gui;
    delete m_sbPageLabel;
}

void KWView::initConfig()
{
  /// ### Why isn't this in KWDocument ??? ( DF )
  KConfig *config = KWFactory::global()->config();
  KSpellConfig ksconfig;
  if( config->hasGroup("KSpell kword" ) )
  {
      config->setGroup( "KSpell kword" );
      ksconfig.setNoRootAffix(config->readNumEntry ("KSpell_NoRootAffix", 0));
      ksconfig.setRunTogether(config->readNumEntry ("KSpell_RunTogether", 0));
      ksconfig.setDictionary(config->readEntry ("KSpell_Dictionary", ""));
      ksconfig.setDictFromList(config->readNumEntry ("KSpell_DictFromList", FALSE));
      ksconfig.setEncoding(config->readNumEntry ("KSpell_Encoding", KS_E_ASCII));
      ksconfig.setClient(config->readNumEntry ("KSpell_Client", KS_CLIENT_ISPELL));
      m_doc->setKSpellConfig(ksconfig);
  }
  if(config->hasGroup("Interface" ) )
  {
      config->setGroup( "Interface" );
      m_doc->setGridY(config->readNumEntry("GridY",10));
      m_doc->setGridX(config->readNumEntry("GridX",10));
      // Config-file value in mm, default 10 pt
      double indent = MM_TO_POINT( config->readDoubleNumEntry("Indent", POINT_TO_MM(10.0) ) );
      m_doc->setIndentValue(indent);
      changeNbOfRecentFiles(config->readNumEntry("NbRecentFile",10));

      m_doc->setShowRuler(config->readBoolEntry("Rulers",true));
      m_doc->setAutoSave((config->readNumEntry("AutoSave",KoDocument::defaultAutoSave()))*60);
      m_doc->setNbPagePerRow(config->readNumEntry("nbPagePerRow",4));
  }
}

void KWView::changeNbOfRecentFiles(int _nb)
{
    if ( shell() ) // 0 when embedded into konq !
        shell()->setMaxRecentItems( _nb );
}

void KWView::initGui()
{
    clipboardDataChanged();

    if ( m_gui )
        m_gui->showGUI();
    setTool( MM_EDIT );
    actionViewFrameBorders->setChecked( m_doc->getViewFrameBorders() );
    actionViewFormattingChars->setChecked( m_doc->getViewFormattingChars());

    actionViewHeader->setChecked(m_doc->isHeaderVisible());
    actionViewFooter->setChecked(m_doc->isFooterVisible());
    actionFormatDecreaseIndent->setEnabled(false);
    //setNoteType(m_doc->getNoteType(), false);

    actionFormatColor->setColor( Qt::black );

    //refresh zoom combobox
    changeZoomMenu( m_doc->zoom() );
    showZoom( m_doc->zoom() );

    MouseMode mouseMode=m_gui->canvasWidget()->getMouseMode();
    m_gui->canvasWidget()->setMouseMode( mouseMode );

    showFormulaToolbar( FALSE );

    updatePageInfo();
    slotFrameSetEditChanged();
    frameSelectedChanged();
}

void KWView::setupActions()
{
    // The actions here are grouped by menu, because this helps noticing
    // accelerator clashes.

    // -------------- File menu
    actionExtraCreateTemplate = new KAction( i18n( "&Create Template from Document..." ), 0,
                                             this, SLOT( extraCreateTemplate() ),
                                             actionCollection(), "extra_template" );
    (void) new KAction( i18n( "Statistics" ), 0, this, SLOT( fileStatistics() ), actionCollection(), "file_statistics" );

    // -------------- Edit actions
    actionEditCut = KStdAction::cut( this, SLOT( editCut() ), actionCollection(), "edit_cut" );
    actionEditCopy = KStdAction::copy( this, SLOT( editCopy() ), actionCollection(), "edit_copy" );
    actionEditPaste = KStdAction::paste( this, SLOT( editPaste() ), actionCollection(), "edit_paste" );
    KStdAction::find( this, SLOT( editFind() ), actionCollection(), "edit_find" );
    KStdAction::replace( this, SLOT( editReplace() ), actionCollection(), "edit_replace" );
    actionEditSelectAll = KStdAction::selectAll( this, SLOT( editSelectAll() ), actionCollection(), "edit_selectall" );
    actionExtraSpellCheck = KStdAction::spelling( this, SLOT( extraSpelling() ), actionCollection(), "extra_spellcheck" );


    (void) new KAction( i18n( "Serial &Letter Database..." ), 0,
                        this, SLOT( editSerialLetterDataBase() ),
                        actionCollection(), "edit_sldatabase" );

    // -------------- Frame menu
    actionEditDelFrame = new KAction( i18n( "&Delete Frame" ), 0,
                                      this, SLOT( editDeleteFrame() ),
                                      actionCollection(), "edit_delframe" );

    /*
    actionToolsEdit = new KToggleAction( i18n( "Edit &Text" ), "edittool", Key_F4,
                                         this, SLOT( toolsEdit() ),
                                         actionCollection(), "tools_edit" );
    actionToolsEdit->setExclusiveGroup( "tools" );
    actionToolsEditFrames = new KToggleAction( i18n( "Edit &Frames" ),"frame_edit", Key_F5,
                                               this, SLOT( toolsEditFrame() ),
                                               actionCollection(), "tools_editframes" );
    actionToolsEditFrames->setExclusiveGroup( "tools" );
    */

    // -------------- View menu
    actionViewPageMode = new KToggleAction( i18n( "&Page mode" ), 0,
                                            this, SLOT( viewPageMode() ),
                                            actionCollection(), "view_pagemode" );
    actionViewPageMode->setExclusiveGroup( "viewmodes" );
    actionViewPageMode->setChecked( true );
    actionViewPreviewMode = new KToggleAction( i18n( "Pre&view mode" ), 0,
                                            this, SLOT( viewPreviewMode() ),
                                            actionCollection(), "view_previewmode" );
    actionViewPreviewMode->setExclusiveGroup( "viewmodes" );

    actionViewFormattingChars = new KToggleAction( i18n( "&Formatting Characters" ), 0,
                                                   this, SLOT( viewFormattingChars() ),
                                                   actionCollection(), "view_formattingchars" );
    actionViewFormattingChars->setEnabled( false ); // ## not implemented
    actionViewFrameBorders = new KToggleAction( i18n( "Frame &Borders" ), 0,
                                                   this, SLOT( viewFrameBorders() ),
                                                   actionCollection(), "view_frameborders" );
    actionViewHeader = new KToggleAction( i18n( "&Header" ), 0,
                                          this, SLOT( viewHeader() ),
                                          actionCollection(), "view_header" );
    actionViewFooter = new KToggleAction( i18n( "Foo&ter" ), 0,
                                          this, SLOT( viewFooter() ),
                                          actionCollection(), "view_footer" );
    actionViewFootNotes = new KToggleAction( i18n( "Foot&notes" ), 0,
                                             this, SLOT( viewFootNotes() ),
                                          actionCollection(), "view_footnotes" );
    actionViewFootNotes->setEnabled( false ); // #### TODO
    actionViewFootNotes->setExclusiveGroup( "notes" );
    actionViewEndNotes = new KToggleAction( i18n( "&Endnotes" ), 0,
                                             this, SLOT( viewEndNotes() ),
                                          actionCollection(), "view_endnotes" );
    actionViewEndNotes->setExclusiveGroup( "notes" );
    actionViewEndNotes->setEnabled( false ); // #### TODO

    actionViewZoom = new KSelectAction( i18n( "Zoom" ), "viewmag", 0,
                                        actionCollection(), "view_zoom" );
    connect( actionViewZoom, SIGNAL( activated( const QString & ) ),
             this, SLOT( viewZoom( const QString & ) ) );
    actionViewZoom->setEditable(true);
    changeZoomMenu( );

    // -------------- Insert menu
    (void) new KAction( i18n( "&Table" ), "inline_table", 0,
                        this, SLOT( insertTable() ),
                        actionCollection(), "insert_table" );

    actionInsertPicture = new KAction( i18n( "&Picture Inline..." ),"inline_image", Key_F2,
                        this, SLOT( insertPicture() ),
                        actionCollection(), "insert_picture" );

    actionInsertSpecialChar = new KAction( i18n( "Sp&ecial Character..." ), "char",
                        ALT + SHIFT + Key_C,
                        this, SLOT( insertSpecialChar() ),
                        actionCollection(), "insert_specialchar" );

    actionInsertFormula = new KAction( i18n( "For&mula" ), "frame_formula", 0,
                                       this, SLOT( insertFormula() ),
                                       actionCollection(), "tools_formula" );

    actionInsertFrameBreak = new KAction( i18n( "&Hard Frame Break" ), CTRL + Key_Return,
                                          this, SLOT( insertFrameBreak() ),
                                          actionCollection(), "insert_framebreak" );

    actionInsertFootEndNote = new KAction( i18n( "&Footnote or Endnote..." ), 0,
                                           this, SLOT( insertFootNoteEndNote() ),
                                           actionCollection(), "insert_footendnote" );
    actionInsertFootEndNote->setEnabled( false ); // ### TODO

    actionInsertContents = new KAction( i18n( "Table of &Contents" ), 0,
                                        this, SLOT( insertContents() ),
                                        actionCollection(), "insert_contents" );

    actionToolsCreateText = new KToggleAction( i18n( "Te&xt Frame" ), "frame_text", Key_F6,
                                               this, SLOT( toolsCreateText() ),
                                               actionCollection(), "tools_createtext" );
    actionToolsCreateText->setExclusiveGroup( "tools" );
    actionToolsCreatePix = new KToggleAction( i18n( "P&icture Frame" ), "frame_image", Key_F7,
                                              this, SLOT( toolsCreatePix() ),
                                              actionCollection(), "tools_createpix" );
    actionToolsCreatePix->setExclusiveGroup( "tools" );
    actionToolsCreatePart = new KToggleAction( i18n( "&Object Frame" ), "frame_query", Key_F12,
                                               this, SLOT( toolsPart() ),
                                               actionCollection(), "tools_part" );
    actionToolsCreatePart->setExclusiveGroup( "tools" );

    m_variableDefMap.clear();
    actionInsertVariable = new KActionMenu( i18n( "&Variable" ),
                                            actionCollection(), "insert_variable" );
    // The last argument is only needed if a submenu is to be created
    addVariableActions( VT_FIELD, KWFieldVariable::actionTexts(), actionInsertVariable, i18n("&Property") );
    addVariableActions( VT_DATE, KWDateVariable::actionTexts(), actionInsertVariable, i18n("&Date") );
    addVariableActions( VT_TIME, KWTimeVariable::actionTexts(), actionInsertVariable, i18n("&Time") );
    addVariableActions( VT_PGNUM, KWPgNumVariable::actionTexts(), actionInsertVariable, QString::null );
    addVariableActions( VT_CUSTOM, KWCustomVariable::actionTexts(), actionInsertVariable, QString::null );
    addVariableActions( VT_SERIALLETTER, KWSerialLetterVariable::actionTexts(), actionInsertVariable, QString::null );

    actionInsertExpression = new KActionMenu( i18n( "&Expression" ),
                                            actionCollection(), "insert_expression" );
    loadexpressionActions( actionInsertExpression);

    actionEditPersonnalExpr=new KAction( i18n( "Edit personnal expression..." ), 0,
                                         this, SLOT( editPersonalExpr() ),
                                     actionCollection(), "personnal_expr" );

    actionChangeCase=new KAction( i18n( "Change case..." ), 0,
                                     this, SLOT( changeCaseOfText() ),
                                     actionCollection(), "change_case" );

    // ------------------------- Format menu
    actionFormatFont = new KAction( i18n( "&Font..." ), ALT + CTRL + Key_F,
                                    this, SLOT( formatFont() ),
                                    actionCollection(), "format_font" );
    actionFormatColor = new KColorAction( i18n( "Text Color..." ), KColorAction::TextColor, ALT + CTRL + Key_C,
                                     this, SLOT( textColor() ),
                                     actionCollection(), "format_color" );
    actionFormatParag = new KAction( i18n( "&Paragraph..." ), ALT + CTRL + Key_P,
                                     this, SLOT( formatParagraph() ),
                                     actionCollection(), "format_paragraph" );
    actionFormatFrameSet = new KAction( i18n( "F&rame/Frameset..." ), 0,
                                     this, SLOT( formatFrameSet() ),
                                     actionCollection(), "format_frameset" );
    (void) new KAction( i18n( "P&age..." ), 0,
                        this, SLOT( formatPage() ),
                        actionCollection(), "format_page" );

    (void) new KAction( i18n( "&Stylist..." ), ALT + CTRL + Key_S,
                        this, SLOT( extraStylist() ),
                        actionCollection(), "format_stylist" );

    actionFormatFontSize = new KFontSizeAction( i18n( "Font Size" ), 0,
                                              actionCollection(), "format_fontsize" );
    connect( actionFormatFontSize, SIGNAL( fontSizeChanged( int ) ),
             this, SLOT( textSizeSelected( int ) ) );
    actionFormatFontFamily = new KFontAction( i18n( "Font Family" ), 0,
                                              actionCollection(), "format_fontfamily" );
    connect( actionFormatFontFamily, SIGNAL( activated( const QString & ) ),
             this, SLOT( textFontSelected( const QString & ) ) );

    actionFormatStyle = new KSelectAction( i18n( "St&yle" ), 0,
                                           actionCollection(), "format_style" );
    connect( actionFormatStyle, SIGNAL( activated( int ) ),
             this, SLOT( textStyleSelected( int ) ) );
    updateStyleList();

    actionFormatDefault=new KAction( i18n( "Default Format" ), 0,
                                          this, SLOT( textDefaultFormat() ),
                                          actionCollection(), "text_default" );

    actionFormatBold = new KToggleAction( i18n( "&Bold" ), "text_bold", CTRL + Key_B,
                                           this, SLOT( textBold() ),
                                           actionCollection(), "format_bold" );
    actionFormatItalic = new KToggleAction( i18n( "&Italic" ), "text_italic", CTRL + Key_I,
                                           this, SLOT( textItalic() ),
                                           actionCollection(), "format_italic" );
    actionFormatUnderline = new KToggleAction( i18n( "&Underline" ), "text_under", CTRL + Key_U,
                                           this, SLOT( textUnderline() ),
                                           actionCollection(), "format_underline" );
    actionFormatStrikeOut = new KToggleAction( i18n( "&Strike out" ), "text_strike", 0 ,
                                           this, SLOT( textStrikeOut() ),
                                           actionCollection(), "format_strike" );

    actionFormatAlignLeft = new KToggleAction( i18n( "Align &Left" ), "text_left", CTRL + Key_L,
                                       this, SLOT( textAlignLeft() ),
                                       actionCollection(), "format_alignleft" );
    actionFormatAlignLeft->setExclusiveGroup( "align" );
    actionFormatAlignLeft->setChecked( TRUE );
    actionFormatAlignCenter = new KToggleAction( i18n( "Align &Center" ), "text_center", CTRL + ALT + Key_C,
                                         this, SLOT( textAlignCenter() ),
                                         actionCollection(), "format_aligncenter" );
    actionFormatAlignCenter->setExclusiveGroup( "align" );
    actionFormatAlignRight = new KToggleAction( i18n( "Align &Right" ), "text_right", CTRL + ALT + Key_R,
                                        this, SLOT( textAlignRight() ),
                                        actionCollection(), "format_alignright" );
    actionFormatAlignRight->setExclusiveGroup( "align" );
    actionFormatAlignBlock = new KToggleAction( i18n( "Align &Block" ), "text_block", CTRL + Key_J,
                                        this, SLOT( textAlignBlock() ),
                                        actionCollection(), "format_alignblock" );
    actionFormatAlignBlock->setExclusiveGroup( "align" );
    actionFormatList = new KToggleAction( i18n( "List" ), "enumList", 0,
                                              this, SLOT( textList() ),
                                              actionCollection(), "format_list" );
    actionFormatList->setExclusiveGroup( "style" );
    actionFormatSuper = new KToggleAction( i18n( "Superscript" ), "super", 0,
                                              this, SLOT( textSuperScript() ),
                                              actionCollection(), "format_super" );
    actionFormatSuper->setExclusiveGroup( "valign" );
    actionFormatSub = new KToggleAction( i18n( "Subscript" ), "sub", 0,
                                              this, SLOT( textSubScript() ),
                                              actionCollection(), "format_sub" );
    actionFormatSub->setExclusiveGroup( "valign" );

    actionFormatIncreaseIndent= new KAction( i18n( "Increase Indent" ), "format_increaseindent", 0,
                                      this, SLOT( textIncreaseIndent() ),
                                      actionCollection(), "format_increaseindent" );

    actionFormatDecreaseIndent= new KAction( i18n( "Decrease Indent" ),"format_decreaseindent", 0,
                                      this, SLOT( textDecreaseIndent() ),
                                      actionCollection(), "format_decreaseindent" );


#if KDE_VERSION < 220
    // Necessary with kdelibs-2.1.x, because those actions are only in the toolbar
    KAccel * accel = new KAccel( this );
    actionFormatBold->plugAccel( accel );
    actionFormatItalic->plugAccel( accel );
    actionFormatUnderline->plugAccel( accel );
    actionFormatStrikeOut->plugAccel( accel );

    actionFormatAlignLeft->plugAccel( accel );
    actionFormatAlignCenter->plugAccel( accel );
    actionFormatAlignRight->plugAccel( accel );
    actionFormatAlignBlock->plugAccel( accel );
#endif


    // ---------------------------- frame toolbar actions

    actionBorderOutline = new KToggleAction( i18n( "Border Outline" ), "borderoutline",
                            0, this, SLOT( borderOutline() ), actionCollection(), "border_outline" );
    actionBorderLeft = new KToggleAction( i18n( "Border Left" ), "borderleft",
                            0, this, SLOT( borderLeft() ), actionCollection(), "border_left" );
    actionBorderRight = new KToggleAction( i18n( "Border Right" ), "borderright",
                            0, this, SLOT( borderRight() ), actionCollection(), "border_right" );
    actionBorderTop = new KToggleAction( i18n( "Border Top" ), "bordertop",
                            0, this, SLOT( borderTop() ), actionCollection(), "border_top" );
    actionBorderBottom = new KToggleAction( i18n( "Border Bottom" ), "borderbottom",
                            0, this, SLOT( borderBottom() ),  actionCollection(), "border_bottom" );
    actionBorderStyle = new KSelectAction( i18n( "Border Style" ),
                            0,  actionCollection(), "border_style" );
    connect( actionBorderStyle, SIGNAL( activated( const QString & ) ),
             this, SLOT( borderStyle( const QString & ) ) );

    QStringList lst;
    lst << Border::getStyle( Border::SOLID );
    lst << Border::getStyle( Border::DASH );
    lst << Border::getStyle( Border::DOT );
    lst << Border::getStyle( Border::DASH_DOT );
    lst << Border::getStyle( Border::DASH_DOT_DOT );
    actionBorderStyle->setItems( lst );
    actionBorderWidth = new KSelectAction( i18n( "Border Width" ), 0,
                                                 actionCollection(), "border_width" );
    connect( actionBorderWidth, SIGNAL( activated( const QString & ) ),
             this, SLOT( borderWidth( const QString & ) ) );
    lst.clear();
    for ( unsigned int i = 1; i < 10; i++ )
        lst << QString::number( i );
    actionBorderWidth->setItems( lst );


     actionBorderColor = new TKSelectColorAction( i18n("Border Color"), TKSelectColorAction::LineColor, actionCollection(), "border_color" );
    connect(actionBorderColor,SIGNAL(activated()),SLOT(borderColor()));


    actionBackgroundColor = new TKSelectColorAction( i18n( "Background Color" ), TKSelectColorAction::FillColor, actionCollection(),"border_backgroundcolor");
    connect(actionBackgroundColor,SIGNAL(activated()),SLOT(backgroundColor() ));

    // ---------------------- Table menu

    actionTableInsertRow = new KAction( i18n( "&Insert Row..." ), "insert_table_row", 0,
                               this, SLOT( tableInsertRow() ),
                               actionCollection(), "table_insrow" );
    actionTableInsertCol = new KAction( i18n( "I&nsert Column..." ), "insert_table_col", 0,
                               this, SLOT( tableInsertCol() ),
                               actionCollection(), "table_inscol" );
    actionTableDelRow = new KAction( i18n( "&Delete Row..." ), "delete_table_row", 0,
                                     this, SLOT( tableDeleteRow() ),
                                     actionCollection(), "table_delrow" );
    actionTableDelCol = new KAction( i18n( "D&elete Column..." ), "delete_table_col", 0,
                                     this, SLOT( tableDeleteCol() ),
                                     actionCollection(), "table_delcol" );
    actionTableJoinCells = new KAction( i18n( "&Join Cells" ), 0,
                                        this, SLOT( tableJoinCells() ),
                                        actionCollection(), "table_joincells" );
    actionTableSplitCells= new KAction( i18n( "&Split Cells..." ), 0,
                                         this, SLOT( tableSplitCells() ),
                                         actionCollection(), "table_splitcells" );

    actionTableUngroup = new KAction( i18n( "&Ungroup Table" ), 0,
                                      this, SLOT( tableUngroupTable() ),
                                      actionCollection(), "table_ungroup" );
    actionTableDelete = new KAction( i18n( "Delete &Table" ), 0,
                                     this, SLOT( tableDelete() ),
                                     actionCollection(), "table_delete" );

    // ---------------------- Tools menu

    actionEditCustomVars = new KAction( i18n( "&Custom Variables..." ), 0,
                                        this, SLOT( editCustomVars() ),
                                        actionCollection(), "edit_customvars" );
    (void) new KAction( i18n( "&Autocorrection..." ), 0,
                        this, SLOT( extraAutoFormat() ),
                        actionCollection(), "extra_autocorrection" );

    //------------------------ Settings menu
    KStdAction::preferences(this, SLOT(configure()), actionCollection(), "configure" );
}


void KWView::refreshMenuExpression()
{
    loadexpressionActions( actionInsertExpression);
}

void KWView::loadexpressionActions( KActionMenu * parentMenu)
{
    parentMenu->popupMenu()->clear();
    QStringList files = KWFactory::global()->dirs()->findAllResources( "expression", "*.xml", TRUE );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
	createExpressionActions( parentMenu,*it );
    }
}

void KWView::createExpressionActions( KActionMenu * parentMenu,const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return;

    QDomDocument doc;
    doc.setContent( &file );
    file.close();

    QString group = "";

    parentMenu->popupMenu()->insertSeparator();

    QDomNode n = doc.documentElement().firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
        {
            if ( n.isElement() )
                {
                    QDomElement e = n.toElement();
                    if ( e.tagName() == "Type" )
                        {
                            group = i18n( e.namedItem( "TypeName" ).toElement().text().latin1() );
                            KActionMenu * subMenu = new KActionMenu( group, actionCollection() );
                            parentMenu->insert( subMenu );

                            QDomNode n2 = e.firstChild();
                            for( ; !n2.isNull(); n2 = n2.nextSibling() )
                                {

                                    if ( n2.isElement() )
                                        {
                                            QDomElement e2 = n2.toElement();
                                            if ( e2.tagName() == "Expression" )
                                                {
                                                    QString text = i18n( e2.namedItem( "Text" ).toElement().text().latin1() );
                                                    KAction * act = new KAction( text, 0, this, SLOT( insertExpression() ),
                                                                                 actionCollection(), "expression-action" );
                                                    subMenu->insert( act );
                                                }
                                        }
                                }

                            group = "";
                        }
                }
        }
}

void KWView::insertExpression()
{
 KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        edit->insertExpression(act->text());
    }
}


void KWView::addVariableActions( int type, const QStringList & texts,
                                 KActionMenu * parentMenu, const QString & menuText )
{
    // Single items go directly into parentMenu.
    // For multiple items we create a submenu.
    if ( texts.count() > 1 && !menuText.isEmpty() )
    {
        KActionMenu * subMenu = new KActionMenu( menuText, actionCollection() );
        parentMenu->insert( subMenu );
        parentMenu = subMenu;
    }
    QStringList::ConstIterator it = texts.begin();
    for ( int i = 0; it != texts.end() ; ++it, ++i )
    {
        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            VariableDef v;
            v.type = type;
            v.subtype = i;
            KAction * act = new KAction( (*it), 0, this, SLOT( insertVariable() ),
                                         actionCollection(), "var-action" );
            m_variableDefMap.insert( act, v );
            parentMenu->insert( act );
        }
    }
}

void KWView::fileStatistics()
{
    ulong charsWithSpace = 0L;
    ulong charsWithoutSpace = 0L;
    ulong words = 0L;
    ulong sentences = 0L;
    QListIterator<KWFrameSet> framesetIt( m_doc->framesetsIterator() );
    for ( ; framesetIt.current(); ++framesetIt )
    {
        KWFrameSet *frameSet = framesetIt.current();
        // Exclude headers and footers
        if ( frameSet->getFrameInfo() == FI_BODY && frameSet->isVisible() )
        {
            frameSet->statistics( charsWithSpace, charsWithoutSpace, words, sentences );
        }
    }

    KDialogBase dlg( KDialogBase::Plain, i18n( "Document Statistics" ),
                     KDialogBase::Ok, KDialogBase::Ok, this, 0, true,
                     true /* separator */ );

    QVBoxLayout * vlayout = new QVBoxLayout( dlg.plainPage() );
    vlayout->addWidget( new QLabel(
        i18n( "<qt><center>Document Statistics</center><hr><p>"
              "Characters (total count including spaces) : <b>%1</b><br/>"
              "Characters without spaces: <b>%2</b><br/>"
              "Words: <b>%3</b><br/>"
              "Sentences: <b>%4</b></p>" ).
        arg( charsWithSpace ).arg( charsWithoutSpace ).arg( words ).arg( sentences ),
                        dlg.plainPage() ) );
    dlg.setInitialSize( QSize( 400, 200 ) ); // not too good for long translations... -> use a real layout and 5 labels
    dlg.show();
}

/*======================== create GUI ==========================*/
void KWView::createKWGUI()
{
    // setup GUI
    setupActions();

    m_gui = new KWGUI( this, this );
    m_gui->setGeometry( 0, 0, width(), height() );
    m_gui->show();
}

void KWView::showFormulaToolbar( bool show )
{
  // This might not be exactly the right place. But these actions
  // must be enabled when a formula is active...
  m_doc->getFormulaDocument()->getMakeGreekAction()->setEnabled( show );
  m_doc->getFormulaDocument()->getAddGenericUpperAction()->setEnabled( show );
  m_doc->getFormulaDocument()->getAddGenericLowerAction()->setEnabled( show );
  m_doc->getFormulaDocument()->getRemoveEnclosingAction()->setEnabled( show );
  m_doc->getFormulaDocument()->getInsertSymbolAction()->setEnabled( show );
  if(shell())
      shell()->showToolbar( "formula_toolbar", show );
}

void KWView::updatePageInfo()
{
    if ( m_sbPageLabel )
    {
        KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
        if ( edit )
            m_currentPage = edit->currentFrame()->pageNum();
        /*kdDebug() << this << " KWView::updatePageInfo m_currentPage=" << m_currentPage
                  << " m_sbPageLabel=" << m_sbPageLabel
                  << endl;*/

        // ### TODO what's the current page when we have no edit object (e.g. frames are selected) ?
        // To avoid bugs, apply max page number in case a page was removed.
        m_currentPage = QMIN( m_currentPage, m_doc->getPages()-1 );

        m_sbPageLabel->setText( QString(" ")+i18n("Page %1/%2").arg(m_currentPage+1).arg(m_doc->getPages())+' ' );
        slotUpdateRuler();
    }
}

void KWView::slotUpdateRuler()
{
    QPoint rulerTopLeft = m_gui->canvasWidget()->rulerPos();
    m_gui->getHorzRuler()->setOffset( rulerTopLeft.x(), 0 );
    m_gui->getVertRuler()->setOffset( 0, rulerTopLeft.y() );
}

void KWView::pageNumChanged()
{
     docStructChanged(TextFrames);
     updatePageInfo();
}

void KWView::updateFrameStatusBarItem()
{
    KStatusBar * sb = statusBar();
    int nbFrame=m_doc->getSelectedFrames().count();
    if ( sb && nbFrame > 0 )
    {
        if ( !m_sbFramesLabel )
        {
            m_sbFramesLabel = sb ? new KStatusBarLabel( QString::null, 0, sb ) : 0;
            addStatusBarItem( m_sbFramesLabel );
        }
        if ( nbFrame == 1 )
        {
            KWUnit::Unit unit = m_doc->getUnit();
            QString unitName = m_doc->getUnitName();
            KWFrame * frame = m_doc->getFirstSelectedFrame();
            m_sbFramesLabel->setText( i18n( "Statusbar info", "%1. Frame: %2, %3  -  %4, %5 (width: %6, height: %7) (%8)" )
                                      .arg( frame->getFrameSet()->getName() )
                                      .arg( KWUnit::userValue( frame->left(), unit ) )
                                      .arg( KWUnit::userValue( frame->top(), unit ) )
                                      .arg( KWUnit::userValue( frame->right(), unit ) )
                                      .arg( KWUnit::userValue( frame->bottom(), unit ) )
                                      .arg( KWUnit::userValue( frame->width(), unit ) )
                                      .arg( KWUnit::userValue( frame->height(), unit ) )
                                      .arg( unitName ) );
        } else
            m_sbFramesLabel->setText( i18n( "%1 frames selected" ).arg( nbFrame ) );
    }
    else if ( sb && m_sbFramesLabel )
    {
        removeStatusBarItem( m_sbFramesLabel );
        delete m_sbFramesLabel;
        m_sbFramesLabel = 0L;
    }
}

void KWView::clipboardDataChanged()
{
    // Can we paste into something ?
    if ( !m_gui || !m_gui->canvasWidget()->currentFrameSetEdit() || !m_doc->isReadWrite() )
    {
        actionEditPaste->setEnabled(false);
        return;
    }
    // Is there plain text in the clipboard ?
    if ( !QApplication::clipboard()->text().isEmpty() )
    {
        actionEditPaste->setEnabled(true);
        return;
    }
    // Is there kword XML in the clipboard ?
    QMimeSource *data = QApplication::clipboard()->data();
    actionEditPaste->setEnabled( data->provides( MIME_TYPE ) );

}

/*=========================== file print =======================*/
void KWView::setupPrinter( KPrinter &prt )
{
#ifdef HAVE_KDEPRINT
    prt.setPageSelection( KPrinter::ApplicationSide );
    prt.setCurrentPage( currentPage() + 1 );
#endif
    prt.setMinMax( 1, m_doc->getPages() );

    KoPageLayout pgLayout = m_doc->pageLayout();

    prt.setPageSize( static_cast<KPrinter::PageSize>( KoPageFormat::printerPageSize( pgLayout.format ) ) );

    if ( pgLayout.orientation == PG_LANDSCAPE || pgLayout.format == PG_SCREEN )
        prt.setOrientation( KPrinter::Landscape );
    else
        prt.setOrientation( KPrinter::Portrait );
}

void KWView::print( KPrinter &prt )
{
    // Don't repaint behind the print dialog until we're done zooming/unzooming the doc
    m_gui->canvasWidget()->setUpdatesEnabled(false);
    m_gui->canvasWidget()->viewport()->setCursor( waitCursor );

    prt.setFullPage( true );

    // ### HACK: disable zooming-when-printing if embedded parts are used.
    // No koffice app supports zooming in paintContent currently.
    bool doZoom = true;
    QListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() && doZoom ; ++fit )
        if ( fit.current()->getFrameType() == FT_PART )
            doZoom = false;

    int oldZoom = m_doc->zoom();
    // We don't get valid metrics from the printer - and we want a better resolution
    // anyway (it's the PS driver that takes care of the printer resolution).
    QPaintDeviceMetrics metrics( &prt );
    //m_doc->setZoomAndResolution( 100, metrics.logicalDpiX(), metrics.logicalDpiY(), false );
    if ( doZoom )
        m_doc->setZoomAndResolution( 100, 300, 300, false );
    //kdDebug() << "KWView::print zoom&res set" << endl;

    bool serialLetter = FALSE;
#if 0
    QList<KWVariable> *vars = m_doc->getVariables();
    KWVariable *v = 0;
    for ( v = vars->first(); v; v = vars->next() ) {
        if ( v->getType() == VT_SERIALLETTER ) {
            serialLetter = TRUE;
            break;
        }
    }

    if ( !m_doc->getSerialLetterDataBase() ||
         m_doc->getSerialLetterDataBase()->getNumRecords() == 0 )
        serialLetter = FALSE;
#endif
    //float left_margin = 0.0;
    //float top_margin = 0.0;

    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPageLayout( pgLayout, cl, hf );
    KoPageLayout oldPGLayout = pgLayout;

    if ( pgLayout.format == PG_SCREEN )
    {
        //left_margin = 25.8;
        //top_margin = 15.0;
        pgLayout.ptLeft += 25.8;         // Not sure why we need this....
        pgLayout.ptRight += 15.0;
        // TODO the other units. Well, better get rid of the multiple-unit thing.
        m_doc->setPageLayout( pgLayout, cl, hf );
    }

    if ( !serialLetter ) {
        QPainter painter;
        painter.begin( &prt );
        if ( doZoom )
            painter.scale( metrics.logicalDpiX() / 300.0, metrics.logicalDpiY() / 300.0 );
        m_gui->canvasWidget()->print( &painter, &prt );
        painter.end();
    } else {
#if 0
        QPainter painter;
        painter.begin( &prt );
        for ( int i = 0; i < m_doc->getSerialLetterDataBase()->getNumRecords(); ++i ) {
            m_doc->setSerialLetterRecord( i );
            m_gui->canvasWidget()->print( &painter, &prt, left_margin, top_margin );
            if ( i < m_doc->getSerialLetterDataBase()->getNumRecords() - 1 )
                prt.newPage();
        }
        m_doc->setSerialLetterRecord( -1 );
        painter.end();
#endif
    }

    if ( pgLayout.format == PG_SCREEN )
        m_doc->setPageLayout( oldPGLayout, cl, hf );

    m_doc->setZoomAndResolution( oldZoom, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY(), false );
    //kdDebug() << "KWView::print zoom&res reset" << endl;

    m_gui->canvasWidget()->setUpdatesEnabled(true);
    m_gui->canvasWidget()->viewport()->setCursor( ibeamCursor );
    m_doc->repaintAllViews();
}

void KWView::showFormat( const QTextFormat &currentFormat )
{
    // update the gui with the current format.
    //kdDebug() << "KWView::setFormat font family=" << currentFormat.font().family() << endl;
    actionFormatFontFamily->setFont( currentFormat.font().family() );
    actionFormatFontSize->setFontSize( currentFormat.font().pointSize() );
    actionFormatBold->setChecked( currentFormat.font().bold());
    actionFormatItalic->setChecked( currentFormat.font().italic() );
    actionFormatUnderline->setChecked( currentFormat.font().underline());
    actionFormatStrikeOut->setChecked( currentFormat.font().strikeOut());

    actionFormatColor->setColor( currentFormat.color() );

    switch(currentFormat.vAlign())
      {
      case QTextFormat::AlignSuperScript:
	{
	  actionFormatSub->setChecked( false );
	  actionFormatSuper->setChecked( true );
	  break;
	}
      case QTextFormat::AlignSubScript:
	{
	  actionFormatSub->setChecked( true );
	  actionFormatSuper->setChecked( false );
	  break;
	}
      case QTextFormat::AlignNormal:
      default:
	{
	  actionFormatSub->setChecked( false );
	  actionFormatSuper->setChecked( false );
	  break;
	}
      }

}

void KWView::showRulerIndent( double _leftMargin, double _firstLine )
{
  KoRuler * hRuler = m_gui ? m_gui->getHorzRuler() : 0;
  if ( hRuler )
  {
      hRuler->setFirstIndent( KWUnit::userValue( _firstLine + _leftMargin, m_doc->getUnit() ) );
      hRuler->setLeftIndent( KWUnit::userValue( _leftMargin, m_doc->getUnit() ) );
      actionFormatDecreaseIndent->setEnabled( _leftMargin>0);
  }
}

void KWView::showAlign( int align ) {
    switch ( align ) {
        case Qt3::AlignAuto: // In left-to-right mode it's align left. TODO: alignright if text->isRightToLeft()
        case Qt::AlignLeft:
            actionFormatAlignLeft->setChecked( TRUE );
            break;
        case Qt::AlignCenter:
            actionFormatAlignCenter->setChecked( TRUE );
            break;
        case Qt::AlignRight:
            actionFormatAlignRight->setChecked( TRUE );
            break;
        case Qt3::AlignJustify: // Make this Qt:AlignJustify after the merge with Qt3
            actionFormatAlignBlock->setChecked( TRUE );
            break;
    }
}

void KWView::showCounter( Counter &c )
{
    actionFormatList->setChecked( c.numbering() == Counter::NUM_LIST );
}

void KWView::showFrameBorders( Border _left, Border _right,
                               Border _top, Border _bottom )
{
    showParagBorders( _left, _right, _top, _bottom );
}

void KWView::showParagBorders( Border left, Border right,
                               Border top, Border bottom )
{
    if ( m_border.left != left || m_border.right != right || m_border.top != top || m_border.bottom != bottom )
    {
        m_border.left = left;
        m_border.right = right;
        m_border.top = top;
        m_border.bottom = bottom;

        actionBorderLeft->setChecked( left.ptWidth > 0 );
        actionBorderRight->setChecked( right.ptWidth > 0 );
        actionBorderTop->setChecked( top.ptWidth > 0 );
        actionBorderBottom->setChecked( bottom.ptWidth > 0 );
        actionBorderOutline->setChecked(
            actionBorderLeft->isChecked() &&
            actionBorderRight->isChecked() &&
            actionBorderTop->isChecked() &&
            actionBorderBottom->isChecked());

        if ( left.ptWidth > 0 ) {
            m_border.common = left;
            borderShowValues();
        }
        if ( right.ptWidth > 0 ) {
            m_border.common = right;
            borderShowValues();
        }
        if ( top.ptWidth > 0 ) {
            m_border.common = top;
            borderShowValues();
        }
        if ( bottom.ptWidth > 0 ) {
            m_border.common = bottom;
            borderShowValues();
        }
    }
}

void KWView::updateReadWrite( bool readwrite )
{
    // Disable everything if readonly.
    // But don't enable everything if readwrite. E.g. "undo" must be initially disabled.
    if ( !readwrite )
    {
        QValueList<KAction*> actions = actionCollection()->actions();
        QValueList<KAction*>::ConstIterator aIt = actions.begin();
        QValueList<KAction*>::ConstIterator aEnd = actions.end();
        for (; aIt != aEnd; ++aIt )
            (*aIt)->setEnabled( readwrite );
        // A few harmless actions
        actionEditCopy->setEnabled( true );
        actionViewFormattingChars->setEnabled( true );
        actionViewFrameBorders->setEnabled( true );
        actionViewHeader->setEnabled( true );
        actionViewFooter->setEnabled( true );
        actionViewFootNotes->setEnabled( true );
        actionViewEndNotes->setEnabled( true );
        actionViewZoom->setEnabled( true );
        // Well, the view menu doesn't appear in konq, so this is currently useless...
    }
}

void KWView::setTool( MouseMode _mouseMode )
{
    switch ( _mouseMode ) {
    case MM_EDIT:
    case MM_CREATE_TABLE:
    case MM_CREATE_FORMULA:
        // No tool to activate for this mode -> deselect all the others
        actionToolsCreateText->setChecked( false );
        actionToolsCreatePix->setChecked( false );
        actionToolsCreatePart->setChecked( false );
        break;
    case MM_CREATE_TEXT:
        actionToolsCreateText->setChecked( true );
        break;
    case MM_CREATE_PIX:
        actionToolsCreatePix->setChecked( true );
        break;
    case MM_CREATE_PART:
        actionToolsCreatePart->setChecked( true );
        break;
    }

    actionTableJoinCells->setEnabled( FALSE );
    actionTableSplitCells->setEnabled( FALSE );
    actionFormatFrameSet->setEnabled(FALSE);
}

void KWView::showStyle( const QString & styleName )
{
    QListIterator<KWStyle> styleIt( m_doc->styleList() );
    for ( int pos = 0 ; styleIt.current(); ++styleIt, ++pos )
    {
        if ( styleIt.current()->name() == styleName ) {
            actionFormatStyle->setCurrentItem( pos );
            return;
        }
    }
}

void KWView::updateStyleList()
{
    QString currentStyle = actionFormatStyle->currentText();
    QStringList lst;
    QListIterator<KWStyle> styleIt( m_doc->styleList() );
    for (; styleIt.current(); ++styleIt ) {
        lst << i18n("KWord style", styleIt.current()->name().utf8()); // try to translate the name, if standard
    }
    actionFormatStyle->setItems( lst );
    showStyle( currentStyle );
}

void KWView::editCut()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->cut();
}

void KWView::editCopy()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->copy();
}

void KWView::editPaste()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->paste();
}

void KWView::editSelectAll()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->selectAll();
}

void KWView::editFind()
{
    if (!searchEntry)
        searchEntry = new KWSearchContext();
    KWSearchDia dialog( m_gui->canvasWidget(), "find", searchEntry );
    if ( dialog.exec() == QDialog::Accepted )
    {
        KWFindReplace find( m_gui->canvasWidget(), &dialog );
        find.proceed();
    }
}

void KWView::editReplace()
{
    if (!searchEntry)
        searchEntry = new KWSearchContext();
    if (!replaceEntry)
        replaceEntry = new KWSearchContext();
    KWReplaceDia dialog( m_gui->canvasWidget(), "replace", searchEntry, replaceEntry );
    if ( dialog.exec() == QDialog::Accepted )
    {
        KWFindReplace replace( m_gui->canvasWidget(), &dialog );
        replace.proceed();
    }
}

void KWView::editDeleteFrame()
{
    QList<KWFrame> frames=m_doc->getSelectedFrames();
    ASSERT( frames.count() == 1 ); // the action isn't enabled otherwise.
    if( frames.count() != 1)
        return;
    KWFrame *theFrame = frames.at(0);
    KWFrameSet *fs = theFrame->getFrameSet();

    ASSERT( !fs->isAHeader() ); // the action is disabled for such cases
    ASSERT( !fs->isAFooter() );
    if ( fs->isAFooter() || fs->isAHeader() )
        return;

    // frame is part of a table?
    if ( fs->getGroupManager() ) {
        int result;
        result = KMessageBox::warningContinueCancel(this,
                                                    i18n( "You are about to delete a table.\n"
                                                          "Doing so will delete all the text in the table.\n"
                                                          "Are you sure you want to do that?"), i18n("Delete Table"), i18n("&Delete"));
        if (result != KMessageBox::Continue)
            return;
        m_doc->deleteTable( fs->getGroupManager() );
        m_gui->canvasWidget()->emitFrameSelectedChanged();
        return;
    }

    if ( fs->getNumFrames() == 1 && fs->getFrameType() == FT_TEXT) {
        if ( m_doc->processingType() == KWDocument::WP && m_doc->getFrameSetNum( fs ) == 0 )
            return;

        KWTextFrameSet * textfs = dynamic_cast<KWTextFrameSet *>(m_doc->getFrameSet( 0 ) );
        if ( !textfs )
            return;

        QTextDocument * textdoc = textfs->textDocument();
        QTextParag * parag = textdoc->firstParag();
        if ( parag && parag->string()->length() > 0 ) {
            int result = KMessageBox::warningContinueCancel(this,
                                                        i18n( "You are about to delete the last Frame of the\n"
                                                              "Frameset '%1'.\n"
                                                              "The contents of this Frameset will not appear\n"
                                                              "anymore!\n\n"
                                                              "Are you sure you want to do that?").arg(fs->getName()),
                                                        i18n("Delete Frame"), i18n("&Delete"));

            if (result == KMessageBox::Continue)
            {
                m_doc->deleteFrame( theFrame );
                m_gui->canvasWidget()->emitFrameSelectedChanged();
            }
            return;
        }

    }

    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("Do you want to delete this frame?"),
                                                    i18n("Delete Frame"),
                                                    i18n("&Delete"));
    if (result == KMessageBox::Continue)
    {
        m_doc->deleteFrame( theFrame );
        m_gui->canvasWidget()->emitFrameSelectedChanged();
    }
}

void KWView::editCustomVars()
{
    KWCustomVariablesDia dia( this, m_doc->getVariables() );
    if(dia.exec())
        m_doc->recalcVariables( VT_CUSTOM );
}

void KWView::editSerialLetterDataBase()
{
#if 0
    KWSerialLetterEditor *dia = new KWSerialLetterEditor( this, m_doc->getSerialLetterDataBase() );
    dia->exec();
    m_gui->canvasWidget()->recalcWholeText();
    m_gui->canvasWidget()->repaintScreen( FALSE );
    delete dia;
#endif
}

void KWView::viewPageMode()
{
    if ( actionViewPageMode->isChecked() )
    {
        m_zoomViewModePreview = m_doc->zoom();
        showZoom( m_zoomViewModeNormal );
        setZoom( m_zoomViewModeNormal, false );
        m_gui->canvasWidget()->switchViewMode( new KWViewModeNormal( m_gui->canvasWidget()) );
    }
    else
        actionViewPageMode->setChecked( true ); // always one has to be checked !
}

void KWView::viewPreviewMode()
{
    if ( actionViewPreviewMode->isChecked() )
    {
        m_zoomViewModeNormal = m_doc->zoom();
        showZoom( m_zoomViewModePreview );
        setZoom( m_zoomViewModePreview, false );
        m_gui->canvasWidget()->switchViewMode( new KWViewModePreview( m_gui->canvasWidget(),m_doc->getNbPagePerRow() ) );
    }
    else
        actionViewPreviewMode->setChecked( true ); // always one has to be checked !
}

void KWView::changeZoomMenu( int zoom )
{
    QStringList lst;
    if(zoom>0)
    {
        QValueList<int> list;
        QString z;
        int val;
        bool ok;
        QStringList itemsList = actionViewZoom->items();
        for (QStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
        {
            z = (*it).replace( QRegExp( "%" ), "" );
            z = z.simplifyWhiteSpace();
            val=z.toInt(&ok);
            //zoom : limit inferior=10
            if(ok && val>9  &&list.contains(val)==0)
                list.append( val );
        }

        qHeapSort( list );

        for (QValueList<int>::Iterator it = list.begin() ; it != list.end() ; ++it)
            lst.append( (QString::number(*it)+'%') );
    }
    else
    {
          lst << "33%";
          lst << "50%";
          lst << "75%";
          lst << "100%";
          lst << "125%";
          lst << "150%";
          lst << "200%";
          lst << "250%";
          lst << "350%";
          lst << "400%";
          lst << "450%";
          lst << "500%";
    }
    actionViewZoom->setItems( lst );
}

void KWView::showZoom( int zoom )
{
    QStringList list = actionViewZoom->items();
    QString zoomStr = QString::number( zoom ) + '%';
    actionViewZoom->setCurrentItem( list.findIndex(zoomStr)  );
}

void KWView::viewFormattingChars()
{
    m_doc->setViewFormattingChars(actionViewFormattingChars->isChecked());
    m_gui->canvasWidget()->repaintAll();
}

void KWView::viewFrameBorders()
{
    m_doc->setViewFrameBorders(actionViewFrameBorders->isChecked());
    m_gui->canvasWidget()->repaintAll();
}

void KWView::viewHeader()
{
    m_doc->setHeaderVisible( actionViewHeader->isChecked() );
    m_doc->updateResizeHandles( );
    /*
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPaeLayout( pgLayout, cl, hf );
    m_doc->setPageLayout( pgLayout, cl, hf );
    */
}

void KWView::viewFooter()
{
    m_doc->setFooterVisible( actionViewFooter->isChecked() );
    m_doc->updateResizeHandles( );
    /*
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPageLayout( pgLayout, cl, hf );
    m_doc->setPageLayout( pgLayout, cl, hf );
    */
}

void KWView::viewFootNotes()
{
#if 0
    if ( !actionViewFootNotes->isChecked() )
        return;
    setNoteType( KWFootNoteManager::FootNotes);
#endif
}

void KWView::viewEndNotes()
{
#if 0
    if ( !actionViewEndNotes->isChecked() )
        return;
    setNoteType( KWFootNoteManager::EndNotes);
#endif
}

#if 0
void KWView::setNoteType( KWFootNoteManager::NoteType nt, bool change)
{
    if (change)
        m_doc->setNoteType( nt );
    switch (nt)
    {
      case KWFootNoteManager::FootNotes:
      actionViewFootNotes->setChecked( TRUE );
      actionInsertFootEndNote->setText(i18n("&Footnote"));
      break;
    case KWFootNoteManager::EndNotes:
      default:
      actionViewEndNotes->setChecked( TRUE );
      actionInsertFootEndNote->setText(i18n("&Endnote"));
      break;
    }
}
#endif

void KWView::viewZoom( const QString &s )
{
    QString z( s );
    bool ok=false;

    z = z.replace( QRegExp( "%" ), "" );
    z = z.simplifyWhiteSpace();
    int zoom = z.toInt(&ok);
    //bad value
    if(!ok)
        zoom=m_doc->zoom();
    else if(zoom<10 ) //zoom should be >10
        zoom=m_doc->zoom();
    //refresh menu
    changeZoomMenu( zoom );
    //refresh menu item
    showZoom(zoom);
    //apply zoom if zoom!=m_doc->zoom()
    if(zoom != m_doc->zoom() )
    {
        setZoom( zoom, true );

        m_doc->updateResizeHandles();
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
            edit->ensureCursorVisible();
    }

    m_gui->canvasWidget()->setFocus();

}

void KWView::setZoom( int zoom, bool updateViews )
{
    m_doc->setZoomAndResolution( zoom, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY(), updateViews );
    m_gui->getHorzRuler()->setZoom( m_doc->zoomedResolutionX() );
    m_gui->getVertRuler()->setZoom( m_doc->zoomedResolutionY() );
}

void KWView::insertPicture()
{
    insertPicture( selectPicture() );
}

void KWView::insertPicture(const QString &filename)
{
    if ( !filename.isEmpty() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        {
            // ### Old way: edit->insertPicture( filename );
            // New way:
            KWPictureFrameSet *frameset = new KWPictureFrameSet( m_doc, QString::null );
            QPixmap pix( filename );
            // This ensures 1-1 at 100% on screen, but allows zooming and printing with correct DPI values
            int width = qRound( (double)pix.width() * m_doc->zoomedResolutionX() / POINT_TO_INCH( QPaintDevice::x11AppDpiX() ) );
            int height = qRound( (double)pix.height() * m_doc->zoomedResolutionY() / POINT_TO_INCH( QPaintDevice::x11AppDpiY() ) );
            // Apply reasonable limits
            width = QMIN( width, m_doc->paperWidth() - m_doc->leftBorder() - m_doc->rightBorder() - m_doc->zoomItX( 10 ) );
            height = QMIN( height, m_doc->paperHeight() - m_doc->topBorder() - m_doc->bottomBorder() - m_doc->zoomItY( 10 ) );

            frameset->setFileName( filename, QSize( width, height ) );
            m_doc->addFrameSet( frameset, false ); // done first since the frame number is stored in the undo/redo
            KWFrame *frame = new KWFrame(frameset, 0, 0, m_doc->unzoomItX( width ), m_doc->unzoomItY( height ) );
            frameset->addFrame( frame, false );
            edit->insertFloatingFrameSet( frameset, i18n("Insert Picture Inline") );
            frameset->finalize(); // done last since it triggers a redraw
        }
    }
}

void KWView::insertSpecialChar()
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    QString f = edit->textFontFamily();
    QChar c=' ';
    KCharSelectDia *dlg=new KCharSelectDia( this, "insert special char", f,c );
    connect(dlg,SIGNAL(insertChar(QChar,const QString &)),this,SLOT(slotSpecialChar(QChar,const QString &)));
    dlg->show();
    delete dlg;

}

void KWView::slotSpecialChar(QChar c, const QString &_font)
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    edit->setFamily( _font );
    edit->insertSpecialChar(c);

}

void KWView::insertFrameBreak()
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    // We use "new paragraph" and "frame break before", because this allows using 'Backspace'
    // to get rid of it. With "frame break after" we'd need to open the paragraph dialog.
    edit->insertParagraph();
    edit->setPageBreaking( edit->currentParagLayout().pageBreaking | KWParagLayout::HardFrameBreakBefore );
}

void KWView::insertVariable()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        VariableDefMap::Iterator it = m_variableDefMap.find( act );
        if ( it == m_variableDefMap.end() )
            kdWarning() << "Action not found in m_variableDefMap." << endl;
        else
        {
            edit->insertVariable( (*it).type, (*it).subtype );
        }
    }
}

void KWView::insertFootNoteEndNote()
{
#if 0
    int start = m_doc->getFootNoteManager().findStart( m_gui->canvasWidget()->getCursor() );

    if ( start == -1 )
    {
        KMessageBox::sorry( this,
                            i18n( "Sorry, you can only insert footnotes or\n"
                                  "endnotes into the first frameset."),
                            i18n("Insert Footnote/Endnote"));
    } else {
        KWFootNoteDia dia( 0L, "", m_doc, m_gui->canvasWidget(), start,
                 m_doc->getNoteType() == KWFootNoteManager::FootNotes );
        dia.show();
    }
#endif
}

void KWView::renameButtonTOC(const QString & _name)
{
   KActionCollection * coll = actionCollection();
   coll->action("insert_contents")->setText(_name);
}

void KWView::insertContents()
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
        edit->insertTOC();
}

void KWView::formatFont()
{
    //kdDebug(32002) << "KWView::formatFont" << endl;
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        KWFontDia *fontDia = new KWFontDia( this, "", edit->textFont(),
                                            actionFormatSub->isChecked(), actionFormatSuper->isChecked(),
                                            edit->textColor() );
        fontDia->show();
        int flags = fontDia->changedFlags();
        kdDebug() << "KWView::formatFont changedFlags = " << flags << endl;
        if ( flags )
        {
            // The "change all the format" call
            edit->setFont(fontDia->getNewFont(),
                          fontDia->getSubScript(), fontDia->getSuperScript(),
                          fontDia->color(),
                          flags);
        }

        delete fontDia;
    }
    m_gui->canvasWidget()->setFocus();
}

void KWView::formatParagraph()
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        KWParagDia *paragDia = new KWParagDia( this, "",
                                               KWParagDia::PD_SPACING | KWParagDia::PD_ALIGN |
                                               KWParagDia::PD_BORDERS |
                                               KWParagDia::PD_NUMBERING | KWParagDia::PD_TABS, m_doc );
        paragDia->setCaption( i18n( "Paragraph settings" ) );

        // Initialize the dialog from the current paragraph's settings
        KWParagLayout lay = static_cast<KWTextParag *>(edit->getCursor()->parag())->paragLayout();
        paragDia->setParagLayout( lay );
        if(!paragDia->exec())
            return;

        // TODO a macro command with all the changes in it !
        // undo should do all in one step.
        if(paragDia->isLeftMarginChanged())
        {
            edit->setMargin( QStyleSheetItem::MarginLeft, paragDia->leftIndent() );
            m_gui->getHorzRuler()->setLeftIndent( KWUnit::userValue( paragDia->leftIndent(), m_doc->getUnit() ) );
        }

        if(paragDia->isRightMarginChanged())
        {
            edit->setMargin( QStyleSheetItem::MarginRight, paragDia->rightIndent() );
            //koRuler doesn't support setRightIndent
            //m_gui->getHorzRuler()->setRightIndent( KWUnit::userValue( paragDia->rightIndent(), m_doc->getUnit() ) );
        }
        if(paragDia->isSpaceBeforeChanged())
            edit->setMargin( QStyleSheetItem::MarginTop, paragDia->spaceBeforeParag() );

        if(paragDia->isSpaceAfterChanged())
            edit->setMargin( QStyleSheetItem::MarginBottom, paragDia->spaceAfterParag() );

        if(paragDia->isFirstLineChanged())
        {
            edit->setMargin( QStyleSheetItem::MarginFirstLine, paragDia->firstLineIndent());
            m_gui->getHorzRuler()->setFirstIndent(
                KWUnit::userValue( paragDia->leftIndent() + paragDia->firstLineIndent(), m_doc->getUnit() ) );
        }

        if(paragDia->isAlignChanged())
            edit->setAlign( paragDia->align() );

        if(paragDia->isCounterChanged())
            edit->setCounter( paragDia->counter() );

        if(paragDia->listTabulatorChanged())
            edit->setTabList( paragDia->tabListTabulator() );

        if(paragDia->isLineSpacingChanged())
            edit->setLineSpacing( paragDia->lineSpacing() );

        if(paragDia->isBorderChanged())
            edit->setBorders( paragDia->leftBorder(), paragDia->rightBorder(),
                              paragDia->topBorder(), paragDia->bottomBorder() );

        if ( paragDia->isPageBreakingChanged() )
            edit->setPageBreaking( paragDia->pageBreaking() );

        delete paragDia;
    }

}

void KWView::formatPage()
{
    if( !m_doc->isReadWrite())
        return;
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter kwhf;
    m_doc->getPageLayout( pgLayout, cl, kwhf );

    pageLayout tmpOldLayout;
    tmpOldLayout._pgLayout=pgLayout;
    tmpOldLayout._cl=cl;
    tmpOldLayout._hf=kwhf;

    KoHeadFoot hf;
    int flags = FORMAT_AND_BORDERS | KW_HEADER_AND_FOOTER | DISABLE_UNIT;
    if ( m_doc->processingType() == KWDocument::WP )
        flags = flags | COLUMNS;
    else
        flags = flags | DISABLE_BORDERS;

    if ( KoPageLayoutDia::pageLayout( pgLayout, hf, cl, kwhf, flags ) ) {

        pageLayout tmpNewLayout;
        tmpNewLayout._pgLayout=pgLayout;
        tmpNewLayout._cl=cl;
        tmpNewLayout._hf=kwhf;

        KWTextFrameSetEdit *edit = currentTextEdit();
        if (edit)
            edit->textFrameSet()->clearUndoRedoInfo();
        KWPageLayoutCommand *cmd =new KWPageLayoutCommand( i18n("Change Layout"),m_doc,tmpOldLayout,tmpNewLayout ) ;
        m_doc->addCommand(cmd);

        m_doc->setPageLayout( pgLayout, cl, kwhf );
        m_doc->updateRuler();

        m_doc->updateResizeHandles();
#if 0
        m_gui->canvasWidget()->frameSizeChanged( pgLayout );
#endif
    }
}

void KWView::formatFrameSet()
{
    if ( m_doc->getFirstSelectedFrame() )
      m_gui->canvasWidget()->editFrameProperties();
    else // TODO enable/disable the action depending on whether a frame is selected, instead
        KMessageBox::sorry( this,
                            i18n("Sorry, you have to select a frame first."),
                            i18n("Format Frameset"));
}

void KWView::extraSpelling()
{
    if (m_kspell) return; // Already in progress
    m_spellCurrFrameSetNum = -1;
    startKSpell();
}

void KWView::extraAutoFormat()
{
    m_doc->getAutoFormat()->readConfig();
    KWAutoFormatDia dia( this, 0, m_doc->getAutoFormat() );
    dia.show();
}

void KWView::extraStylist()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->hideCursor();
    KWStyleManager * styleManager = new KWStyleManager( this, m_doc );
    styleManager->show();
    delete styleManager;
    if ( edit )
        edit->showCursor();
}

void KWView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    double ratio = m_doc->ptPaperHeight() / m_doc->ptPaperWidth();
    if ( ratio > 1 )
        width = qRound( 60 / ratio );
    else
        height = qRound( 60 / ratio );
    double zoom = (double)width / m_doc->ptPaperWidth();
    int oldZoom = m_doc->zoom();
    setZoom( qRound( 100 * zoom ), false );
    kdDebug() << "KWView::extraCreateTemplate ratio=" << ratio << " preview size: " << width << "," << height
              << " zoom:" << zoom << endl;
    QPixmap pix( width, height );
    pix.fill( Qt::white );
    QPainter painter;
    painter.begin( &pix );
    QRect pageRect( 0, 0, m_doc->paperWidth(), m_doc->paperHeight() );

    KWViewModeNormal * viewMode = new KWViewModeNormal( 0L );
    QColorGroup cg = QApplication::palette().active();

    // Draw all framesets contents
    QListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() ; ++fit )
    {
        KWFrameSet * frameset = fit.current();
        if ( frameset->isVisible() && !frameset->isFloating() )
            frameset->drawContents( &painter, pageRect, cg, false, false, 0L, viewMode );
    }

    painter.end();
    delete viewMode;
    setZoom( oldZoom, false );

    KTempFile tempFile( QString::null, ".kwt" );
    tempFile.setAutoDelete(true);

    m_doc->saveNativeFormat( tempFile.name() );

    KoTemplateCreateDia::createTemplate( "kword_template", KWFactory::global(),
                                         tempFile.name(), pix, this );

    KWFactory::global()->dirs()->addResourceType("kword_template",
                                                    KStandardDirs::kde_default( "data" ) +
                                                    "kword/templates/");
}

#if 0
void KWView::toolsEdit()
{
    if ( actionToolsEdit->isChecked() )
        m_gui->canvasWidget()->setMouseMode( MM_EDIT );
    else
        actionToolsEdit->setChecked( true ); // always one has to be checked !
}

void KWView::toolsEditFrame()
{
    if ( actionToolsEditFrames->isChecked() )
        m_gui->canvasWidget()->setMouseMode( MM_EDIT_FRAME );
    else
        actionToolsEditFrames->setChecked( true ); // always one has to be checked !
}
#endif

void KWView::toolsCreateText()
{
    if ( actionToolsCreateText->isChecked() )
        m_gui->canvasWidget()->setMouseMode( MM_CREATE_TEXT );
    else
        actionToolsCreateText->setChecked( true ); // always one has to be checked !
}

void KWView::toolsCreatePix()
{
    if ( !actionToolsCreatePix->isChecked() )
    {
        actionToolsCreatePix->setChecked( true ); // always one has to be checked !
        return;
    }
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );


    QString file = selectPicture();
    if ( !file.isEmpty() ) {
        m_gui->canvasWidget()->setMouseMode( MM_CREATE_PIX );
        m_gui->canvasWidget()->setPixmapFilename( file );
    } else
        m_gui->canvasWidget()->setMouseMode( MM_EDIT );
}

QString KWView::selectPicture()
{
    KFileDialog fd( QString::null, KImageIO::pattern(KImageIO::Writing), 0, 0, TRUE );
    fd.setCaption(i18n("Insert Picture"));
    fd.setPreviewWidget( new Preview( &fd ) );
    KURL url;
    if ( fd.exec() == QDialog::Accepted )
        url = fd.selectedURL();

    if( url.isEmpty() )
      return QString::null;

    if( !url.isLocalFile() )
    {
      KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
      return QString::null;
    }

    return url.path();
}

void KWView::insertTable()
{
    KWCanvas * canvas = m_gui->canvasWidget();
    KWTableDia *tableDia = new KWTableDia( this, 0, canvas, m_doc,
                                           canvas->tableRows(),
                                           canvas->tableCols(),
                                           canvas->tableWidthMode(),
                                           canvas->tableHeightMode(),
                                           canvas->tableIsFloating() );
    tableDia->setCaption( i18n( "Insert Table" ) );
    tableDia->show();
    delete tableDia;
}

void KWView::insertFormula()
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        KWFormulaFrameSet *frameset = new KWFormulaFrameSet( m_doc, QString::null );
        m_doc->addFrameSet( frameset, false ); // done first since the frame number is stored in the undo/redo
        KWFrame *frame = new KWFrame(frameset, 0, 0, 10, 10 );
        frameset->addFrame( frame, false );
        edit->insertFloatingFrameSet( frameset, i18n("Insert Formula") );
        frameset->finalize(); // done last since it triggers a redraw

        // Strange, seems we need this - hmm, do we, still ?
        edit->getCursor()->parag()->invalidate( 0 ); // and that's done by KWTextParag::setCustomItem. Hmm.
        edit->getCursor()->parag()->setChanged( true );
        m_doc->slotRepaintChanged( edit->frameSet() );
        m_doc->refreshDocStructure(FT_FORMULA);
    }
}

void KWView::toolsPart()
{
    if ( !actionToolsCreatePart->isChecked() )
    {
        actionToolsCreatePart->setChecked( true ); // always one has to be checked !
        return;
    }
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );

    KoDocumentEntry pe = KoPartSelectDia::selectPart( this );
    if ( pe.isEmpty() )
        return;

    m_gui->canvasWidget()->setMouseMode( MM_CREATE_PART );
    m_gui->canvasWidget()->setPartEntry( pe );
}

void KWView::tableInsertRow()
{
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    KWInsertDia dia( this, "", table, m_doc, KWInsertDia::ROW, m_gui->canvasWidget() );
    dia.setCaption( i18n( "Insert Row" ) );
    dia.show();
}

void KWView::tableInsertCol()
{
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    // value = 62 because a insert column = 60 +2 (border )see kwtableframeset.cc
    if ( table->boundingRect().right() + 62 > static_cast<int>( m_doc->ptPaperWidth() ) )
    {
        KMessageBox::sorry( this,
                            i18n( "There is not enough space at the right of the table\n"
                                  "to insert a new column." ),
                            i18n( "Insert Column" ) );
    }
    else
    {
        KWInsertDia dia( this, "", table, m_doc, KWInsertDia::COL, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Insert Column" ) );
        dia.show();
    }
}

void KWView::tableDeleteRow()
{
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    if ( table->getRows() == 1 )
    {
        int result;
        result = KMessageBox::warningContinueCancel(this,
                                                    i18n("The table has only one row.\n"
                                                         "Deleting this row will delete the table.\n\n"
                                                         "Do you want to delete the table?"),
                                                    i18n("Delete Row"),
                                                    i18n("&Delete"));
        if (result == KMessageBox::Continue)
        {
            m_doc->deleteTable( table );
            m_gui->canvasWidget()->emitFrameSelectedChanged();
        }
    }
    else
    {
        KWDeleteDia dia( this, "", table, m_doc, KWDeleteDia::ROW, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Delete Row" ) );
        dia.show();
    }

}

void KWView::tableDeleteCol()
{
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    if ( table->getCols() == 1 )
    {
        int result;
        result = KMessageBox::warningContinueCancel(this,
                                                    i18n("The table has only one column.\n"
                                                         "Deleting this column will delete the table.\n\n"
                                                         "Do you want to delete the table?"),
                                                    i18n("Delete Column"),
                                                    i18n("&Delete"));
        if (result == KMessageBox::Continue)
        {
            m_doc->deleteTable( table );
            m_gui->canvasWidget()->emitFrameSelectedChanged();
        }
    }
    else
    {
        KWDeleteDia dia( this, "", table, m_doc, KWDeleteDia::COL, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Delete Column" ) );
        dia.show();
    }
}

void KWView::tableJoinCells()
{
    //m_gui->canvasWidget()->setMouseMode( MM_EDIT_FRAME );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    if ( !table->joinCells() )
    {
        KMessageBox::sorry( this,
                            i18n( "You have to select some cells which are next to each other\n"
                                  "and are not already joined." ),
                            i18n( "Join Cells" ) );
        return;
    }
    m_doc->layout();
    //KoRect r = m_doc->zoomRect( table->boundingRect() );
    //m_gui->canvasWidget()->repaintScreen( r, TRUE );
    m_gui->canvasWidget()->repaintAll();
    m_gui->canvasWidget()->emitFrameSelectedChanged();
}

void KWView::tableSplitCells() {
    KWSplitCellDia *splitDia=new KWSplitCellDia( this,"split cell" ); // TODO remember last settings..
    if(splitDia->exec()) {
        tableSplitCells(splitDia->cols(), splitDia->rows());
    }
    delete splitDia;
}

void KWView::tableSplitCells(int cols, int rows)
{
    //m_gui->canvasWidget()->setMouseMode( MM_EDIT_FRAME );

    QList <KWFrame> selectedFrames = m_doc->getSelectedFrames();
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    if ( !table && selectedFrames.count() > 0) {
        table=selectedFrames.at(0)->getFrameSet()->getGroupManager();
    }

    if(selectedFrames.count() >1 || table == 0) {
        KMessageBox::sorry( this,
                            i18n( "You have to put the cursor into a table\n"
                                  "before splitting cells." ),
                            i18n( "Split Cells" ) );
        return;
    }

    //int rows=1, cols=2;
    if ( !table->splitCell(rows,cols) ) {
        KMessageBox::sorry( this,
                            i18n("There is not enough space to split the cell into that many parts, make it bigger first"),
                            i18n("Split Cells") );
    }
    //KoRect r = m_doc->zoomRect( table->boundingRect() );
    //m_gui->canvasWidget()->repaintScreen( r, TRUE );
    m_doc->updateAllFrames();
    m_doc->layout();
    m_gui->canvasWidget()->repaintAll();
}

void KWView::tableUngroupTable()
{
    m_gui->canvasWidget()->setMouseMode( MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);

    // Use a macro command because we may have to make the table non-floating first
    KMacroCommand * macroCmd = new KMacroCommand( i18n( "Ungroup Table" ) );

    if ( table->isFloating() )
    {
        KWFrameSetFloatingCommand *cmd = new KWFrameSetFloatingCommand( QString::null, table, false ) ;
        macroCmd->addCommand(cmd);
    }

    KWUngroupTableCommand *cmd = new KWUngroupTableCommand( QString::null, table );
    macroCmd->addCommand( cmd );
    m_doc->addCommand( macroCmd );
    macroCmd->execute(); // do it all
}

void KWView::tableDelete()
{
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    ASSERT(table);
    m_doc->deleteTable( table );
    m_gui->canvasWidget()->emitFrameSelectedChanged();
}

void KWView::textStyleSelected( int index )
{
    if(m_gui->canvasWidget()->currentFrameSetEdit())
    {
        KWTextFrameSetEdit * edit = dynamic_cast<KWTextFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit()->currentTextEdit());
        if ( edit )
            edit->applyStyle( m_doc->styleAt( index ) );
        m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
    }
}

void KWView::textSizeSelected( int size )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setPointSize( size );
    m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
}

void KWView::textFontSelected( const QString & font )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setFamily( font );
    m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
}

void KWView::textBold()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setBold(actionFormatBold->isChecked());
}

void KWView::textItalic()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setItalic(actionFormatItalic->isChecked());

}

void KWView::textUnderline()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setUnderline(actionFormatUnderline->isChecked());
}

void KWView::textStrikeOut()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setStrikeOut(actionFormatStrikeOut->isChecked());
}

void KWView::textColor()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        QColor color = edit->textColor();
        if ( KColorDialog::getColor( color ) ) {
            actionFormatColor->setColor( color );
            edit->setTextColor( color );
        }
    }
}

void KWView::textAlignLeft()
{
    if ( actionFormatAlignLeft->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
            edit->setAlign(Qt::AlignLeft);
    } else
        actionFormatAlignLeft->setChecked( true );
}

void KWView::textAlignCenter()
{
    if ( actionFormatAlignCenter->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
            edit->setAlign(Qt::AlignCenter);
    } else
        actionFormatAlignCenter->setChecked( true );
}

void KWView::textAlignRight()
{
    if ( actionFormatAlignRight->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        edit->setAlign(Qt::AlignRight);
    } else
        actionFormatAlignRight->setChecked( true );
}

void KWView::textAlignBlock()
{
    if ( actionFormatAlignBlock->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
            edit->setAlign(Qt3::AlignJustify);
    } else
        actionFormatAlignBlock->setChecked( true );
}

void KWView::textList()
{
    Counter c;
    if ( actionFormatList->isChecked() )
    {
        c.setNumbering( Counter::NUM_LIST );
        c.setStyle( Counter::STYLE_NUM );
    }
    else
    {
        c.setNumbering( Counter::NUM_NONE );
    }
    KWTextFrameSetEdit * edit = currentTextEdit();
    ASSERT(edit);
    if ( edit )
        edit->setCounter( c );
}

void KWView::textSuperScript()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setTextSuperScript(actionFormatSuper->isChecked());
}

void KWView::textSubScript()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setTextSubScript(actionFormatSub->isChecked());
}

void KWView::changeCaseOfText()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if(!edit)
        return;
    KWChangeCaseDia *caseDia=new KWChangeCaseDia( this,"change case" );
    if(caseDia->exec())
    {
        edit->changeCaseOfText(caseDia->getTypeOfCase());
    }
    delete caseDia;
}

void KWView::editPersonalExpr()
{
   KWEditPersonnalExpression *personalDia=new KWEditPersonnalExpression( this,"personnal" );
   if(personalDia->exec())
       m_doc->refreshMenuExpression();
   delete personalDia;
}


void KWView::textIncreaseIndent()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        double leftMargin = edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
        double indent = m_doc->getIndentValue();
        double newVal = leftMargin + indent;
        // Test commented out. This breaks with the DTP case... The user can put
        // a frame anywhere, even closer to the edges than left/right border allows (DF).
        //if( newVal <= (m_doc->ptPaperWidth()-m_doc->ptRightBorder()-m_doc->ptLeftBorder()))
        {
            edit->setMargin( QStyleSheetItem::MarginLeft, newVal );
        }
    }
}

void KWView::textDecreaseIndent()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        double leftMargin = edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
        if ( leftMargin > 0 )
        {
            double indent = m_doc->getIndentValue();
            double newVal = leftMargin - indent;
            edit->setMargin( QStyleSheetItem::MarginLeft, QMAX( newVal, 0 ) );
        }
    }
}


void KWView::textDefaultFormat()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setDefaultFormat();
}


void KWView::borderOutline()
{
    bool b = actionBorderOutline->isChecked();

    actionBorderLeft->setChecked(b);
    actionBorderRight->setChecked(b);
    actionBorderTop->setChecked(b);
    actionBorderBottom->setChecked(b);

    borderSet();
}

void KWView::borderLeft()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderRight()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderTop()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderBottom()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderColor()
{
    m_border.common.color = actionBorderColor->color();
    m_border.left.color = m_border.common.color;
    m_border.right.color = m_border.common.color;
    m_border.top.color = m_border.common.color;
    m_border.bottom.color = m_border.common.color;
    borderSet();
}

void KWView::borderWidth( const QString &width )
{
    m_border.common.ptWidth = width.toInt();
    m_border.left.ptWidth = m_border.common.ptWidth;
    m_border.right.ptWidth = m_border.common.ptWidth;
    m_border.top.ptWidth = m_border.common.ptWidth;
    m_border.bottom.ptWidth = m_border.common.ptWidth;
    borderSet();
    m_gui->canvasWidget()->setFocus();
}

void KWView::borderStyle( const QString &style )
{
    m_border.common.style = Border::getStyle( style );
    m_border.left.style = m_border.common.style;
    m_border.right.style = m_border.common.style;
    m_border.top.style = m_border.common.style;
    m_border.bottom.style = m_border.common.style;
    borderSet();
    m_gui->canvasWidget()->setFocus();
}

void KWView::backgroundColor()
{
    // This action is disabled when no frame is selected.
    // So here we know that a frame is selected.
    backColor = actionBackgroundColor->color();
    if ( m_gui )
        m_gui->canvasWidget()->setFrameBackgroundColor( backColor );
}

void KWView::borderSet()
{
    // The effect of this action depends on if we are in Edit Text or Edit Frame mode.

    m_border.left = m_border.common;
    m_border.right = m_border.common;
    m_border.top = m_border.common;
    m_border.bottom = m_border.common;
    if ( !actionBorderLeft->isChecked() )
    {
        m_border.left.ptWidth = 0;
    }
    if ( !actionBorderRight->isChecked() )
    {
        m_border.right.ptWidth = 0;
    }
    if ( !actionBorderTop->isChecked() )
    {
        m_border.top.ptWidth = 0;
    }
    if ( !actionBorderBottom->isChecked() )
    {
        m_border.bottom.ptWidth = 0;
    }
    KWTextFrameSetEdit *edit = currentTextEdit();
    if ( edit )
    {
        edit->setBorders( m_border.left, m_border.right, m_border.top, m_border.bottom );
    }
    else
    {
        if ( (actionBorderLeft->isChecked() && actionBorderRight->isChecked()
              && actionBorderBottom->isChecked() && actionBorderTop->isChecked())
             || (!actionBorderLeft->isChecked() && !actionBorderRight->isChecked()
                 && !actionBorderBottom->isChecked() && !actionBorderTop->isChecked()))
        {
            m_gui->canvasWidget()->setOutlineFrameBorder( m_border.common, actionBorderLeft->isChecked() );
        }
        else
        {
            m_gui->canvasWidget()->setLeftFrameBorder( m_border.common, actionBorderLeft->isChecked() );
            m_gui->canvasWidget()->setRightFrameBorder( m_border.common, actionBorderRight->isChecked() );
            m_gui->canvasWidget()->setTopFrameBorder( m_border.common, actionBorderTop->isChecked() );
            m_gui->canvasWidget()->setBottomFrameBorder( m_border.common, actionBorderBottom->isChecked() );
        }
    }
}

void KWView::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( m_gui ) m_gui->resize( width(), height() );
}

void KWView::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    if ( ev->activated() )
    {
        initGui();
    }
    KoView::guiActivateEvent( ev );
}

void KWView::borderShowValues()
{
    actionBorderWidth->setCurrentItem( (int)m_border.common.ptWidth - 1 );
    actionBorderStyle->setCurrentItem( (int)m_border.common.style );
}

void KWView::tabListChanged( const KoTabulatorList & tabList )
{
    if(!m_doc->isReadWrite())
        return;
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (!edit)
        return;
    edit->setTabList( tabList );
}

void KWView::newPageLayout( KoPageLayout _layout )
{
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPageLayout( pgLayout, cl, hf );

    pageLayout tmpOldLayout;
    tmpOldLayout._pgLayout=pgLayout;
    tmpOldLayout._cl=cl;
    tmpOldLayout._hf=hf;

    m_doc->setPageLayout( _layout, cl, hf );

    pageLayout tmpNewLayout;
    tmpNewLayout._pgLayout=_layout;
    tmpNewLayout._cl=cl;
    tmpNewLayout._hf=hf;

    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
        edit->textFrameSet()->clearUndoRedoInfo();
    KWPageLayoutCommand *cmd = new KWPageLayoutCommand( i18n("Change Layout"),m_doc,tmpOldLayout,tmpNewLayout ) ;
    m_doc->addCommand(cmd);

    m_doc->updateRuler();

    m_doc->updateResizeHandles();
}

void KWView::newFirstIndent( double _firstIndent )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (!edit) return;
    double val = _firstIndent - edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
    edit->setMargin( QStyleSheetItem::MarginFirstLine, val );
}

void KWView::newLeftIndent( double _leftIndent)
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (edit)
        edit->setMargin( QStyleSheetItem::MarginLeft, _leftIndent );
}

void KWView::openPopupMenuEditText( const QPoint & _point )
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    QString menuName;
    if (edit)
        menuName=edit->getPopupName();
    if(!menuName.isEmpty())
    {
        QPopupMenu * popup = ((QPopupMenu*)factory()->container(menuName,this));
        ASSERT(popup);
        if (popup)
        {
            KWTextFrameSetEdit * textedit = dynamic_cast<KWTextFrameSetEdit *>(edit);
            if (textedit)
            {
                // Removed previous stuff
                unplugActionList( "datatools" );
                m_actionList.clear();
                m_actionList = textedit->dataToolActionList();
                kdDebug() << "KWView::openPopupMenuEditText plugging actionlist with " << m_actionList.count() << " actions" << endl;
                plugActionList( "datatools", m_actionList );
                popup->popup(_point); // using exec() here breaks the spellcheck tool (event loop pb)
            } else
                popup->popup(_point);
        }
    }
}

void KWView::openPopupMenuChangeAction( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() )
        return;
    ((QPopupMenu*)factory()->container("action_popup",this))->popup(_point);
}

void KWView::updatePopupMenuChangeAction()
{
    KWFrame *frame=m_doc->getFirstSelectedFrame();
    // Warning, frame can be 0L !

    // if a header/footer etc. Dont show the popup.
    if(frame && frame->getFrameSet() && frame->getFrameSet()->getFrameInfo() != FI_BODY)
        return;
    int nbFrame=m_doc->getSelectedFrames().count();
    // enable delete
    actionEditDelFrame->setEnabled(true && nbFrame==1);

    // if text frame,
    if(frame && frame->getFrameSet() && frame->getFrameSet()->getFrameType() == FT_TEXT)
        {
            // if frameset 0 disable delete
            if(m_doc->processingType()  == KWDocument::WP && frame->getFrameSet() == m_doc->getFrameSet(0))
                {
                    actionEditDelFrame->setEnabled(false);
                }
        }
}

void KWView::openPopupMenuEditFrame( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() )
        return;
    updatePopupMenuChangeAction();
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    if(!table)
        ((QPopupMenu*)factory()->container("frame_popup",this))->popup(_point);
    else
        ((QPopupMenu*)factory()->container("frame_popup_table",this))->popup(_point);
}

void KWView::startKSpell()
{
    // m_spellCurrFrameSetNum is supposed to be set by the caller of this method
    m_kspell = new KSpell( this, i18n( "Spell Checking" ), this, SLOT( spellCheckerReady() ), m_doc->getKSpellConfig() );

    QObject::connect( m_kspell, SIGNAL( death() ),
                      this, SLOT( spellCheckerFinished() ) );
    QObject::connect( m_kspell, SIGNAL( misspelling( QString, QStringList*, unsigned ) ),
                      this, SLOT( spellCheckerMisspelling( QString, QStringList*, unsigned ) ) );
    QObject::connect( m_kspell, SIGNAL( corrected( QString, QString, unsigned ) ),
                      this, SLOT( spellCheckerCorrected( QString, QString, unsigned ) ) );
    QObject::connect( m_kspell, SIGNAL( done( const QString & ) ),
                      this, SLOT( spellCheckerDone( const QString & ) ) );
}

void KWView::spellCheckerReady()
{
    for ( unsigned int i = m_spellCurrFrameSetNum + 1; i < m_doc->getNumFrameSets(); i++ ) {
        KWFrameSet *frameset = m_doc->getFrameSet( i );
        if ( !frameset->isVisible() || frameset->getFrameType() != FT_TEXT )
            continue;
        m_spellCurrFrameSetNum = i; // store as number, not as pointer, to implement "go to next frameset" when done
        //kdDebug() << "KWView::spellCheckerReady spell-checking frameset " << m_spellCurrFrameSetNum << endl;
        KWTextFrameSet * textfs = dynamic_cast<KWTextFrameSet*>( frameset );
        QTextParag * p = textfs->textDocument()->firstParag();
        QString text;
        while ( p ) {
            QString str = p->string()->toString();
            str.truncate( str.length() - 1 ); // damn trailing space
            text += str + '\n';
            p = p->next();
        }
        text += '\n';
        m_kspell->check( text );
        return;
    }
    //kdDebug() << "KWView::spellCheckerReady done" << endl;

    // Done
    m_kspell->cleanUp();
    delete m_kspell;
    m_kspell = 0;
}

void KWView::spellCheckerMisspelling( QString old, QStringList* , unsigned pos )
{
    //kdDebug() << "KWView::spellCheckerMisspelling old=" << old << " pos=" << pos << endl;
    KWTextFrameSet * fs = dynamic_cast<KWTextFrameSet *>( m_doc->getFrameSet( m_spellCurrFrameSetNum ) );
    ASSERT( fs );
    if ( !fs ) return;
    QTextParag * p = fs->textDocument()->firstParag();
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    ASSERT( p );
    if ( !p ) return;
    //kdDebug() << "KWView::spellCheckerMisspelling p=" << p->paragId() << " pos=" << pos << " length=" << old.length() << endl;
    fs->highlightPortion( p, pos, old.length(), m_gui->canvasWidget() );
}

void KWView::spellCheckerCorrected( QString old, QString corr, unsigned pos )
{
    //kdDebug() << "KWView::spellCheckerCorrected old=" << old << " corr=" << corr << " pos=" << pos << endl;

    KWTextFrameSet * fs = dynamic_cast<KWTextFrameSet *>( m_doc->getFrameSet( m_spellCurrFrameSetNum ) );
    ASSERT( fs );
    if ( !fs ) return;
    QTextParag * p = fs->textDocument()->firstParag();
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    ASSERT( p );
    if ( !p ) return;
    fs->highlightPortion( p, pos, old.length(), m_gui->canvasWidget() );

    QTextCursor cursor( fs->textDocument() );
    cursor.setParag( p );
    cursor.setIndex( pos );
    fs->replaceSelection( &cursor, corr, KWTextFrameSet::HighlightSelection,
                          i18n("Correct misspelled word") );
}

void KWView::spellCheckerDone( const QString & )
{
    KWTextFrameSet * fs = dynamic_cast<KWTextFrameSet *>( m_doc->getFrameSet( m_spellCurrFrameSetNum ) );
    ASSERT( fs );
    if ( fs )
        fs->removeHighlight();

    int result = m_kspell->dlgResult();
    m_kspell->cleanUp();
    delete m_kspell;
    m_kspell = 0;

    if ( result != KS_CANCEL && result != KS_STOP )
    {
        // Try to check another frameset
        startKSpell();
    }
}

void KWView::spellCheckerFinished()
{
    KSpell::spellStatus status = m_kspell->status();
    delete m_kspell;
    m_kspell = 0;
    if (status == KSpell::Error)
    {
        KMessageBox::sorry(this, i18n("ISpell could not be started.\n"
                                      "Please make sure you have ISpell properly configured and in your PATH.\nUsed settings->configure."));
    }
    else if (status == KSpell::Crashed)
    {
        KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
    }
    KWTextFrameSet * fs = dynamic_cast<KWTextFrameSet *>( m_doc->getFrameSet( m_spellCurrFrameSetNum ) );
    ASSERT( fs );
    if ( fs )
        fs->removeHighlight();

    KWTextFrameSetEdit * edit = currentTextEdit();
    if (edit)
        edit->drawCursor( TRUE );
}

void KWView::configure()
{
    KWConfig configDia( this );
    configDia.show();
}

KWTextFrameSetEdit *KWView::currentTextEdit()
{
    if (!m_gui)
        return 0L;
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        return dynamic_cast<KWTextFrameSetEdit *>(edit->currentTextEdit());
    return 0L;
}

void KWView::slotFrameSetEditChanged()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    bool rw = koDocument()->isReadWrite();
    bool hasSelection = edit && edit->textFrameSet()->hasSelection();
    actionEditCut->setEnabled( hasSelection && rw );
    actionEditCopy->setEnabled( hasSelection );
    clipboardDataChanged(); // for paste

    bool state = (edit != 0L) && rw;
    actionEditSelectAll->setEnabled(state);
    actionFormatFont->setEnabled(state);
    actionFormatFontSize->setEnabled(state);
    actionFormatFontFamily->setEnabled(state);
    actionFormatStyle->setEnabled(state);
    actionFormatBold->setEnabled(state);
    actionFormatItalic->setEnabled(state);
    actionFormatUnderline->setEnabled(state);
    actionFormatStrikeOut->setEnabled(state);
    actionFormatColor->setEnabled(state);
    actionFormatAlignLeft->setEnabled(state);
    actionFormatAlignCenter->setEnabled(state);
    actionFormatAlignRight->setEnabled(state);
    actionFormatAlignBlock->setEnabled(state);
    actionFormatIncreaseIndent->setEnabled(state);
    actionChangeCase->setEnabled(state);


    bool goodleftMargin=false;
    if(state)
        goodleftMargin=(edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft]>0);

    actionFormatDecreaseIndent->setEnabled(goodleftMargin && state);

    actionFormatList->setEnabled(state);
    actionFormatSuper->setEnabled(state);
    actionFormatSub->setEnabled(state);
    actionFormatParag->setEnabled(state);
    actionInsertSpecialChar->setEnabled(state);
    actionInsertPicture->setEnabled(state);
    actionInsertFormula->setEnabled(state);
    actionInsertContents->setEnabled(state);
    actionInsertVariable->setEnabled(state);
    actionInsertExpression->setEnabled(state);
    actionInsertFrameBreak->setEnabled(state);

    // Set the "frame start" in the ruler (tabs are relative to that position)
    if ( edit )
    {
        QPoint p = m_doc->zoomPoint( edit->currentFrame()->topLeft() );
        p = m_gui->canvasWidget()->viewMode()->normalToView( p );
        m_gui->getHorzRuler()->setFrameStart( p.x() );
    }

}

void KWView::frameSelectedChanged()
{
    int nbFrame=m_doc->getSelectedFrames().count();
    actionFormatFrameSet->setEnabled( !currentTextEdit() && (nbFrame>=1));
    if ( !currentTextEdit() && (nbFrame==1) )
    {
        KWFrameSet * fs = m_doc->getFirstSelectedFrame()->getFrameSet();
        actionEditDelFrame->setEnabled( !fs->isAHeader() && !fs->isAFooter() );
    } else
        actionEditDelFrame->setEnabled( false );

    actionBackgroundColor->setEnabled( nbFrame >= 1 );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    actionTableJoinCells->setEnabled( table && (nbFrame>1));

    bool state=(table && nbFrame==1);

    actionTableSplitCells->setEnabled( state );

    state=(table && nbFrame>0);

    actionTableInsertRow->setEnabled( state);
    actionTableInsertCol->setEnabled( state);
    actionTableDelRow->setEnabled( state );
    actionTableDelCol->setEnabled( state );
    actionTableDelete->setEnabled( state );
    actionTableUngroup->setEnabled( state );

    m_doc->refreshFrameBorderButton();

    updateFrameStatusBarItem();
}

void KWView::docStructChanged(int _type)
{
    KWDocStruct *m_pDocStruct=m_gui->getDocStruct();
    if(m_pDocStruct)
        m_pDocStruct->getDocStructTree()->refreshTree(_type);
}


bool KWView::doubleClickActivation() const
{
    return TRUE;
}

QWidget* KWView::canvas()
{
    return m_gui->canvasWidget()->viewport();
}

int KWView::canvasXOffset() const
{
    return m_gui->canvasWidget()->contentsX();
}

int KWView::canvasYOffset() const
{
    return m_gui->canvasWidget()->contentsY();
}

void KWView::canvasAddChild( KoViewChild *child )
{
    m_gui->canvasWidget()->addChild( child->frame() );
}

/******************************************************************/
/* Class: KWLayoutWidget                                          */
/******************************************************************/

KWLayoutWidget::KWLayoutWidget( QWidget *parent, KWGUI *g )
    : QWidget( parent )
{
    gui = g;
}

void KWLayoutWidget::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    gui->reorganize();
}

/******************************************************************/
/* Class: KWGUI                                                */
/******************************************************************/
KWGUI::KWGUI( QWidget *parent, KWView *_view )
    : QWidget( parent, "" )
{
    view = _view;

    r_horz = r_vert = 0;
    KWDocument * doc = view->kWordDocument();

    panner = new QSplitter( Qt::Horizontal, this );
    docStruct = new KWDocStruct( panner, doc, this );
    docStruct->setMinimumWidth( 0 );
    left = new KWLayoutWidget( panner, this );
    left->show();
    canvas = new KWCanvas( left, doc, this );

    QValueList<int> l;
    l << 0;
    panner->setSizes( l );

    KoPageLayout layout = doc->pageLayout();

    tabChooser = new KoTabChooser( left, KoTabChooser::TAB_ALL );
    tabChooser->setReadWrite(doc->isReadWrite());

    r_horz = new KoRuler( left, canvas->viewport(), Qt::Horizontal, layout,
                          KoRuler::F_INDENTS | KoRuler::F_TABS, tabChooser );
    r_horz->setReadWrite(doc->isReadWrite());
    r_vert = new KoRuler( left, canvas->viewport(), Qt::Vertical, layout, 0 );
    connect( r_horz, SIGNAL( newPageLayout( KoPageLayout ) ), view, SLOT( newPageLayout( KoPageLayout ) ) );
    r_vert->setReadWrite(doc->isReadWrite());

    r_horz->setZoom( doc->zoomedResolutionX() );
    r_vert->setZoom( doc->zoomedResolutionY() );

    connect( r_horz, SIGNAL( newLeftIndent( double ) ), view, SLOT( newLeftIndent( double ) ) );
    connect( r_horz, SIGNAL( newFirstIndent( double ) ), view, SLOT( newFirstIndent( double ) ) );

    connect( r_horz, SIGNAL( openPageLayoutDia() ), view, SLOT( formatPage() ) );
    connect( r_horz, SIGNAL( unitChanged( QString ) ), this, SLOT( unitChanged( QString ) ) );
    connect( r_vert, SIGNAL( newPageLayout( KoPageLayout ) ), view, SLOT( newPageLayout( KoPageLayout ) ) );
    connect( r_vert, SIGNAL( openPageLayoutDia() ), view, SLOT( formatPage() ) );
    connect( r_vert, SIGNAL( unitChanged( QString ) ), this, SLOT( unitChanged( QString ) ) );

    r_horz->setUnit( doc->getUnitName() );
    r_vert->setUnit( doc->getUnitName() );

    r_horz->hide();
    r_vert->hide();

    canvas->show();
    docStruct->show();

    reorganize();

#if 0
    if ( doc->processingType() == KWDocument::DTP )   // ???
        canvas->setRuler2Frame( 0, 0 );
#endif

    connect( r_horz, SIGNAL( tabListChanged( const KoTabulatorList & ) ), view,
             SLOT( tabListChanged( const KoTabulatorList & ) ) );

    setKeyCompression( TRUE );
    setAcceptDrops( TRUE );
    setFocusPolicy( QWidget::NoFocus );

    canvas->setContentsPos( 0, 0 );
}

void KWGUI::showGUI()
{
    reorganize();
}

void KWGUI::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    reorganize();
}

void KWGUI::reorganize()
{
    int space=20;
    if(view->kWordDocument()->getShowRuler())
    {
        r_vert->show();
        r_horz->show();
        tabChooser->show();
        tabChooser->setGeometry( 0, 0, 20, 20 );
    }
    else
    {
        r_vert->hide();
        r_horz->hide();
        tabChooser->hide();
        space=0;
    }

    panner->setGeometry( 0, 0, width(), height() );
    canvas->setGeometry( space, space, left->width() - space, left->height() - space );
    r_horz->setGeometry( space, 0, left->width() - space, space );
    r_vert->setGeometry( 0, space, space, left->height() - space );
}

void KWGUI::unitChanged( QString u )
{
    view->kWordDocument()->setUnit( KWUnit::unit( u ) );
}

#include "kwview.moc"
