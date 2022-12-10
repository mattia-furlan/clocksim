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
#include "clocksim.h"
#include "ui_ClockSim.h"
#include "ui_settingsform.h"

#include "pbsviewer.h"

ClockSim::ClockSim (QWidget *parent) : QWidget {parent}, ui {new Ui::ClockSim}
{
    ui->setupUi (this);
    setWindowTitle ("Clocksim v" + VERSION_STRING);
    setMouseTracking (true);
    settings = new SettingsForm {this};
    pbs_viewer = new PBsViewer {this};
    setMinimumSize (QSize {width (), height ()});
    setMaximumSize (QSize {width (), height ()});
    timer = new QTimer {this};

    connect (timer, SIGNAL (timeout ()), this, SLOT (timerTimeout ()));
    connect (ui->pushButtonSettings, SIGNAL (clicked ()), settings, SLOT (show ()));
    connect (ui->pushButtonPBs, SIGNAL (clicked ()), pbs_viewer, SLOT (show ()));

#define X(label, n) connect (label, &QClickableLabel::clicked, this, [this] {displayTimes (n);})

    X (ui->labelSingleTime, 1); X (ui->labelSingleMovecount, 1); X (ui->labelSingleTPS, 1);
    X (ui->labelAvg5Time, 5); X (ui->labelAvg5Movecount, 5); X (ui->labelAvg5TPS, 5);
    X (ui->labelAvg12Time, 12); X (ui->labelAvg12Movecount, 12); X (ui->labelAvg12TPS, 12);
    X (ui->labelAvg50Time, 50); X (ui->labelAvg50Movecount, 50); X (ui->labelAvg50TPS, 50);
    X (ui->labelAvg100Time, 100); X (ui->labelAvg100Movecount, 100); X (ui->labelAvg100TPS, 100);
    X (ui->labelSessionAvgTime, times_vector.size ()); X (ui->labelSessionAvgMovecount, times_vector.size ()); X (ui->labelSessionAvgTPS, times_vector.size ());
    X (ui->labelSessionMeanTime, 0); X (ui->labelSessionMeanMovecount, 0); X (ui->labelSessionMeanTPS, 0);

#undef X

    readPBs ();
    readSession ();
    readSettings ();
    qDebug ("1");
    updateStats ();
}

ClockSim::~ClockSim ()
{
    delete ui;
    delete timer;
    delete settings;
    delete pbs_viewer;
}

QString ClockSim::getTimeString (float time)
{
    int inttime = static_cast<int> (time);
    int msec = (time - inttime) * std::pow (10, settings->getTimerPrecision ());
    int sec = inttime % 60;
    int min = inttime / 60;

    QString s_msec = QString {"%1"}.arg (msec, settings->getTimerPrecision (), 10, QChar {'0'});
    QString s_sec;
    if (min > 0)
        s_sec = QString {"%1"}.arg (sec, 2, 10, QChar {'0'});
    else
        s_sec = QString::number (sec);
    QString s_min = QString::number (min);
    QString s = (min > 0 ? s_min + ":" : "") + s_sec + "." + s_msec;

    return s;
}

QString ClockSim::floatToQString (float x)
{
    return QString::number (x, 'f', settings->getTimerPrecision ());
}

void ClockSim::doZRotation (int rot)
{
    if (rot == -1)
        rot = 3;
    for (int i = 0; i < rot; i++)
    {
        angle += 90;

#define X(pin) settings->pins_controls[settings->ui->pin]
        int ul = X (keyUL);
        int ur = X (keyUR);
        int dl = X (keyDL);
        int dr = X (keyDR);
        X (keyUL) = dl;
        X (keyUR) = ul;
        X (keyDL) = dr;
        X (keyDR) = ur;
#undef X
    }

}

void ClockSim::doY2Rotation ()
{
    clock.y2 ();

    if (angle % 90 == 0 && angle % 180 != 0) //swap pins controls
    {
        std::swap (settings->pins_controls[settings->ui->keyUL], settings->pins_controls[settings->ui->keyDR]);
        std::swap (settings->pins_controls[settings->ui->keyUR], settings->pins_controls[settings->ui->keyDL]);

        angle += 180;
    }
}

void ClockSim::timerTimeout ()
{
    QTime diff  = QTime {0, 0, 0}.addMSecs (beginning.elapsed ());

    int msec = diff.msec ();
    int sec = diff.second ();
    int min = diff.minute ();

    float tot_time = (min * 60.0) + sec + (msec / 1000.0);

    QString time_s;

    auto index = settings->ui->comboBoxTimerUpdating->currentIndex ();
    if (index == 0) //On
        time_s = getTimeString (tot_time);
    else if (index == 1) //Off
        time_s = "Running...";
    else //Seconds only
    {
        QString buffer = getTimeString (tot_time);
        time_s = buffer.left (buffer.size () - 4); //delete the three decimal digits and the dot
    }

    float tps = movecount / tot_time;

    ui->labelSingleTime->setText (time_s);
    ui->labelSingleMovecount->setText (QString::number (movecount));
    ui->labelSingleTPS->setText (floatToQString (tps));

    if (clock.isSolved ())
    {
        timer->stop ();
        is_solving = false;

        current[0].single = tot_time;
        current[1].single = movecount;
        current[2].single = tps;

        solves.push_back (Solve {tot_time, movecount, tps, last_scramble, reconstruction});

        updateStats ();

        ui->pushButtonSettings->setEnabled (true);
    }
}

void ClockSim::startSolve ()
{
    is_scrambled = false;
    is_solving = true;
    movecount = 0;
    reconstruction = "";
    beginning = QTime::currentTime ();
    applyToLabels (&QLabel::setStyleSheet, QString {"color:black;"});
    ui->pushButtonSettings->setEnabled (false);
    timer->start (10);
}

void ClockSim::updateInspectionCount ()
{
    QTime now = QTime::currentTime ();
    int msecs = beginning.msecsTo (now);
    float secs = (float) msecs / 1000;

    if (!is_scrambled || is_solving)
    {
        ui->labelNOSolves->setText ("Solves: " + QString::number (solves.size ()));
        return;
    }
    if (secs >= 15.0)
    {
        startSolve ();
        ui->labelNOSolves->setText ("Solves: " + QString::number (solves.size ()));
        return ;
    }
    ui->labelNOSolves->setText ("Inspection: " + QString::number (qCeil (15.0 - secs)));
    QTimer::singleShot (10, this, SLOT (updateInspectionCount ()));
}

void ClockSim::keyPressEvent (QKeyEvent *event)
{
    int key = event->key ();

    bool move {false};
    int count {0};

    Qt::KeyboardModifiers modifiers = event->modifiers ();

    if (modifiers.testFlag (Qt::ShiftModifier))
        key += Qt::SHIFT;
    if (modifiers.testFlag (Qt::ControlModifier))
        key += Qt::CTRL;
    if (modifiers.testFlag (Qt::AltModifier))
        key += Qt::ALT;

    QKeySequence keySeq = QKeySequence (key);

    if (keySeq == settings->ui->key_scramble->getKey () && !is_solving)
    {
        if (!is_scrambled)
        {
            while (angle % 360 != 0)
                doZRotation (1);
            last_scramble = clock.scramble ();
            is_scrambled = true;
            beginning = QTime::currentTime ();
            update ();
            QTimer::singleShot (10, this, SLOT (updateInspectionCount ()));
        }
    }

    if (keySeq == settings->ui->key_y2->getKey ())
    {
        doY2Rotation ();
        movecount++;
    }

    if (keySeq == settings->ui->key_z->getKey ())
    {
        doZRotation (1);
        movecount++;
    }

    if (keySeq == settings->ui->key_zp->getKey ())
    {
        doZRotation (-1);
        movecount++;
    }

    if (keySeq == settings->ui->key_z2->getKey ())
    {
        doZRotation (2);
        movecount++;
    }

    if (keySeq == settings->ui->key_x2->getKey ())
    {
        doY2Rotation ();
        doZRotation (2);
        movecount++;
    }

    for (const auto& seq : settings->wheels_controls.toStdMap ())
    {
        if (keySeq == seq.first->getKey ())
        {
            move = true;
            count = seq.second;
            if (is_scrambled)
                startSolve ();
            movecount++;
        }
    }
    for (const auto& seq : settings->pins_controls.toStdMap ())
    {
        if (keySeq == seq.first->getKey ())
        {
            clock.pins[seq.second].flip ();
            if (is_scrambled)
                startSolve ();
            movecount++;
        }
    }

    if (key == Qt::Key_Escape)
    {
        if (times_vector.size () > 0)
        {
            if (settings->ui->checkBoxSessionResetPopup->isChecked ())
            {
                auto reply = QMessageBox::question (this, "QtClock", "This will reset your session, are you sure?",
                                          QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No)
                    return;
            }
        }
        solves.clear ();

        movecount = 0;
        reconstruction = "";
        last_scramble = Scramble {};

        is_solving = false;
        is_scrambled = false;

        timer->stop ();
        clock.solve ();

        applyToLabels (&QLabel::clear);
        ui->labelNOSolves->setText ("Solves: 0");
        ui->pushButtonSettings->setEnabled (true);

        updateStats ();
    }

    if (move)
    {
#define PIN_UP(i) clock.pins[i].isUp ()

        if (PIN_UP (0))
            clock.UL (count);
        else if (PIN_UP (1))
            clock.UR (count);
        else if (PIN_UP (2))
            clock.DL (count);
        else if (PIN_UP (3))
            clock.DR (count);
        else
            clock.UL (count);

#undef PIN_UP
    }

    update ();
}

void ClockSim::closeEvent (QCloseEvent *event)
{
    if (is_solving)
    {
        /*auto reply = QMessageBox::question (this, "QtClock", "Exiting during a solve will reset your session, are you sure?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;*/
        solves.clear ();
    }
    savePBs ();
    saveSession ();
    saveSettings ();

    event->accept ();
}

QVector<int> getBestN (const QVector<float>& v, int n)
{
    QVector<int> indices;
    int i = 0;
    std::generate_n (std::back_inserter (indices), v.size (),[&i] () {return i++;});

    std::partial_sort (indices.begin (), indices.begin () + n, indices.end (),
      [&v] (int ind1, int ind2) {return v[ind1] <= v[ind2];});

    return indices.mid (0, n);
}

QVector<int> getWorstN (const QVector<float>& v, int n)
{
    QVector<int> indices;
    int i = 0;
    std::generate_n (std::back_inserter (indices), v.size (),[&i] () {return i++;});

    std::partial_sort (indices.begin (), indices.begin () + n, indices.end (),
      [&v] (int ind1, int ind2) {return v[ind1] >= v[ind2];});

    return indices.mid (0, n);
}


float ClockSim::getAvg (const QVector<float>& v, int n)
{
    Q_ASSERT (v.size () >= n);

    int m = static_cast<int> (qCeil (n * 0.1));

    QVector<float> slice = v.mid (v.size () - n, n);

    for (int i = 0; i < m; i++)
    {
        auto min = std::min_element (slice.begin (), slice.end ());
        slice.erase (min);

        auto max = std::max_element (slice.begin (), slice.end ());
        slice.erase (max);
    }

    return std::accumulate (slice.begin (), slice.end (), 0.0f) / static_cast<float> (n - 2 * m);
}

float ClockSim::getMean (const QVector<float>& v, int n)
{
    Q_ASSERT (v.size () >= n);

    return std::accumulate (v.begin () + v.size () - n, v.end (), 0.0f) / n;
}

void ClockSim::savePBs ()
{
    QFile file {QStandardPaths::writableLocation (QStandardPaths::HomeLocation) + "/clockPBs.bin"};
    if (!file.open (QIODevice::WriteOnly))
        return;

    QDataStream out {&file};
    out.setFloatingPointPrecision (QDataStream::SinglePrecision);

    for (int i = 0; i < 3; i++)
    {
        out << pbs[i].single;
        out << pbs[i].avg5;
        out << pbs[i].avg12;
        out << pbs[i].avg50;
        out << pbs[i].avg100;
    }

    file.close ();
}

void ClockSim::readPBs ()
{
    QFile file {QStandardPaths::writableLocation (QStandardPaths::HomeLocation) + "/clockPBs.bin"};
    if (!file.open (QIODevice::ReadOnly))
        return;

    QDataStream in {&file};
    in.setFloatingPointPrecision (QDataStream::SinglePrecision);

    for (int i = 0; i < 3; i++)
    {
        in >> pbs[i].single;
        in >> pbs[i].avg5;
        in >> pbs[i].avg12;
        in >> pbs[i].avg50;
        in >> pbs[i].avg100;
    }
    file.close ();
}

void ClockSim::saveSession ()
{
    QFile file {QStandardPaths::writableLocation (QStandardPaths::HomeLocation) + "/clocksession.bin"};
    if (!file.open (QIODevice::WriteOnly))
        return;

    QDataStream out {&file};
    out.setFloatingPointPrecision (QDataStream::SinglePrecision);

    foreach (const Solve& s, solves)
        out << s;

    file.close ();
}

void ClockSim::readSession ()
{
    QFile file {QStandardPaths::writableLocation (QStandardPaths::HomeLocation) + "/clocksession.bin"};
    if (!file.open (QIODevice::ReadOnly))
        return;
    QDataStream in {&file};
    in.setFloatingPointPrecision (QDataStream::SinglePrecision);

    int bytes = file.size ();

    while (bytes > 0)
    {
        Solve buffer;
        in >> buffer;
        solves.push_back (buffer);

        current[0].single = buffer.time;
        current[1].single = buffer.movecount;
        current[2].single = buffer.tps;

        bytes -= buffer.getBytesCount ();
    }

    updateStats ();

    file.close ();
}

void ClockSim::saveSettings ()
{
    QSettings s {"MF", "clocksim"};
    //drawing
    s.beginGroup ("Drawing");
    s.setValue ("draw_dots", settings->ui->checkBoxDrawDots->isChecked ());
    s.setValue ("clock_hand_style", settings->ui->comboBoxClockHandStyle->currentIndex ());

    s.setValue ("pin_up_color", settings->ui->pinUpColor->getColor ());
    s.setValue ("pin_down_color", settings->ui->pinDownColor->getColor ());

    s.setValue ("front_color", settings->ui->frontColor->getColor ());
    s.setValue ("back_color", settings->ui->backColor->getColor ());

    s.setValue ("clock_hand_color", settings->ui->clockHandColor->getColor ());
    s.setValue ("dots_color", settings->ui->dotsColor->getColor ());

    s.setValue ("dot_12_color", settings->ui->dot12Color->getColor ());
    s.setValue ("sign_12_color", settings->ui->sign12Color->getColor ());

    s.setValue ("clock_border_color", settings->ui->clockBorderColor->getColor ());
    s.setValue ("clock_hand_border_color", settings->ui->clockHandBorderColor->getColor ());
    s.endGroup ();

    //timer
    s.beginGroup ("Timer");
    s.setValue ("timer_updating", settings->ui->comboBoxTimerUpdating->currentIndex ());
    s.setValue ("timer_precision", settings->getTimerPrecision ());
    s.setValue ("session_reset_popup", settings->ui->checkBoxSessionResetPopup->isChecked ());
    s.endGroup ();

    //controls
    s.beginGroup ("Controls");
    s.setValue ("key1p", settings->ui->key1p->getKey ());
    s.setValue ("key2p", settings->ui->key2p->getKey ());
    s.setValue ("key3p", settings->ui->key3p->getKey ());
    s.setValue ("key4p", settings->ui->key4p->getKey ());
    s.setValue ("key5p", settings->ui->key5p->getKey ());
    s.setValue ("key6", settings->ui->key6->getKey ());
    s.setValue ("key1m", settings->ui->key1m->getKey ());
    s.setValue ("key2m", settings->ui->key2m->getKey ());
    s.setValue ("key3m", settings->ui->key3m->getKey ());
    s.setValue ("key4m", settings->ui->key4m->getKey ());
    s.setValue ("key5m", settings->ui->key5m->getKey ());
    s.setValue ("pinUL", settings->ui->keyUL->getKey ());
    s.setValue ("pinUR", settings->ui->keyUR->getKey ());
    s.setValue ("pinDL", settings->ui->keyDL->getKey ());
    s.setValue ("pinDR", settings->ui->keyDR->getKey ());
    s.setValue ("key_scramble", settings->ui->key_scramble->getKey ());
    s.setValue ("key_y2", settings->ui->key_y2->getKey ());
    s.setValue ("key_x2", settings->ui->key_x2->getKey ());
    s.setValue ("key_z", settings->ui->key_z->getKey ());
    s.setValue ("key_zp", settings->ui->key_zp->getKey ());
    s.setValue ("key_z2", settings->ui->key_z2->getKey ());
    s.endGroup ();
}

void ClockSim::readSettings ()
{
    QSettings s {"MF", "clocksim"};

    s.beginGroup ("Drawing");
    if (s.contains ("draw_dots"))
        settings->ui->checkBoxDrawDots->setChecked (s.value ("draw_dots").toBool ());
    if (s.contains ("clock_hand_style"))
        settings->ui->comboBoxClockHandStyle->setCurrentIndex (s.value ("clock_hand_style").toInt ());
#define READ_COLOR(name, c) if (s.contains (name)) c->setDefaultColor (s.value (name).value<QColor> ())
    READ_COLOR ("pin_up_color", settings->ui->pinUpColor);
    READ_COLOR ("pin_down_color", settings->ui->pinDownColor);

    READ_COLOR ("front_color", settings->ui->frontColor);
    READ_COLOR ("back_color", settings->ui->backColor);

    READ_COLOR ("clock_hand_color", settings->ui->clockHandColor);
    READ_COLOR ("dots_color", settings->ui->dotsColor);

    READ_COLOR ("dot_12_color", settings->ui->dot12Color);
    READ_COLOR ("sign_12_color", settings->ui->sign12Color);

    READ_COLOR ("clock_border_color", settings->ui->clockBorderColor);
    READ_COLOR ("clock_hand_border_color", settings->ui->clockHandBorderColor);

#undef READ_COLOR
    s.endGroup ();

    s.beginGroup ("Timer");

    if (s.contains ("timer_updating"))
        settings->ui->comboBoxTimerUpdating->setCurrentIndex (s.value ("timer_updating").toInt ());
    if (s.contains ("timer_precision"))
    {
        int prec = s.value ("timer_precision").toInt ();
        if (prec == 0)
            settings->ui->radioButton1_10->setChecked (true);
        else if (prec == 1)
            settings->ui->radioButton1_100->setChecked (true);
        else
            settings->ui->radioButton1_1000->setChecked (true);
    }
    else
        settings->ui->radioButton1_1000->setChecked (true); //default = 0.001
    if (s.contains ("session_reset_popup"))
        settings->ui->checkBoxSessionResetPopup->setChecked (s.value ("session_reset_popup").toBool ());

    s.endGroup ();
#define READ_KEY(name, k) if (s.contains (name)) k->setDefaultKey ({s.value (name).value <QKeySequence> ()});
    s.beginGroup ("Controls");

    READ_KEY ("key1p", settings->ui->key1p);
    READ_KEY ("key2p", settings->ui->key2p);
    READ_KEY ("key3p", settings->ui->key3p);
    READ_KEY ("key4p", settings->ui->key4p);
    READ_KEY ("key5p", settings->ui->key5p);
    READ_KEY ("key6", settings->ui->key6);
    READ_KEY ("key1m", settings->ui->key1m);
    READ_KEY ("key2m", settings->ui->key2m);
    READ_KEY ("key3m", settings->ui->key3m);
    READ_KEY ("key4m", settings->ui->key4m);
    READ_KEY ("key5m", settings->ui->key5m);
    READ_KEY ("pinUL", settings->ui->keyUL);
    READ_KEY ("pinUR", settings->ui->keyUR);
    READ_KEY ("pinDL", settings->ui->keyDL);
    READ_KEY ("pinDR", settings->ui->keyDR);
    READ_KEY ("key_scramble", settings->ui->key_scramble);
    READ_KEY ("key_y2", settings->ui->key_y2);
    READ_KEY ("key_x2", settings->ui->key_x2);
    READ_KEY ("key_z", settings->ui->key_z);
    READ_KEY ("key_zp", settings->ui->key_zp);
    READ_KEY ("key_z2", settings->ui->key_z2);
#undef READ_KEY
    s.endGroup ();
}

void ClockSim::displayTimes (int n)
{
    Q_ASSERT (solves.size () >= n);

    auto size = solves.size ();
    const QString datetime_format {"dd/MM/yy - hh:mm"};

    const auto& solves_slice = solves.mid (size - n, n);

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

    auto *table  = new QTableWidget;
    int height = 23 ;
    if (n == 0)
        height += 30 + times_vector.size () * 30;
    else if (n == 1)
        height += 30;
    else
        height += 30 + n * 30;

    table->setColumnCount (5);
    table->setRowCount (1);

    table->setHorizontalHeaderLabels (QStringList () << "Time" << "Movecount" << "TPS" << "Scramble" << "When");

    if (n == 0) //session mean
    {
        n = times_vector.size ();
        table->setRowCount (n + 1);
        table->setVerticalHeaderItem (0, new QTableWidgetItem {"Session mean"});

        table->setItem (0, 0, makeColorItem (getTimeString (getMean (times_vector, n))));
        table->setItem (0, 1, makeColorItem (floatToQString (getMean (movecount_vector, n))));
        table->setItem (0, 2, makeColorItem (floatToQString (getMean (tps_vector, n))));
        table->setItem (0, 3, makeColorItem ());
        table->setItem (0, 4, makeColorItem ());

        QVector<int> bests[3] = {getBestN (times_vector, 1), getBestN (movecount_vector, 1), getWorstN (tps_vector, 1)};
        QVector<int> worsts[3] = {getWorstN (times_vector, 1), getWorstN (movecount_vector, 1), getBestN (tps_vector, 1)};

        if (n == 1) //just one time = no highlighting
            for (int i = 0; i < 3; i++)
            {
                bests[i].clear ();
                worsts[i].clear ();
            }

        for (int x = 0; x < n; x++)
        {
            table->setVerticalHeaderItem (x + 1, new QTableWidgetItem {QString::number (x + 1)});

            QTableWidgetItemCopyable *time = makeCenterAlignedItem (getTimeString (times_vector[x]));
            if (bests[0].contains (x))
                time->setBackgroundColor (Qt::green);
            else if (worsts[0].contains (x))
                time->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 0, time);

            QTableWidgetItemCopyable *movecount = makeCenterAlignedItem (floatToQString (movecount_vector[x]));
            if (bests[1].contains (x))
                movecount->setBackgroundColor (Qt::green);
            else if (worsts[1].contains (x))
                movecount->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 1, movecount);

            QTableWidgetItemCopyable *tps = makeCenterAlignedItem (floatToQString (tps_vector[x]));
            if (bests[2].contains (x))
                tps->setBackgroundColor (Qt::green);
            else if (worsts[2].contains (x))
                tps->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 2, tps);
            table->setItem (x + 1, 3, new QTableWidgetItemCopyable {scrambles_vector[x].toQString ()});


            QString date = solves[x].datetime.toString (datetime_format);
            table->setItem (x + 1, 4, makeCenterAlignedItem (date));
        }
    }
    else if (n == 1) //single
    {
        table->setVerticalHeaderItem (0, new QTableWidgetItem {"Single"});
        table->setItem (0, 0, makeCenterAlignedItem (getTimeString (solves.back ().time)));
        table->setItem (0, 1, makeCenterAlignedItem (floatToQString (solves.back ().movecount)));
        table->setItem (0, 2, makeCenterAlignedItem (floatToQString (solves.back ().tps)));
        table->setItem (0, 3, makeLeftAlignedItem (solves.back ().scramble.toQString ()));
        table->setItem (0, 4, makeCenterAlignedItem (solves.back ().datetime.toString (datetime_format)));
    }
    else if (n > 1) //avgs
    {
        const auto& times_slice = times_vector.mid (size - n, n);
        const auto& movecount_slice = movecount_vector.mid (size - n, n);
        const auto& tps_slice = tps_vector.mid (size - n, n);
        const auto& scrambles_slice = scrambles_vector.mid (size - n, n);

        int m = static_cast<int> (qCeil (n * 0.05));

        QVector<int> bests[3] = {getBestN (times_slice, m), getBestN (movecount_slice, m), getWorstN (tps_slice, m)};
        QVector<int> worsts[3] = {getWorstN (times_slice, m), getWorstN (movecount_slice, m), getBestN (tps_slice, m)};

        table->setRowCount (n + 1);
        table->setVerticalHeaderItem (0, new QTableWidgetItem {n == solves.size () ? "Session avg" : "Avg of " + QString::number (n)});
        table->setItem (0, 0, makeColorItem (getTimeString (getAvg (times_vector, n))));
        table->setItem (0, 1, makeColorItem (floatToQString (getAvg (movecount_vector, n))));
        table->setItem (0, 2, makeColorItem (floatToQString (getAvg (tps_vector, n))));
        table->setItem (0, 3, makeColorItem ());
        table->setItem (0, 4, makeColorItem ());

        for (int x = 0; x < n; x++)
        {
            table->setVerticalHeaderItem (x + 1, new QTableWidgetItem {QString::number (x + 1)});

            QTableWidgetItemCopyable *time = makeCenterAlignedItem (getTimeString (times_slice[x]));
            if (bests[0].contains (x))
                time->setBackgroundColor (Qt::green);
            else if (worsts[0].contains (x))
                time->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 0, time);

            QTableWidgetItemCopyable *movecount = makeCenterAlignedItem (floatToQString (movecount_slice[x]));
            if (bests[1].contains (x))
                movecount->setBackgroundColor (Qt::green);
            else if (worsts[1].contains (x))
                movecount->setBackgroundColor (Qt::red);
            table->setItem (x + 1, 1, movecount);

            QTableWidgetItemCopyable *tps = makeCenterAlignedItem (floatToQString (tps_slice[x]));
            if (bests[2].contains (x))
                tps->setBackgroundColor (Qt::green);
            else if (worsts[2].contains (x))
                tps->setBackgroundColor (Qt::red);

            table->setItem (x + 1, 2, tps);
            table->setItem (x + 1, 3, makeLeftAlignedItem (scrambles_slice[x].toQString ()));
            table->setItem (x + 1, 4, makeCenterAlignedItem (solves_slice[x].datetime.toString (datetime_format)));
        }
    }
    height = (height > 600 ? 600 : height);
    table->horizontalHeader ()->setStretchLastSection (true);
    table->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget (table);
    layout->setContentsMargins (0, 0, 0, 0);
    layout->setSpacing (0);

    QWidget *w = new QWidget;
    w->resize (620, height);
    w->setMaximumHeight (600);
    w->setLayout (layout);
    w->show ();
}

/*template <class T>
QVector<T>& ClockSim::getQVector (T Solve::*x, int i)
{
    static QVector<T> vs[5];
    QVector<T>& v = vs[i];
    v.clear ();
    foreach (const Solve& s, solves)
        v.push_back (s.*x);
    return v;
}*/

template <class T>
QVector<T>& ClockSim::getQVector (T Solve::*x)
{
    static QVector<T> v; //need static storage
    v.clear ();
    foreach (const Solve& s, solves)
        v.push_back (s.*x);
    return v;
}


void ClockSim::updateStats ()
{
    int n = solves.size ();
    if (!n)
        return;
    bool pb_broken {false};

    applyToLabels (&QLabel::clear);

    ui->labelNOSolves->setText ("Solves: " + QString::number (n));

    //Single
    ui->labelSingleTime->setText (getTimeString (solves.back ().time));
    ui->labelSingleMovecount->setText (floatToQString (solves.back ().movecount));
    ui->labelSingleTPS->setText (floatToQString (solves.back ().tps));

    //Session mean
    ui->labelSessionMeanTime->setText (getTimeString (std::accumulate (times_vector.begin (), times_vector.end (),0.0) / times_vector.size ()));
    ui->labelSessionMeanMovecount->setText (floatToQString (std::accumulate (movecount_vector.begin (), movecount_vector.end (),0.0) / movecount_vector.size ()));
    ui->labelSessionMeanTPS->setText (floatToQString (std::accumulate (tps_vector.begin (), tps_vector.end (),0.0) / tps_vector.size ()));

    //Session avg
    if (n >= 3)
    {
        ui->labelSessionAvgTime->setText (getTimeString (getAvg (times_vector, times_vector.size ())));
        ui->labelSessionAvgMovecount->setText (floatToQString (getAvg (movecount_vector, movecount_vector.size ())));
        ui->labelSessionAvgTPS->setText (floatToQString (getAvg (tps_vector, tps_vector.size ())));
    }

    //Order: 0 = single, 1 = avg5, 2 = avg12, 3 = avg50, 4 = avg100
    QLabel* time_labels[5] = {ui->labelSingleTime, ui->labelAvg5Time, ui->labelAvg12Time, ui->labelAvg50Time, ui->labelAvg100Time};
    QLabel* movecount_labels[5] = {ui->labelSingleMovecount, ui->labelAvg5Movecount, ui->labelAvg12Movecount, ui->labelAvg50Movecount, ui->labelAvg100Movecount};
    QLabel* tps_labels[5] = {ui->labelSingleTPS, ui->labelAvg5TPS, ui->labelAvg12TPS, ui->labelAvg50TPS, ui->labelAvg100TPS};

    //Labels (0 = time, 1 = movecount, 2 = tps)
    QLabel** labels[3] = {time_labels, movecount_labels, tps_labels};

    QVector<float> vectors[3] = {times_vector, movecount_vector, tps_vector};

    //Stringification functions (0 = time, 1 = movecount, 2 = tps): ClockSim's member functions which take a float and return a QString
    QString ((ClockSim::*functions[3]) (float)) = {&ClockSim::getTimeString, &ClockSim::floatToQString, &ClockSim::floatToQString};

    //Comparison functions (0 = time (best = the lower), 1 = movecount (best = the lower), 2 = tps (best = the higher))
    auto less = [] (float _1, float _2) -> bool {return _1 <= _2;};
    auto greater = [] (float _1, float _2) -> bool {return _1 >= _2;};
    bool ((*compare_functions[3]) (float, float)) = {less, less, greater};

    //for the single
    float Solve::*v[3] {&Solve::time, &Solve::movecount, &Solve::tps};

#define X(N, index)\
    if (n >= N)\
    {\
        float avg = getAvg (vectors[i], N);\
        if (pbs[i].avg##N.avg == FLT_MAX || (*compare_functions[i]) (avg, pbs[i].avg##N.avg))\
        {\
            pbs[i].avg##N.avg = avg;\
            pbs[i].avg##N.solves = solves.mid (n - N, N);\
            pb_broken = true;\
            labels[i][index]->setStyleSheet ("font-weight: bold; color:blue;");\
        }\
        labels[i][index]->setText ((this->*(functions[i])) (avg));\
    }

    for (int i = 0; i < 3; i++)
    {
        if (pbs[i].single.*(v[i]) == FLT_MAX || (compare_functions[i]) (solves.back ().*(v[i]), pbs[i].single.*(v[i])))
        {
            pb_broken = true;
            pbs[i].single = solves.back ();
            labels[i][0]->setStyleSheet ("font-weight: bold; color:blue;");
        }

        X (5, 1);
        X (12, 2);
        X (50, 3);
        X (100, 4);
    }

#undef X

    if (pb_broken)
        savePBs ();
}

/* explaination:
 * http://stackoverflow.com/questions/38878798/pass-function-member-pointer-as-parameter-to-function?noredirect=1#38879072
 */
template <class R, class C, class ...Params, class ...Args>
typename std::enable_if<std::is_base_of<C,QLabel>::value &&
                        sizeof... (Params) == sizeof... (Args)>::type
ClockSim::applyToLabels (R (C::*f) (Params...), Args&& ...args)
{
    constexpr auto size = 21;
    QLabel* labels[size] = {ui->labelSingleTime,
                        ui->labelAvg5Time,
                        ui->labelAvg12Time,
                        ui->labelAvg50Time,
                        ui->labelAvg100Time,

                        ui->labelSingleMovecount,
                        ui->labelAvg5Movecount,
                        ui->labelAvg12Movecount,
                        ui->labelAvg50Movecount,
                        ui->labelAvg100Movecount,

                        ui->labelSingleTPS,
                        ui->labelAvg5TPS,
                        ui->labelAvg12TPS,
                        ui->labelAvg50TPS,
                        ui->labelAvg100TPS,

                        ui->labelSessionAvgTime,
                        ui->labelSessionAvgMovecount,
                        ui->labelSessionAvgTPS,

                        ui->labelSessionMeanTime,
                        ui->labelSessionMeanMovecount,
                        ui->labelSessionMeanTPS};

    for (int i = 0; i < size; i++)
         (labels[i]->*f) (std::forward<Args> (args)...);
}

void ClockSim::paintEvent (QPaintEvent *)
{
    QPainter painter {this};

    painter.translate (450.0/2, 450.0/2);
    painter.rotate (angle);
    painter.translate (-450.0/2, -450.0/2);

    painter.setRenderHint (QPainter::Antialiasing);
    QPen pen {Qt::red, 3, Qt::SolidLine, Qt::RoundCap};

#define GET_COLOR(c) (settings->ui->c->getColor ())

    QColor bgColor, clockColor;

    bgColor = GET_COLOR (backColor);
    clockColor = GET_COLOR (frontColor);
    if (!clock.isFront ())
        std::swap (bgColor, clockColor);

    //Background color
    QPainterPath path;
    path.addRect (0, 0, 450, 450);
    painter.fillPath (path, bgColor);
    painter.drawPath (path);
    painter.setPen (pen);

    auto drawClock = [&painter, &pen, &clockColor, this] (size_t x, size_t y)
    {
        QPainterPath path;
        path.addEllipse (QPointF (x, y), 50, 50);
        QPen pen2 {QBrush {GET_COLOR (clockBorderColor)}, 1};
        painter.setPen (pen2);
        painter.fillPath (path, clockColor);
        painter.drawPath (path);
        painter.setPen (pen);
    };


    //Clocks
    QVector<hour_t> hours = {clock.corners[0].clock.hour, clock.edges[0].front.hour, clock.corners[1].clock.hour,
                                 clock.edges[1].front.hour, clock.center.front.hour, clock.edges[2].front.hour,
                                clock.corners[2].clock.hour, clock.edges[3].front.hour, clock.corners[3].clock.hour};

    for (int i = 0; i < 9; i++)
    {
        int clock_x = i % 3;
        int clock_y = i / 3;
        int center_x = 50 + clock_x * 150 + 25;
        int center_y = 50 + clock_y * 150 + 25;

        //Sign "12" dot: yellow dot with radius 6
        painter.setPen (QPen {QBrush {GET_COLOR (dot12Color)}, 12, Qt::SolidLine, Qt::RoundCap});
        painter.drawPoint (center_x , center_y - 58);

        //Sign "12": Arial 7
        painter.setPen (QPen {GET_COLOR (sign12Color)});
        QFont sign12Font {"Arial"};
        sign12Font.setPixelSize (7);
        painter.setFont (sign12Font);
        painter.drawText (center_x - 4, center_y - 58 + 3, "12");

        painter.setPen (pen);
        if (!settings->ui->checkBoxDrawDots->isChecked ())
            goto SKIP_DRAW_DOTS;
        painter.save ();

        painter.setPen (QPen {QBrush {GET_COLOR (dotsColor)}, 5, Qt::SolidLine, Qt::RoundCap});

        for (double angle = 30.0; angle < 360.0; angle += 30.0)
        {
            double line_y = qCos (qDegreesToRadians (angle));
            double line_x = qSin (qDegreesToRadians (angle));

            line_x *= 1.2;
            line_y *= 1.2;
            painter.drawPoint (center_x + line_x * 50, center_y - line_y * 50);
        }

        painter.restore ();

        SKIP_DRAW_DOTS:

        drawClock (center_x, center_y);

        int hour = hours[i];

        painter.save ();
        painter.translate (QPointF (center_x, center_y));
        painter.rotate (hour * 30);
        painter.setPen (QPen {QBrush {GET_COLOR (clockHandBorderColor)}, 3, Qt::SolidLine, Qt::RoundCap});

        int clockhand_style = settings->ui->comboBoxClockHandStyle->currentIndex ();
        if (clockhand_style == 0) //Rubik's style
        {
            const int dim = 10;
            QRect rectangle {QPoint {-dim, -dim}, QPoint {dim - 1, dim}};
            QPainterPath pointer;
            pointer.moveTo (QPointF (dim, 0.0));
            pointer.arcTo (rectangle, 0, -180);
            pointer.lineTo (QPointF (0.0, -40.0));
            pointer.closeSubpath ();

            painter.fillPath (pointer, QBrush {GET_COLOR (clockHandColor)});
            painter.drawPath (pointer);

        }
        else //Simple line
        {
            painter.drawLine (QPointF {0.0, 0.0}, QPointF {0.0, -45.0});
        }

        painter.restore ();
    }

    //Pins
    painter.setPen ({Qt::black, 1});

    for (int i = 0; i < 4; i++)
    {
        int pos_x = i % 2;
        int pos_y = i / 2;
        double x = 150 + pos_x * 150;
        double y = 150 + pos_y * 150;

        QPainterPath path;
        path.addEllipse (QPointF {x, y}, 15, 15);

        QColor pin_color {clock.pins[i].isUp () ? settings->ui->pinUpColor->getColor () : settings->ui->pinDownColor->getColor ()};


        painter.fillPath (path, pin_color);
        painter.drawPath (path);
    }
#undef GET_COLOR
}
