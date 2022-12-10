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
#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QWidget>
#include <QMap>
#include <QColor>
#include <QColorDialog>
#include <QDataStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>

#include "qlineedithotkey.h"

namespace Ui
{
    class SettingsForm;
}

class SettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsForm (QWidget * = 0);
    ~SettingsForm ();

    int getTimerPrecision ();

    friend class ClockSim;

private:
    Ui::SettingsForm *ui;
    QMap<QLineEditHotKey*, int> wheels_controls;
    QMap<QLineEditHotKey*, int> pins_controls;


private slots:
    void setDefaultControls ();
    void setDefaultColors ();

    void saveControls ();
    void loadControls ();
    void saveColors ();
    void loadColors ();

    void checkKeyConflicts ();
};

#endif // SETTINGSFORM_H
