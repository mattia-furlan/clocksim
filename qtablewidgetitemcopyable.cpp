/*
 clocksim - Rubik's Clock Simulator
 Copyright (C) 2016 Mattia Furlan

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program;  If not, see <http://www.gnu.org/licenses/>.
*/
#include "qtablewidgetitemcopyable.h"

QTableWidgetItemCopyable::QTableWidgetItemCopyable (QString text) : QTableWidgetItem {text}
{
    setFlags (Qt::ItemIsEnabled | Qt::ItemIsSelectable); //not editable
}

void QTableWidgetItemCopyable::keyPressEvent (QKeyEvent *event)
{
   if (event->key () == Qt::Key_C && (event->modifiers () & Qt::ControlModifier))
   {
      QApplication::clipboard ()->setText (text ());
   }
}
