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
#ifndef CLOCK_H
#define CLOCK_H


#include <QtGlobal>
#include <QDebug>
#include <QTime>
#include <QByteArray>
#include <QDataStream>
#include <QSet>

#include <ctime>
#include <cstdlib>
//#include <utility>

#include "structures.h"

typedef unsigned int hour_t;

struct Clock
{
    hour_t hour {0};

    void turn (int m)
    {
        while (m < 0)
            m += 12;
        hour += m;
        hour %= 12;
    }
};

struct Corner
{
    Clock clock;
};

struct Edge
{
    Clock front;
    Clock back;
};

struct Center
{
    Clock front;
    Clock back;
};

struct Pin
{
    Corner *corner;
    Edge *edges[2];
    Center *center;
    bool up {false};

    inline void flip ()
    {
        up = !up;
    }
    inline bool isUp () const
    {
        return up;
    }
    inline void set (bool state)
    {
        up = state;
    }
};

struct ClockPuzzle
{
    Pin pins[4];
    Corner corners[4];
    Edge edges[4];
    Center center;
    QSet<Clock*> clocks;
    bool front {true};

    ClockPuzzle ();

    void y2 ();

    void turn (int m, size_t wheel);

    inline bool isFront () const
    {
        return front;
    }

    void addClocks (QSet<Clock*>& v, int pin);
/*#define UL(m) turn (m, 0)
#define UR(m) turn (m, 1)
#define DL(m) turn (m, 2)
#define DR(m) turn (m, 3)*/
    inline void UL (int m)
    {
        turn (m, 0);
    }

    inline void UR (int m)
    {
        turn (m, 1);
    }

    inline void DL (int m)
    {
        turn (m, 2);
    }

    inline void DR (int m)
    {
        turn (m, 3);
    }
/*#define pinUL(state) pins[0].set (state)
#define pinUR(state) pins[1].set (state)
#define pinDL(state) pins[2].set (state)
#define pinDR(state) pins[3].set (state)*/

    inline void U (int m)
    {
        pinUL (true);
        pinUR (true);
        UL (m);
        pinUL (false);
        pinUR (false);
    }

    inline void L (int m)
    {
        pinUL (true);
        pinDL (true);
        UL (m);
        pinUL (false);
        pinDL (false);
    }

    inline void D (int m)
    {
        pinDR (true);
        pinDL (true);
        DL (m);
        pinDR (false);
        pinDL (false);
    }

    inline void R (int m)
    {
        pinDR (true);
        pinUR (true);
        DR (m);
        pinDR (false);
        pinUR (false);
    }

    inline void all (int m)
    {
        pinall (true);
        UL (m);
        pinall (false);
    }

    inline void pinUL (bool state)
    {
        pins[0].set (state);
    }

    inline void pinUR (bool state)
    {
        pins[1].set (state);
    }

    inline void pinDL (bool state)
    {
        pins[2].set (state);
    }

    inline void pinDR (bool state)
    {
        pins[3].set (state);
    }

    inline void pinall (bool state)
    {
        for (int i = 0; i < 4; i++)
            pins[i].set (state);
    }

    bool isSolved () const;

    void solve ();

    Scramble scramble ();
};


#endif // CLOCK_H
