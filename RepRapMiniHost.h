#ifndef REPRAPMINIHOST_H
#define REPRAPMINIHOST_H

#include <QtGui/QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QTime>
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
};

#endif // REPRAPMINIHOST_H
