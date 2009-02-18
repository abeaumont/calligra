/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2005-2009 Jarosław Staniek <staniek@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KFORMEDITOR_COMMANDS_H
#define KFORMEDITOR_COMMANDS_H

#include <QHash>
#include <QVariant>
#include <qdom.h>
#include <QString>

#include <k3command.h>
#include "utils.h"
#include "form.h"

class QRect;
class QPoint;
class QStringList;

namespace KFormDesigner
{

//removed 2.0 class WidgetPropertySet;
class ObjectTreeItem;
class Container;
class Form;

//! Base class for KFormDesigner's commands
class KFORMEDITOR_EXPORT Command : public K3Command
{
public:
    Command();
    
    virtual ~Command();

    virtual void debug() = 0;
};

//! Command is used when changing a property for one or more widgets. 
class KFORMEDITOR_EXPORT PropertyCommand : public Command
{
public:
   /*! @a oldValue is the old property value for selected widget. 
     This enables reverting the change. @a value is the new property value. */
    PropertyCommand(Form& form, const QByteArray &wname, const QVariant &oldValue,
                    const QVariant &value, const QByteArray &propertyName);

   /*! @a oldValues is a QHash of the old property values for every widget, 
     to allow reverting the change. @a value is the new property value.
     You can use the simpler constructor for a single widget. */
    PropertyCommand(Form& form, const QHash<QByteArray, QVariant> &oldValues,
                    const QVariant &value, const QByteArray &propertyName);

    virtual ~PropertyCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;

    QByteArray property() const;

    QVariant value() const;

    void setValue(const QVariant &value);
    
    const QHash<QByteArray, QVariant>& oldValues() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

/*! Command used when moving multiples widgets at the same time, while holding Ctrl or Shift.
/*! You need to supply a list of widget names, and the position of the cursor before moving. Use setPos()
  to tell the new cursor pos every time it changes.*/
class KFORMEDITOR_EXPORT GeometryPropertyCommand : public Command
{
public:
    GeometryPropertyCommand(Form& form, const QStringList &names, const QPoint& oldPos);

    virtual ~GeometryPropertyCommand();

    virtual void execute();
    virtual void unexecute();
    virtual QString name() const;
    void setPos(const QPoint& pos);
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

//! Command used when an "Align Widgets position" action is activated.
/* You just need to give the list of widget names (the selected ones), and the
  type of alignment (see the enum for possible values). */
class KFORMEDITOR_EXPORT AlignWidgetsCommand : public Command
{
public:
    AlignWidgetsCommand(Form &form, Form::WidgetAlignment alignment, const QWidgetList &list);

    virtual ~AlignWidgetsCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

//! Command used when an "Adjust Widgets Size" action is activated.
/*! You just need to give the list of widget names (the selected ones), 
    and the type of size modification (see the enum for possible values). */
class KFORMEDITOR_EXPORT AdjustSizeCommand : public Command
{
public:
    enum Adjustment {
        SizeToGrid,
        SizeToFit,
        SizeToSmallWidth,
        SizeToBigWidth,
        SizeToSmallHeight,
        SizeToBigHeight
    };

    AdjustSizeCommand(Form& form, Adjustment type, const QWidgetList &list);

    virtual ~AdjustSizeCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    QSize getSizeFromChildren(ObjectTreeItem *item);

protected:
    class Private;
    Private * const d;
};

//! Command used when switching the layout of a container.
/*! It remembers the old pos of every widget inside the container. */
class KFORMEDITOR_EXPORT LayoutPropertyCommand : public PropertyCommand
{
public:
    LayoutPropertyCommand(Form& form, const QByteArray &wname,
                          const QVariant &oldValue, const QVariant &value);

    virtual ~LayoutPropertyCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

//! Command used when inserting a widget using toolbar or menu. 
/*! You only have to give the parent Container and the widget pos.
 The other information is taken from the form. */
class KFORMEDITOR_EXPORT InsertWidgetCommand : public Command
{
public:
    InsertWidgetCommand(const Container& container);

    virtual ~InsertWidgetCommand();

    /*! This ctor allows to set explicit class name and position.
     Used for dropping widgets on the form surface.
     If \a namePrefix is empty, widget's unique name is constructed using
     hint for \a className (WidgetLibrary::namePrefix()),
     otherwise, \a namePrefix is used to generate widget's name.
     This allows e.g. inserting a widgets having name constructed using
     */
    InsertWidgetCommand(const Container& container, const QByteArray& className,
                        const QPoint& pos, const QByteArray& namePrefix = QByteArray());

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

    //! \return inserted widget's name
    QByteArray widgetName() const;

protected:
    class Private;
    Private * const d;
};

//! Command used when creating a layout from some widgets using "Lay out in...".
/*! It remembers the old pos of every widget, and takes care of updating ObjectTree too. You need
 to supply a QWidgetList of the selected widgets. */
class KFORMEDITOR_EXPORT CreateLayoutCommand : public Command
{
public:
    CreateLayoutCommand(Form &form, Form::LayoutType layoutType, const QWidgetList &list);

    virtual ~CreateLayoutCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    //! Used in BreakLayoutCommand ctor.
    CreateLayoutCommand();

    class Private;
    Private * const d;
};

//! Command used when the 'Break Layout' menu item is selected. 
/*! It does exactly the opposite of the CreateLayoutCommand. */
class KFORMEDITOR_EXPORT BreakLayoutCommand : public CreateLayoutCommand
{
public:
    BreakLayoutCommand(const Container &container);

    virtual ~BreakLayoutCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();
};

//! @todo add CopyWidgetCommand

//! Command used when pasting widgets. 
/*! You need to give the QDomDocument containing
    the widget(s) to paste, and optionally the point where to paste widgets. */
class KFORMEDITOR_EXPORT PasteWidgetCommand : public Command
{
public:
    PasteWidgetCommand(const QDomDocument &domDoc, const Container& container, const QPoint& p = QPoint());

    virtual ~PasteWidgetCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    /*! Internal function used to change the coordinates of a widget to \a newPos
     before pasting it (to paste it at the position of the contextual menu). It modifies
       the "geometry" property of the QDomElement representing the widget. */
    void changePos(QDomElement &el, const QPoint &newPos);

    /*! Internal function used to fix the coordinates of a widget before pasting it
       (to avoid having two widgets at the same position). It moves the widget by
       (10, 10) increment (several times if there are already pasted widgets at this position). */
    void fixPos(QDomElement &el, Container *container);
    
    void moveWidgetBy(QDomElement &el, Container *container, const QPoint &p);
    
    /*! Internal function used to fix the names of the widgets before pasting them.
      It prevents from pasting a widget with
      the same name as an actual widget. The child widgets are also fixed recursively.\n
      If the name of the widget ends with a number (eg "QLineEdit1"), the new name is
      just incremented by one (eg becomes "QLineEdit2"). Otherwise, a "2" is just
      appended at the end of the name (eg "myWidget" becomes "myWidget2"). */
    void fixNames(QDomElement &el);

protected:
    class Private;
    Private * const d;
};

//! Command used when deleting widgets using the "Delete" menu item.
/*! You need to give a QWidgetList of the selected widgets. */
class KFORMEDITOR_EXPORT DeleteWidgetCommand : public Command
{
public:
    DeleteWidgetCommand(Form& form, const QWidgetList &list);

    virtual ~DeleteWidgetCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

//! Command used when duplicating widgets.
/*! You need to give a QWidgetList of the selected widgets. */
class KFORMEDITOR_EXPORT DuplicateWidgetCommand : public Command
{
public:
    DuplicateWidgetCommand(const Container& container, const QWidgetList &list, const QPoint& copyToPoint);

    virtual ~DuplicateWidgetCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d;
};

//! Command used when cutting widgets. 
/*! It is basically a DeleteWidgetCommand which also updates the clipboard contents. */
class KFORMEDITOR_EXPORT CutWidgetCommand : public DeleteWidgetCommand
{
public:
    CutWidgetCommand(Form &form, const QWidgetList &list);

    virtual ~CutWidgetCommand();

    virtual void execute();
    
    virtual void unexecute();
    
    virtual QString name() const;
    
    virtual void debug();

protected:
    class Private;
    Private * const d2;
};

//! Command that holds several subcommands.
/*! It appears as one to the user and in the command history,
 but it can use the implementation of multiple commands internally.
 It extends KMacroCommand by providing the list of commands executed.
 Selected subcommands can be marked as nonexecutable by adding them using
 addCommand(KCommand *command, bool allowExecute) special method.
*/
class KFORMEDITOR_EXPORT CommandGroup : public Command
{
public:
    CommandGroup(Form &form, const QString & name);

    virtual ~CommandGroup();

    /*! Like KMacroCommand::addCommand(KCommand*)
     but if \a allowExecute is false, \a command will not be executed
     as a subcommand when CommandGroup::execute() is called.

     This is useful e.g. in KexiFormView::insertAutoFields(),
     where a number of subcommands of InsertWidgetCommand type and subcommands
     is groupped using CommandGroup but some of these subcommands are executed
     before executing CommandGroup::execute().

     If \a allowExecute is true, this method behaves exactly like
     KMacroCommand::addCommand(KCommand*).

     Note that unexecute() doesn't check \a allowExecute flag:
     all subcommands will be unexecuted (in reverse order
     to the one in which they were added).
    */
    void addCommand(K3Command *command, bool allowExecute);

    /*! Executes all subcommands added to this group
     in the same order as they were added. Subcommands added with
     addCommand(KCommand *command, bool allowExecute) where allowExecute == false,
     will not be executed. */
    virtual void execute();

    /*! Unexecutes all subcommands added to this group,
     (in reversed order). */
    virtual void unexecute();

    virtual QString name() const;

    /*! \return a list of all subcommands of this group.
     Note that if a given subcommand is a group itself,
     it will not be expanded to subcommands on this list. */
    const QList< K3Command*> commands() const;

    /*! Resets all 'allowExecute' flags that was set in addCommand().
     Call this after calling CommandGroup::execute() to ensure that
     in the future, when REDO is be executed, all subcommands will be executed. */
    void resetAllowExecuteFlags();

    virtual void debug();

protected:
    class Private;
    Private * const d;
};

}

#endif
