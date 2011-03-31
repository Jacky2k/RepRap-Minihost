/*
 * This file is part of RepRap Minihost.
 *
 * RepRap Minihost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RepRap Minihost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REPRAPMINIHOST_H
#define REPRAPMINIHOST_H

#include <QtGui/QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <vector>
#include <string>
#include <iostream>
#include "ui_RepRapMiniHost.h"
#include "BoostComPort.hpp"
#include "RepRapHost.h"

using namespace std;

struct OldCommand
{
	QString command;
	double time;
};

class RepRapMiniHost : public QMainWindow
{
    Q_OBJECT

public:
    RepRapMiniHost(QWidget *parent = 0);
    ~RepRapMiniHost();

protected:
    void restoreValues();
    void storeValues();
    void readPos();
    void addPos(float dx, float dy, float dz, float de, bool autoCalcde=false);
    void setXYZ();
    int getXYZF();
    
    double steps;
    RepRapHost repRapHost;
    double x,y,z,f;
    QStatusBar* statusBar;
    QTimer* tempTimer; // Timer for adding temperature read commands
    QTimer* tickTimer; // Timer for the RepRapHost class to poll the I/O
    QTimer* remainingTimeTimer;
    int remainingTimeCounter;
    
    //configuration
    int boardAnswerTimout; // timeout for answer of the board in milliseconds, 0=no timeout
    int tempReadTime;   // time in milliseconds between each temperature read
    bool autoRefreshTemperatures;
    bool extrudeWhenMoving;
    float targetTempExtruder;
    float targetTempBed;
    bool debug;
    bool autoOpenPort;
    
private:
    Ui::RepRapMiniHostClass ui;
   
private slots:
	void onTempTimer();
	void onTickTimer();
	void onRemainingTimeTimer();
	void onButtonCom();
	void onRadio();
	void onButtonXPlus();
	void onButtonYPlus();
	void onButtonZPlus();
	void onButtonEPlus();
	void onButtonXMinus();
	void onButtonYMinus();
	void onButtonZMinus();
	void onButtonEMinus();
	void onButtonGo();
	void onButtonHomeX();
	void onButtonHomeY();
	void onButtonHomeZ();
	void onButtonHomeAll();
	void onButtonBrowse();
	void onButtonExecute();
	void onButtonStop();
	void onCheckDebugging(int status);
	void onCheckExtrudeWhenMoving(int status);
	void onCheckAutoRefreshTemperatures(int status);
	void onButtonResetHashCounter();
	void onCheckEnableHashes(int status);
	void editTempExtruderChanged(QString value);
	void onButtonTempExtruder();
	void onButtonTempBed();
	void editTempBedChanged(QString value);
	void onCheckAutoOpenPort(int value);
};

#endif // REPRAPMINIHOST_H
