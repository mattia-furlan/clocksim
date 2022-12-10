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
#ifndef CLOCKSIM_H
#define CLOCKSIM_H

#include <QMessageBox>
#include <QKeySequence>
#include <QStandardPaths>
#include <QDataStream>
#include <QSettings>
#include <QTableWidget>
#include <QWidget>
#include <QDebug>
#include <QVector>
#include <QPainter>
#include <QKeyEvent>
#include <QtMath>
#include <QTime>
#include <QTimer>
#include <QDateTime>

#include <algorithm>
#include <float.h>

#include "clock.h"
#include "settingsform.h"
#include "qclickablelabel.h"
#include "qtablewidgetitemcopyable.h"
#include "structures.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define STR(a) #a
#define STR2(a) STR (a)
//Format: MAJOR.MINOR
#define VERSION_STRING (QString {STR2 (VERSION_MAJOR)} + QString {"."} + QString {STR2 (VERSION_MINOR)})

namespace Ui
{
    class ClockSim;
}

class PBsViewer;

class ClockSim : public QWidget
{
    Q_OBJECT

public:
    explicit ClockSim (QWidget* = 0);
    ~ClockSim ();

private slots:
    void timerTimeout ();
    void startSolve ();
    void updateStats ();
    void updateInspectionCount ();

private:
    ClockPuzzle clock;

    QTimer *timer;
    QTime beginning;

    int angle {0};

    bool is_scrambled {false};
    bool is_solving {false};
    float movecount;

    QString reconstruction;
    Scramble last_scramble;

    QVector<Solve> solves;

#define times_vector (getQVector<float> (&Solve::time))
#define movecount_vector (getQVector<float> (&Solve::movecount))
#define tps_vector (getQVector<float> (&Solve::tps))
#define scrambles_vector (getQVector<Scramble> (&Solve::scramble))
#define reconstructions_vector (getQVector<QString> (&Solve::reconstruction))

    struct Statistics
    {
        float single {FLT_MAX};
        float avg5 {FLT_MAX};
        float avg12 {FLT_MAX};
        float avg50 {FLT_MAX};
        float avg100 {FLT_MAX};
    } current[3], pb[3];

    PBs pbs[3];

    void paintEvent (QPaintEvent *);
    void keyPressEvent (QKeyEvent *);
    void closeEvent (QCloseEvent *);

    void displayTimes (int);
    /*template <class T>
    QVector<T>& getQVector (T Solve::*, int);*/
    template <class T>
        QVector<T>& getQVector (T Solve::*);
    float getAvg (const QVector<float>&, int);
    float getMean (const QVector<float> &, int);
    QString getTimeString (float);
    QString floatToQString (float);
    void doZRotation (int);
    void doY2Rotation ();

    void savePBs ();
    void readPBs ();
    void saveSession ();
    void readSession ();
    void saveSettings ();
    void readSettings ();

    template <class R, class C, class ...Params, class ...Args>
    typename std::enable_if<std::is_base_of<C,QLabel>::value &&
                            sizeof... (Params) == sizeof... (Args)>::type
    applyToLabels (R (C::*) (Params...), Args&& ...);

    SettingsForm *settings;
    PBsViewer *pbs_viewer;
    Ui::ClockSim *ui;

    friend class PBsViewer;
};


#endif
