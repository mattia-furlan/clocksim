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
#include "clock.h"

void ClockPuzzle::addClocks (QSet<Clock *> &v, int pin)
{
    bool up = pins[pin].isUp ();

    for (int i = 0; i < 4; i++)
    {
        if (pin == i)
            continue;

        bool pinUp = pins[i].isUp ();

        if (up && pinUp)
        {
            v.insert (&pins[i].edges[0]->front);
            v.insert (&pins[i].edges[1]->front);
            v.insert (&pins[i].corner->clock);
        }
        else if (!up && !pinUp)
        {
            v.insert (&pins[i].edges[0]->back);
            v.insert (&pins[i].edges[1]->back);
            v.insert (&pins[i].corner->clock);
        }
    }
}

ClockPuzzle::ClockPuzzle ()
{
    srand (time (0));

    for (int i = 0; i < 4; i++)
    {
        pins[i].corner = &corners[i];

        pins[i].center = &center;
    }

    pins[0].edges[0] = &edges[0];
    pins[0].edges[1] = &edges[1];

    pins[1].edges[0] = &edges[0];
    pins[1].edges[1] = &edges[2];

    pins[2].edges[0] = &edges[3];
    pins[2].edges[1] = &edges[1];

    pins[3].edges[0] = &edges[3];
    pins[3].edges[1] = &edges[2];
}

void ClockPuzzle::y2 ()
{
    front = !front;

    //Pins
    for (int i = 0; i < 4; i++)
        pins[i].flip ();

    std::swap (pins[0], pins[1]);
    std::swap (pins[2], pins[3]);

    //Corners
    for (int i = 0; i < 4; i++)
        corners[i].clock.hour = (12 - corners[i].clock.hour) % 12;

    std::swap (corners[0], corners[1]);
    std::swap (corners[2], corners[3]);
    std::swap (pins[0].corner, pins[1].corner);
    std::swap (pins[2].corner, pins[3].corner);

    //Center
    center.front.hour = (12 - center.front.hour) % 12;
    center.back.hour = (12 - center.back.hour) % 12;
    std::swap (center.front, center.back);

    //Edges
    for (int i = 0; i < 4; i++)
    {
        edges[i].front.hour = (12 - edges[i].front.hour) % 12;
        edges[i].back.hour  = (12 -  edges[i].back.hour) % 12;

        std::swap (edges[i].front, edges[i].back);
    }
    std::swap (edges[1], edges[2]);
    std::swap (pins[0].edges[1], pins[1].edges[1]);
    std::swap (pins[2].edges[1], pins[3].edges[1]);

}

void ClockPuzzle::turn (int m, size_t wheel)
{
    clocks.insert (&corners[wheel].clock);

    if (pins[wheel].isUp ())
    {
        clocks.insert (&pins[wheel].edges[0]->front);
        clocks.insert (&pins[wheel].edges[1]->front);
        clocks.insert (&center.front);
    }
    else
    {
        clocks.insert (&pins[wheel].edges[0]->back);
        clocks.insert (&pins[wheel].edges[1]->back);
        clocks.insert (&center.back);
    }

    addClocks (clocks, wheel);

    for (const auto& clock : clocks)
        clock->turn (m);

    clocks.clear ();
}

bool ClockPuzzle::isSolved () const
{
    for (int i = 0; i < 4; i++)
    {
        if (corners[i].clock.hour != 0)
            return false;
        if (edges[i].front.hour != 0 || edges[i].back.hour != 0)
            return false;
    }
    if (center.front.hour != 0 || center.back.hour != 0)
        return false;
    return true;
}

void ClockPuzzle::solve ()
{
    if (!front)
        y2 ();
    center.front.hour = 0;
    center.back.hour = 0;
    for (int i = 0; i < 4; i++)
    {
        edges[i].front.hour = 0;
        edges[i].back.hour = 0;

        corners[i].clock.hour = 0;

        pins[i].set (false);
    }
}

Scramble ClockPuzzle::scramble ()
{
    solve ();

    int x;
    bool sign; // 0 = -, 1 = +

    auto genMove = [&x, &sign] () -> int
    {
        x = rand () % 7;
        if (x == 6) //to avoid '6-' moves (use '6+' instead)
            return 6;
        sign = static_cast<bool> (qrand () % 2);
        return x *= (sign ? 1 : -1);
    };

#define DO_MOVE(wheel, str) do {genMove (); pin##wheel (true); wheel (x); pin##wheel (false);} while (0)
#define DO_DOUBLE_MOVE(wheels, str) do {genMove (); wheels (x);} while (0)
#define DO_PIN(pin, n) do {x = qrand () % 2; pins[n].set (x);} while (0)

    Scramble scramble;

    DO_MOVE (UR, "UR"); scramble.first[0] = x;
    DO_MOVE (DR, "DR"); scramble.first[1] = x;
    DO_MOVE (DL, "DL"); scramble.first[2] = x;
    DO_MOVE (UL, "UL"); scramble.first[3] = x;

    DO_DOUBLE_MOVE (U, "U"); scramble.second[0] = x;
    DO_DOUBLE_MOVE (R, "R"); scramble.second[1] = x;
    DO_DOUBLE_MOVE (D, "D"); scramble.second[2] = x;
    DO_DOUBLE_MOVE (L, "L"); scramble.second[3] = x;

    DO_DOUBLE_MOVE (all, "ALL"); scramble.all_1 = x;

    y2 ();
    pinall (false);

    DO_DOUBLE_MOVE (U, "U"); scramble.third[0] = x;
    DO_DOUBLE_MOVE (R, "R"); scramble.third[1] = x;
    DO_DOUBLE_MOVE (D, "D"); scramble.third[2] = x;
    DO_DOUBLE_MOVE (L, "L"); scramble.third[3] = x;

    DO_DOUBLE_MOVE (all, "ALL"); scramble.all_2 = x;

    y2 ();

    DO_PIN ("UR", 1); scramble.ur = x;
    DO_PIN ("DR", 3); scramble.dr = x;
    DO_PIN ("DL", 2); scramble.dl = x;
    DO_PIN ("UL", 0); scramble.ul = x;


    return scramble;

#undef DO_MOVE
#undef DO_DOUBLE_MOVE
#undef DO_PIN
}
