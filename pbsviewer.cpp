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
#include "pbsviewer.h"
#include "ui_pbsviewer.h"

#include "clocksim.h"

#define table ui->tableWidget

PBsViewer::PBsViewer (ClockSim *parent) : ui {new Ui::PBsViewer}, p {parent}
{
    ui->setupUi (this);
    connect (ui->comboBoxPBType, SIGNAL (currentIndexChanged (int)), this, SLOT (displayPBs ()));
    connect (ui->comboBoxPBAvg, SIGNAL (currentIndexChanged (int)), this, SLOT (displayPBs ()));

    layout ()->setMargin (0);
    ui->comboBoxPBAvg->setContentsMargins (0, 0, 0, 0);
    ui->comboBoxPBType->setContentsMargins (0, 0, 0, 0);

    table->setColumnCount (5);
    table->setRowCount (1);
    table->horizontalHeader ()->setStretchLastSection (true);

    table->setHorizontalHeaderLabels (QStringList () << "Time" << "Movecount" << "TPS" << "Scramble" << "When");

    displayPBs ();
}

PBsViewer::~PBsViewer ()
{
    delete ui;
}

template <class T, class X>
QVector<T> getVector (QVector<X> &x, T X::*m)
{
    QVector<T> v;
    foreach (const X& y, x)
        v.push_back (y.*m);
    return v;
}

extern QVector<int> getBestN (const QVector<float>& v, int n);

extern QVector<int> getWorstN (const QVector<float>& v, int n);

void PBsViewer::displayPBs ()
{
    int index = ui->comboBoxPBType->currentIndex ();
    int i = ui->comboBoxPBAvg->currentIndex ();

    const QString datetime_format {"dd/MM/yy - hh:mm"};

    auto makeCenterAlignedItem = [] (const QString& text = "")
    {
        auto *item = new QTableWidgetItemCopyable {text};
        item->setTextAlignment (Qt::AlignVCenter | Qt::AlignHCenter);
        return item;
    };
    auto makeLeftAlignedItem = [] (const QString& text = "")
    {
        auto *item = new QTableWidgetItemCopyable {text};
        item->setTextAlignment (Qt::AlignVCenter | Qt::AlignLeft);
        return item;
    };
    auto makeColorItem = [] (const QString& text = "")
    {
        auto *item = new QTableWidgetItemCopyable {text};
        item->setBackgroundColor ({Qt::gray});
        item->setTextAlignment (Qt::AlignCenter);
        return item;
    };


    if (!i) //single
    {
        const auto& solve = p->pbs[index].single;

        table->setVerticalHeaderItem (0, new QTableWidgetItem ("Single"));
        table->setItem (0, 0, makeCenterAlignedItem (p->getTimeString (solve.time)));
        table->setItem (0, 1, makeCenterAlignedItem (p->floatToQString (solve.movecount)));
        table->setItem (0, 2, makeCenterAlignedItem (p->floatToQString (solve.tps)));
        table->setItem (0, 3, makeLeftAlignedItem (solve.scramble.toQString ()));
        table->setItem (0, 4, makeCenterAlignedItem (solve.datetime.toString (datetime_format)));

    }
    else //averages
    {
        int n = (i == 1 ? 5 : (i == 2 ? 12 : (i == 3 ? 50 : 100)));
        qDebug () << "n = " << n;

        Average avg;
        if (n == 5) avg = p->pbs[index].avg5;
        else if (n == 12) avg = p->pbs[index].avg12;
        else if (n == 50) avg = p->pbs[index].avg50;
        else if (n == 100) avg = p->pbs[index].avg100;

        QVector<float> times = getVector (avg.solves, &Solve::time);
        QVector<float> movecounts = getVector (avg.solves, &Solve::movecount);
        QVector<float> tps_s = getVector (avg.solves, &Solve::tps);

        int m = static_cast<int> (qCeil (n * 0.05));

        QVector<int> bests[3] = {getBestN (times, m), getBestN (movecounts, m), getWorstN (tps_s, m)};
        QVector<int> worsts[3] = {getWorstN (times, m), getWorstN (movecounts, m), getBestN (tps_s, m)};

        table->setRowCount (n + 1);
        table->setVerticalHeaderItem (0, new QTableWidgetItem { "Avg of " + QString::number (n)});
        table->setItem (0, 0, makeColorItem (p->getTimeString (p->getAvg (times, n))));
        table->setItem (0, 1, makeColorItem (p->floatToQString (p->getAvg (movecounts, n))));
        table->setItem (0, 2, makeColorItem (p->floatToQString (p->getAvg (tps_s, n))));
        table->setItem (0, 3, makeColorItem ());
        table->setItem (0, 4, makeColorItem ());

        for (int x = 0; x < n; x++)
        {
            table->setVerticalHeaderItem (x + 1, new QTableWidgetItem {QString::number (x + 1)});
            QTableWidgetItemCopyable *time = makeCenterAlignedItem (p->getTimeString (times[x]));
            if (bests[0].contains (x))
                time->setBackgroundColor (Qt::green);
            else if (worsts[0].contains (x))
                time->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 0, time);

            QTableWidgetItemCopyable *movecount = makeCenterAlignedItem (p->floatToQString (movecounts[x]));
            if (bests[1].contains (x))
                movecount->setBackgroundColor (Qt::green);
            else if (worsts[1].contains (x))
                movecount->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 1, movecount);

            QTableWidgetItemCopyable *tps = makeCenterAlignedItem (p->floatToQString (tps_s[x]));
            if (bests[2].contains (x))
                tps->setBackgroundColor (Qt::green);
            else if (worsts[2].contains (x))
                tps->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 2, tps);
            table->setItem (x + 1, 3, makeLeftAlignedItem (avg.solves[x].scramble.toQString ()));
            table->setItem (x + 1, 4, makeCenterAlignedItem (avg.solves[x].datetime.toString (datetime_format)));
        }
    }
}

void PBsViewer::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    displayPBs ();
}
