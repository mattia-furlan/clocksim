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
#ifndef STRUCTURES_H
#define STRUCTURES_H


#include <QDataStream>
#include <QVector>
#include <QTime>
#include <QDateTime>

#include <cfloat>

struct Scramble
{
    qint8 first[4];
    qint8 second[4];
    qint8 all_1;
    //y2
    qint8 third[4];
    qint8 all_2;
    bool ul;
    bool ur;
    bool dl;
    bool dr;

    Scramble () = default;

    Scramble (const QByteArray& ba);

    QString toQString () const;

    uint getBytesCount () const
    {
        return 15; //4 + 4 + 4 + 1 + 1 + 1
    }
};

struct Solve
{
    float time {FLT_MAX};
    float movecount {FLT_MAX};
    float tps {FLT_MAX};
    Scramble scramble;
    QString reconstruction; //not used yet
    QDateTime datetime;

    Solve (float ti = FLT_MAX, float mo = FLT_MAX, float tp = FLT_MAX, const Scramble& s = {}, const QString& r = {})
    {
        time = ti;
        movecount = mo;
        tps = tp;
        scramble = s;
        reconstruction = r;
        datetime = QDateTime::currentDateTime ();
    }

    uint getBytesCount () const
    {
        return (sizeof (float) * 3
                + scramble.getBytesCount ()
                + sizeof (reconstruction.size ())
                + reconstruction.size ()
                + sizeof (datetime.toTime_t ()));
    }
};

struct Average
{
    float avg {FLT_MAX};
    QVector<Solve> solves;

    Average () = default;
    Average (float average, QVector<Solve> ss)
    {
        avg = average;
        solves = ss;
    }

    uint getBytesCount () const
    {
        uint count {0};
        std::for_each (solves.begin (), solves.end (), [&count] (const Solve& s) {count += s.getBytesCount ();});
        return count + sizeof (float);
    }
};

struct PBs
{
    Solve single;
    Average avg5;
    Average avg12;
    Average avg50;
    Average avg100;
};

QDataStream& operator<< (QDataStream&, const Solve&);
QDataStream& operator>> (QDataStream&, Solve&);
QDataStream& operator<< (QDataStream&, const Scramble&);
QDataStream& operator>> (QDataStream&, Scramble&);
QDataStream& operator<< (QDataStream&, const Average&);
QDataStream& operator>> (QDataStream&, Average&);

#endif // STRUCTURES_H
