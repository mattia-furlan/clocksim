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
#include "structures.h"

Scramble::Scramble (const QByteArray& ba)
{
    Q_ASSERT (ba.size () == 15);
    first[0] = ba[0]; first[1] = ba[1]; first[2] = ba[2]; first[3] = ba[3];
    second[0] = ba[4]; second[1] = ba[5]; second[2] = ba[6]; second[3] = ba[7];
    all_1 = ba[8];
    third[0] = ba[9]; third[1] = ba[10]; third[2] = ba[11]; third[3] = ba[12];
    all_2 = ba[13];
    dr = ba[14] & 1;
    dl = ba[14] & 2;
    ur = ba[14] & 4;
    ul = ba[14] & 8;
}

QString Scramble::toQString () const
{
#define X(x, move) (x ? QString {move + QString::number (x > 0 ? x : -x) + (x > 0 ? "+ " : "- ")} : QString {""})
    QString scramble = X (first[0], "UR") +
            X (first[1], "DR") +
            X (first[2], "DL") +
            X (first[3], "UL") +
            X (second[0], "U") +
            X (second[1], "R") +
            X (second[2], "D") +
            X (second[3], "L") +
            X (all_1, "ALL") +
            "y2 " +
            X (third[0], "U") +
            X (third[1], "R") +
            X (third[2], "D") +
            X (third[3], "L") +
            X (all_2, "ALL") +
            (ul ? "UL " : "") +
            (ur ? "UR " : "") +
            (dr ? "DR " : "") +
            (dl ? "DL " : "");
#undef X
    return scramble.left (scramble.size () - 1); //delete the last space
}

QDataStream& operator<< (QDataStream& stream, const Solve& s)
{
    stream << s.time << s.movecount << s.tps;
    stream << s.scramble;
    stream << s.reconstruction.size ();
    foreach (quint8 x, s.reconstruction.toLocal8Bit ())
        stream << x;
    stream << s.datetime.toTime_t ();
    return stream;
}

QDataStream& operator>> (QDataStream& stream, Solve& s)
{
    stream >> s.time >> s.movecount >> s.tps;
    stream >> s.scramble;
    uint size;
    stream >> size;
    while (size-- != 0)
    {
        qint8 x;
        stream >> x;
        s.reconstruction += x;
    }
    uint datetime;
    stream >> datetime;
    s.datetime.setTime_t (datetime);
    return stream;
}

QDataStream& operator<< (QDataStream& stream, const Scramble& s)
{
    stream << s.first[0] << s.first[1] << s.first[2] << s.first[3];
    stream << s.second[0] << s.second[1] << s.second[2] << s.second[3];
    stream << s.all_1;
    stream << s.third[0] << s.third[1] << s.third[2] << s.third[3];
    stream << s.all_2;
    quint8 pins = s.dr;
    pins |= (s.dl << 1);
    pins |= (s.ur << 2);
    pins |= (s.ul << 3);
    stream << pins;
    return stream;
}

QDataStream& operator>> (QDataStream& stream, Scramble& s)
{
    stream >> s.first[0] >> s.first[1] >> s.first[2] >> s.first[3];
    stream >> s.second[0] >> s.second[1] >> s.second[2] >> s.second[3];
    stream >> s.all_1;
    stream >> s.third[0] >> s.third[1] >> s.third[2] >> s.third[3];
    stream >> s.all_2;
    quint8 pins;
    stream >> pins;
    s.dr = pins & 1;
    s.dl = pins & 2;
    s.ur = pins & 4;
    s.ul = pins & 8;
    return stream;
}


QDataStream& operator<< (QDataStream& stream, const Average& avg)
{
    stream << avg.avg;
    stream << avg.solves.size ();
    foreach (const Solve& s, avg.solves)
        stream << s;
    return stream;
}

QDataStream& operator>> (QDataStream& stream, Average& avg)
{
    stream >> avg.avg;
    int size;
    stream >> size;
    avg.solves.clear ();
    while (size-- > 0)
    {
        Solve s;
        stream >> s;
        avg.solves.push_back (s);
    }
    return stream;
}
