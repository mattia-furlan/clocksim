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
#ifndef QCOLORSELECTOR_H
#define QCOLORSELECTOR_H

#include <QFrame>
#include <QColor>
#include <QColorDialog>
#include <QPainter>

class QColorSelector : public QFrame
{
    Q_OBJECT

public:
    QColorSelector (QWidget *parent = 0);
    void setDefaultColor (const QColor& c);
    QColor getColor ();

private:
    QColor color {255, 255, 255};

signals:
    void changed ();

protected:
    void paintEvent (QPaintEvent *);
    void mousePressEvent (QMouseEvent *);
};

#endif // QCOLORSELECTOR_H
