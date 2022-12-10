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
#ifndef QLINEEDITHOTKEY_H
#define QLINEEDITHOTKEY_H

#include <QLineEdit>
#include <QKeySequence>
#include <QKeyEvent>

class QLineEditHotKey: public QLineEdit
{
    Q_OBJECT

public:
    QLineEditHotKey (QWidget* parent = 0);
    void setDefaultKey (const QKeySequence &k);
    QKeySequence getKey ();

signals:
    void changed ();

protected:
    void keyPressEvent (QKeyEvent *event);

private:
    QKeySequence key;
};

#endif // QLINEEDITHOTKEY_H
