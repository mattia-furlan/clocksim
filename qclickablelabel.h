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
#ifndef QCLICKABLELABEL_H
#define QCLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

class QClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit QClickableLabel (QWidget * = 0);
    ~QClickableLabel ();

signals:
    void clicked ();

protected:
    void mousePressEvent (QMouseEvent *);
    void enterEvent (QEvent *);
    void leaveEvent (QEvent *);

private:
    QString style_sheet;
};
#endif // QCLICKABLELABEL_H
