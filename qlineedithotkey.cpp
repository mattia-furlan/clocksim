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
#include "qlineedithotkey.h"

QLineEditHotKey::QLineEditHotKey (QWidget* parent): QLineEdit {parent}
{

}

QKeySequence QLineEditHotKey::getKey ()
{
    return key;
}

void QLineEditHotKey::setDefaultKey (const QKeySequence &k)
{
    key = QKeySequence {k};
    setText (key.toString (QKeySequence::NativeText));
    emit changed ();
}

void QLineEditHotKey::keyPressEvent (QKeyEvent *event)
{
    int keyInt = event->key ();
    Qt::Key keycode = static_cast<Qt::Key> (keyInt);

    if (keycode == Qt::Key_unknown || keycode == Qt::Key_Backspace)
    {
        key = QKeySequence {};
        setText ("");
        emit changed ();
        return;
    }

    Qt::KeyboardModifiers modifiers = event->modifiers ();

    if (modifiers.testFlag (Qt::ShiftModifier))
        keyInt += Qt::SHIFT;
    if (modifiers.testFlag (Qt::ControlModifier))
        keyInt += Qt::CTRL;
    if (modifiers.testFlag (Qt::AltModifier))
        keyInt += Qt::ALT;

    key = QKeySequence {keyInt};
    setText (key.toString (QKeySequence::NativeText));
    emit changed ();

}
