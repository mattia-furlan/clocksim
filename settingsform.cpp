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
#include "settingsform.h"
#include "ui_settingsform.h"

SettingsForm::SettingsForm (QWidget *parent) : ui {new Ui::SettingsForm}
{
    ui->setupUi (this);

    setDefaultControls ();
    setDefaultColors ();

    //Controls
    wheels_controls[ui->key1m] = -1;
    wheels_controls[ui->key2m] = -2;
    wheels_controls[ui->key3m] = -3;
    wheels_controls[ui->key4m] = -4;
    wheels_controls[ui->key5m] = -5;
    wheels_controls[ui->key1p] = 1;
    wheels_controls[ui->key2p] = 2;
    wheels_controls[ui->key3p] = 3;
    wheels_controls[ui->key4p] = 4;
    wheels_controls[ui->key5p] = 5;
    wheels_controls[ui->key6] = 6;

    pins_controls[ui->keyUL] = 0;
    pins_controls[ui->keyUR] = 1;
    pins_controls[ui->keyDL] = 2;
    pins_controls[ui->keyDR] = 3;

#define X(k) connect (k, SIGNAL (changed ()), this, SLOT (checkKeyConflicts ()))
    X (ui->key1m); X (ui->key2m); X (ui->key3m); X (ui->key4m); X (ui->key5m);
    X (ui->key1p); X (ui->key2p); X (ui->key3p); X (ui->key4p); X (ui->key5p); X (ui->key6);
    X (ui->key_scramble); X (ui->key_y2); X (ui->key_x2); X (ui->key_z); X (ui->key_zp); X (ui->key_z2);
#undef X

    connect (ui->pushButtonSetDefaultControls, SIGNAL (clicked ()), this, SLOT (setDefaultControls ()));
    connect (ui->pushButtonSetDefaultColors, SIGNAL (clicked ()), this, SLOT (setDefaultColors ()));

    connect (ui->pinDownColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->pinUpColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->frontColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->backColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->clockHandColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->dot12Color, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->sign12Color, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->dotsColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->clockBorderColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->clockHandBorderColor, SIGNAL (changed ()), parent, SLOT (update ()));
    connect (ui->comboBoxClockHandStyle, SIGNAL (currentIndexChanged (int)), parent, SLOT (update ()));
    connect (ui->checkBoxDrawDots, SIGNAL (stateChanged (int)), parent, SLOT (update ()));

    connect (ui->radioButton1_10, SIGNAL (toggled (bool)), parent, SLOT (updateStats ()));
    connect (ui->radioButton1_100, SIGNAL (toggled (bool)), parent, SLOT (updateStats ()));
    connect (ui->radioButton1_1000, SIGNAL (toggled (bool)), parent, SLOT (updateStats ()));

    connect (ui->pushButtonLoadColorsFromFile, SIGNAL (clicked ()), this, SLOT (loadColors ()));
    connect (ui->pushButtonSaveColorsToFile, SIGNAL (clicked ()), this, SLOT (saveColors ()));
    connect (ui->pushButtonLoadControlsFromFile, SIGNAL (clicked ()), this, SLOT (loadControls ()));
    connect (ui->pushButtonSaveControlsToFile, SIGNAL (clicked ()), this, SLOT (saveControls ()));
}

SettingsForm::~SettingsForm ()
{
    delete ui;
}


int SettingsForm::getTimerPrecision ()
{
    if (ui->radioButton1_10->isChecked ())
        return 1;
    else if (ui->radioButton1_100->isChecked ())
        return 2;
    return 3;
}

void SettingsForm::checkKeyConflicts ()
{
    QMap<QKeySequence, uint> keys;

#define X(k)\
    if (!ui->k->getKey ().isEmpty ())\
        keys[ui->k->getKey ()]++;

    foreach (QLineEditHotKey* l, wheels_controls.keys ())
        if (!l->getKey ().isEmpty ())
            keys[l->getKey ()]++;

    foreach (QLineEditHotKey* l, pins_controls.keys ())
        if (!l->getKey ().isEmpty ())
            keys[l->getKey ()]++;

    X (key_scramble);
    X (key_y2);
    X (key_x2);
    X (key_z);
    X (key_zp);
    X (key_z2);

#undef X

    foreach (const QKeySequence& seq, keys.keys ())
        if (keys.value (seq) > 1)
        {
            ui->labelKeyConflict->setText ("Key conflict detected");
            return;
        }
    ui->labelKeyConflict->clear ();
}

void SettingsForm::setDefaultControls ()
{
    ui->key1m->setDefaultKey (Qt::Key_J);
    ui->key2m->setDefaultKey ({});
    ui->key3m->setDefaultKey ({});
    ui->key4m->setDefaultKey ({});
    ui->key5m->setDefaultKey ({});

    ui->key1p->setDefaultKey ({Qt::Key_K});
    ui->key2p->setDefaultKey ({});
    ui->key3p->setDefaultKey ({});
    ui->key4p->setDefaultKey ({});
    ui->key5p->setDefaultKey ({});
    ui->key6->setDefaultKey ({Qt::Key_L});

    ui->keyUL->setDefaultKey ({Qt::Key_Q});
    ui->keyUR->setDefaultKey ({Qt::Key_W});
    ui->keyDL->setDefaultKey ({Qt::Key_A});
    ui->keyDR->setDefaultKey ({Qt::Key_S});

    ui->key_scramble->setDefaultKey ({Qt::Key_G});
    ui->key_y2->setDefaultKey ({Qt::Key_Space});
    ui->key_x2->setDefaultKey ({Qt::Key_B});
    ui->key_z->setDefaultKey ({Qt::Key_V});
    ui->key_zp->setDefaultKey ({});
    ui->key_z2->setDefaultKey ({});
}

void SettingsForm::setDefaultColors ()
{
    ui->pinUpColor->setDefaultColor ({"#eae819"});
    ui->pinDownColor->setDefaultColor ({"#98973f"});
    ui->frontColor->setDefaultColor ({"#00ffff"});
    ui->backColor->setDefaultColor ({"#1f83ff"});

    ui->clockHandColor->setDefaultColor ({Qt::yellow});
    ui->sign12Color->setDefaultColor ({Qt::red});
    ui->dot12Color->setDefaultColor ({Qt::yellow});
    ui->dotsColor->setDefaultColor ({Qt::white});

    ui->clockBorderColor->setDefaultColor ({Qt::black});
    ui->clockHandBorderColor->setDefaultColor ({Qt::red});
}

void SettingsForm::saveColors ()
{
    QString filename = QFileDialog::getSaveFileName (this, "Save color scheme", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation));
    if (filename.isEmpty ())
        return;
    QFile file {filename};
    if (!file.open (QIODevice::WriteOnly))
        return;

    QDataStream in {&file};
    in << ui->pinUpColor->getColor ();
    in << ui->pinDownColor->getColor ();
    in << ui->frontColor->getColor ();
    in << ui->backColor->getColor ();
    in << ui->dotsColor->getColor ();
    in << ui->clockHandColor->getColor ();
    in << ui->dot12Color->getColor ();
    in << ui->sign12Color->getColor ();
    in << ui->clockBorderColor->getColor ();
    in << ui->clockHandBorderColor->getColor ();

    file.close ();
}

void SettingsForm::loadColors ()
{
    QString filename = QFileDialog::getOpenFileName (this, "Load color scheme", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation));
    if (filename.isEmpty ())
        return;
    QFile file {filename};
    if (!file.open (QIODevice::ReadOnly))
        return;

    QDataStream out {&file};
    QColor buffer;
    out >> buffer; ui->pinUpColor->setDefaultColor (buffer);
    out >> buffer; ui->pinDownColor->setDefaultColor (buffer);
    out >> buffer; ui->frontColor->setDefaultColor (buffer);
    out >> buffer; ui->backColor->setDefaultColor (buffer);
    out >> buffer; ui->dotsColor->setDefaultColor (buffer);
    out >> buffer; ui->clockHandColor->setDefaultColor (buffer);
    out >> buffer; ui->dot12Color->setDefaultColor (buffer);
    out >> buffer; ui->sign12Color->setDefaultColor (buffer);
    out >> buffer; ui->clockBorderColor->setDefaultColor (buffer);
    out >> buffer; ui->clockHandBorderColor->setDefaultColor (buffer);

    file.close ();
}

void SettingsForm::saveControls ()
{
    QString filename = QFileDialog::getSaveFileName (this, "Save key scheme", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation));
    if (filename.isEmpty ())
        return;
    QFile file {filename};
    if (!file.open (QIODevice::WriteOnly))
        return;

    QDataStream in {&file};
    in << ui->key1p->getKey ();
    in << ui->key2p->getKey ();
    in << ui->key3p->getKey ();
    in << ui->key4p->getKey ();
    in << ui->key5p->getKey ();
    in << ui->key6->getKey ();
    in << ui->key1m->getKey ();
    in << ui->key2m->getKey ();
    in << ui->key3m->getKey ();
    in << ui->key4m->getKey ();
    in << ui->key5m->getKey ();
    in << ui->keyUL->getKey ();
    in << ui->keyUR->getKey ();
    in << ui->keyDL->getKey ();
    in << ui->keyDR->getKey ();
    in << ui->key_scramble->getKey ();
    in << ui->key_y2->getKey ();
    in << ui->key_x2->getKey ();
    in << ui->key_z->getKey ();
    in << ui->key_zp->getKey ();
    in << ui->key_z2->getKey ();

    file.close ();
}

void SettingsForm::loadControls ()
{
    QString filename = QFileDialog::getOpenFileName (this, "Load key scheme", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation));
    if (filename.isEmpty ())
        return;
    QFile file {filename};
    if (!file.open (QIODevice::ReadOnly))
        return;

    QDataStream out {&file};
    QKeySequence buffer;
    out >> buffer; ui->key1p->setDefaultKey (buffer);
    out >> buffer; ui->key2p->setDefaultKey (buffer);
    out >> buffer; ui->key3p->setDefaultKey (buffer);
    out >> buffer; ui->key4p->setDefaultKey (buffer);
    out >> buffer; ui->key5p->setDefaultKey (buffer);
    out >> buffer; ui->key6->setDefaultKey (buffer);
    out >> buffer; ui->key1m->setDefaultKey (buffer);
    out >> buffer; ui->key2m->setDefaultKey (buffer);
    out >> buffer; ui->key3m->setDefaultKey (buffer);
    out >> buffer; ui->key4m->setDefaultKey (buffer);
    out >> buffer; ui->key5m->setDefaultKey (buffer);
    out >> buffer; ui->keyUL->setDefaultKey (buffer);
    out >> buffer; ui->keyUR->setDefaultKey (buffer);
    out >> buffer; ui->keyDL->setDefaultKey (buffer);
    out >> buffer; ui->keyDR->setDefaultKey (buffer);
    out >> buffer; ui->key_scramble->setDefaultKey (buffer);
    out >> buffer; ui->key_y2->setDefaultKey (buffer);
    out >> buffer; ui->key_x2->setDefaultKey (buffer);
    out >> buffer; ui->key_z->setDefaultKey (buffer);
    out >> buffer; ui->key_zp->setDefaultKey (buffer);
    out >> buffer; ui->key_z2->setDefaultKey (buffer);

    file.close ();
}
