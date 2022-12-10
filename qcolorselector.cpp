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
#include "qcolorselector.h"

QColorSelector::QColorSelector (QWidget *parent) : QFrame {parent}
{

}

void QColorSelector::setDefaultColor (const QColor &c)
{
    color = c;
    emit changed ();
    update ();
}

QColor QColorSelector::getColor ()
{
    return color;
}


void QColorSelector::paintEvent (QPaintEvent *)
{
    QPainter p {this};
    p.setBrush (color);
    p.fillRect (0, 0, width (), height (), color);
}

void QColorSelector::mousePressEvent (QMouseEvent *)
{
    QColor old = color;
    color = QColorDialog::getColor (old);
    if (color != old)
        emit changed ();
    update ();
}
