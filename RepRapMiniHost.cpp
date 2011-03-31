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
 *
 * The RepRapMiniHost is a control and g-code transmitter for 3D printers
 * (and maybe other g-code style machines), special developed for the
 * RepRap project (www.reprap.org).
 * The functions of this host are similar to the original host software
 * from RepRap with som differences:
 *   - much more stable than the original host
 *   - wirtten in C++ (with Qt and boost), not Java
 *   - no support for generation of g-code
 * The RepRapHost class together with the BoostComPort class can be extracted
 * from this project to be used in other projects. The GUI is complete seperated 
 * from the pure control. Those two classes only needs C++ standard lib and the
 * boost header only lib.
 * For more information see the README file provided with this sources.
 * 
 * TODO: Aktualisierung der Positionen im Fenster durch Dateien
 */

#include "RepRapMiniHost.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

RepRapMiniHost::RepRapMiniHost(QWidget *parent)
    : QMainWindow(parent),
      steps(1.0),
      x(0.0),
      y(0.0),
      z(0.0),
      f(0.0),
      remainingTimeCounter(0),
      boardAnswerTimout(5000),
      tempReadTime(2000),
      autoRefreshTemperatures(true),
      extrudeWhenMoving(false),
      targetTempExtruder(230.0),
      targetTempBed(120.0),
      debug(true),
      autoOpenPort(false)
{
	ui.setupUi(this);
	
	repRapHost.setHashEnabled(false);
	
	statusBar = new QStatusBar();
	this->setStatusBar(statusBar);
	
	tempTimer = new QTimer(this);
	connect(tempTimer, SIGNAL(timeout()), this, SLOT(onTempTimer()));
	tempTimer->setInterval(tempReadTime);
	
	tickTimer = new QTimer(this);
	connect(tickTimer, SIGNAL(timeout()), this, SLOT(onTickTimer()));
	tickTimer->setInterval(10);
	
	remainingTimeTimer = new QTimer(this);
	connect(remainingTimeTimer, SIGNAL(timeout()), this, SLOT(onRemainingTimeTimer()));
	remainingTimeTimer->setInterval(1000);

	restoreValues();
	
	if(autoRefreshTemperatures)
		tempTimer->start();
	tickTimer->start();
	remainingTimeTimer->start();
	if(autoOpenPort)
		onButtonCom();
}
/*
 * Maybe the above created instances of timers should be deleted
 * here, maybe Qt does this itself, no idea...
 * TODO: Find out if it is necessary to delete the timers in destructor
 */
RepRapMiniHost::~RepRapMiniHost()
{
	storeValues();
}

void RepRapMiniHost::restoreValues()
{
	if(debug)
		cout<<"Restoring values..."<<endl;
	
	QCoreApplication::setOrganizationName("RepRap");
	QCoreApplication::setOrganizationDomain("reprap.org");
	QCoreApplication::setApplicationName("RepRap Minihost");
	QSettings settings;
	
	debug = settings.value("debug", debug).toBool();
	ui.checkDebugging->setChecked(debug);
	repRapHost.setDebug(debug);
	
	extrudeWhenMoving=settings.value("extrudeWhenMoving", extrudeWhenMoving).toBool();
	ui.checkExtrudeWhenMoving->setChecked(extrudeWhenMoving);
	
	autoRefreshTemperatures=settings.value("autoRefreshTemperatures", autoRefreshTemperatures).toBool();
	ui.checkAutoRefreshTemperatures->setChecked(autoRefreshTemperatures);
	
	bool enableHashes=settings.value("enableHashes", false).toBool();
	ui.checkEnableHashes->setChecked(enableHashes);
	
	ui.editComPort->setText(settings.value("comPort", "/dev/ttyUSB0").toString());
	ui.editComBaud->setText(settings.value("comBaud", "19200").toString());
	
	bool ok;
	steps=settings.value("steps", 1.0).toDouble(&ok);
	if(!ok)
		steps=1.0;
	if(steps<0.2)
	{
		ui.radio01->setChecked(true);
		ui.radio1->setChecked(false);
		ui.radio10->setChecked(false);
	}
	else if(steps<1.1)
	{
		ui.radio01->setChecked(false);
		ui.radio1->setChecked(true);
		ui.radio10->setChecked(false);
	}
	else if(steps<10.1)
	{
		ui.radio01->setChecked(false);
		ui.radio1->setChecked(false);
		ui.radio10->setChecked(true);
	}
	
	f=settings.value("f", 1000.0).toDouble(&ok);
	if(!ok)
		f=1000.0;
	ui.editF->setText(QString::number(f));
	
	targetTempBed=settings.value("targetTempBed", 110.0).toDouble(&ok);
	if(!ok)
		targetTempBed=110.0;
	ui.editTempBed->setText(QString::number(targetTempBed));

	targetTempExtruder=settings.value("targetTempExtruder", 110.0).toDouble(&ok);
	if(!ok)
		targetTempExtruder=210.0;
	ui.editTempExtruder->setText(QString::number(targetTempExtruder));
	
	ui.editFile->setText(settings.value("fileName", "").toString());
	
	autoOpenPort=settings.value("autoOpenPort", false).toBool();
	ui.checkAutoOpenPort->setChecked(autoOpenPort);
}

void RepRapMiniHost::storeValues()
{
	if(debug)
		cout<<"Storing values..."<<endl;
	
	QCoreApplication::setOrganizationName("RepRap");
	QCoreApplication::setOrganizationDomain("reprap.org");
	QCoreApplication::setApplicationName("RepRap Minihost");
	QSettings settings;
	
	settings.setValue("debug", debug);
	settings.setValue("extrudeWhenMoving", extrudeWhenMoving);
	settings.setValue("autoRefreshTemperatures", autoRefreshTemperatures);
	settings.setValue("enableHashes", ui.checkEnableHashes->isChecked());
	settings.setValue("comPort", ui.editComPort->text());
	settings.setValue("comBaud", ui.editComBaud->text());
	settings.setValue("steps", steps);
	settings.setValue("f", ui.editF->text());
	settings.setValue("targetTempBed", targetTempBed);
	settings.setValue("targetTempExtruder", targetTempExtruder);
	settings.setValue("fileName", ui.editFile->text());
	settings.setValue("autoOpenPort", autoOpenPort);
}

/*
 * Add every "tempReadTime" milliseconds a new temperature read
 * command at the top of the command stack.
 */
void RepRapMiniHost::onTempTimer()
{
	if(!repRapHost.isConnected())
		return;
	// TODO: Check if the last added command was already a temperature command, then do not add it
	repRapHost.addCommand("M105", false);
	ui.labelTempExtruder->setText(QString::number(repRapHost.getTempExtruder()));
	ui.labelTempBed->setText(QString::number(repRapHost.getTempBed()));
}

void RepRapMiniHost::onTickTimer()
{
	repRapHost.timerTick();
}

void RepRapMiniHost::onRemainingTimeTimer()
{
	remainingTimeCounter++;
	if(remainingTimeCounter>30)
	{
		repRapHost.refreshRemainingTime();
		remainingTimeCounter=0;
	}
	int remainingTime=(int)repRapHost.getRemainingTime(); // we don't need millisecond precision for a displayed value ;)
	int seconds=remainingTime%60;
	int minutes=(remainingTime/60)%60;
	int hours=(remainingTime/3600)%60;
	QString strSeconds=QString::number(seconds);
	if(seconds<10)
		strSeconds=tr("0")+strSeconds;
	QString strMinutes=QString::number(minutes);
	if(minutes<10)
		strMinutes=tr("0")+strMinutes;
	QString strHours=QString::number(hours);
	if(hours<10)
		strHours=tr("0")+strHours;
	ui.labelLeft->setText(tr("Left: ")+strHours+tr(":")+strMinutes+tr(":")+strSeconds);
}

void RepRapMiniHost::onButtonCom()
{
	if(ui.buttonCom->text()=="Open")
	{
		bool ok=true;
		if(repRapHost.connect(ui.editComPort->text().toStdString(), ui.editComBaud->text().toInt(&ok, 10)))
		{
			if(!ok)
				QMessageBox::critical(this, "Fatal error opening com port", "Unable to open the com port: The baud rate seems to be no number!");
			else
				QMessageBox::critical(this, "Fatal error opening com port", "Unable to open the com port!");
			return;
		}
		ui.buttonCom->setText("Close");
	}
	else
	{
		repRapHost.disconnect();
		ui.buttonCom->setText("Open");
	}
}

void RepRapMiniHost::onRadio()
{
	if(ui.radio01->isChecked())
		steps=0.1;
	else if(ui.radio1->isChecked())
		steps=1.0;
	else if(ui.radio10->isChecked())
		steps=10.0;
}

void RepRapMiniHost::onButtonXPlus()
{
	addPos(steps, 0.0, 0.0, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonYPlus()
{
	addPos(0.0, steps, 0.0, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonZPlus()
{
	addPos(0.0, 0.0, steps, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonEPlus()
{
	addPos(0.0, 0.0, 0.0, steps);
}

void RepRapMiniHost::onButtonXMinus()
{
	addPos(-steps, 0.0, 0.0, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonYMinus()
{
	addPos(0.0, -steps, 0.0, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonZMinus()
{
	addPos(0.0, 0.0, -steps, 0.0, extrudeWhenMoving);
}

void RepRapMiniHost::onButtonEMinus()
{
	addPos(0.0, 0.0, 0.0, -steps);
}

void RepRapMiniHost::onButtonGo()
{
	if(!repRapHost.isConnected())
		return;
	float dx=x;
	float dy=y;
	float dz=z;
	if(!getXYZF())
		return;
	dx-=x;
	dy-=y;
	dz-=z;
	double e=0.0;
	if(extrudeWhenMoving)
		e=sqrt(dx*dx+dy*dy+dz*dz);
	repRapHost.addCommand(string("G1 X")+repRapHost.double2String(x) + " Y"+repRapHost.double2String(y) + " Z"+repRapHost.double2String(z) + " F"+repRapHost.double2String(f) + " E"+repRapHost.double2String(e));
}

int RepRapMiniHost::getXYZF()
{
	bool ok;
	x=ui.editX->text().toFloat(&ok);
	if(!ok)
	{
		QMessageBox::critical(this, "Fatal error moving the RepRap", "The X value seems to be no float value, use . instead of ,");
		return 0;
	}
	y=ui.editY->text().toFloat(&ok);
	if(!ok)
	{
		QMessageBox::critical(this, "Fatal error moving the RepRap", "The Y value seems to be no float value, use . instead of ,");
		return 0;
	}
	z=ui.editZ->text().toFloat(&ok);
	if(!ok)
	{
		QMessageBox::critical(this, "Fatal error moving the RepRap", "The Z value seems to be no float value, use . instead of ,");
		return 0;
	}
	f=ui.editF->text().toFloat(&ok);
	if(!ok)
	{
		QMessageBox::critical(this, "Fatal error moving the RepRap", "The F value seems to be no float value, use . instead of ,");
		return 0;
	}
	return 1;
}

void RepRapMiniHost::setXYZ()
{
	ui.editX->setText(QString::number(x));
	ui.editY->setText(QString::number(y));
	ui.editZ->setText(QString::number(z));
}

void RepRapMiniHost::addPos(float dx, float dy, float dz, float de, bool autoCalcde)
{
	if(!repRapHost.isConnected())
		return;
	if(!getXYZF())
		return;
	x+=dx;
	y+=dy;
	z+=dz;
	if(autoCalcde)
		de=sqrt(dx*dx+dy*dy+dz*dz);
	setXYZ();
	repRapHost.addCommand(string("G1 X")+repRapHost.double2String(x) + string(" Y")+repRapHost.double2String(y) + string(" Z")+repRapHost.double2String(z) + string(" F")+repRapHost.double2String(f) + string(" E")+repRapHost.double2String(de));
}

void RepRapMiniHost::onButtonHomeX()
{
	repRapHost.addCommand("G28 X0");
	x=0.0;
	ui.editX->setText(tr("0"));
}

void RepRapMiniHost::onButtonHomeY()
{
	repRapHost.addCommand("G28 Y0");
	y=0.0;
	ui.editY->setText(tr("0"));
}

void RepRapMiniHost::onButtonHomeZ()
{
	repRapHost.addCommand("G28 Z0");
	z=0.0;
	ui.editZ->setText(tr("0"));
}

void RepRapMiniHost::onButtonHomeAll()
{
	repRapHost.addCommand("G28");
	ui.editX->setText(tr("0"));
	ui.editY->setText(tr("0"));
	ui.editZ->setText(tr("0"));
}

void RepRapMiniHost::onCheckExtrudeWhenMoving(int status)
{
	if(status==Qt::Checked)
		extrudeWhenMoving=true;
	else
		extrudeWhenMoving=false;
}

void RepRapMiniHost::onCheckAutoRefreshTemperatures(int status)
{
	if(status==Qt::Checked)
	{
		autoRefreshTemperatures=true;
		tempTimer->start();
	}
	else
	{
		autoRefreshTemperatures=false;
		tempTimer->stop();
	}
}

void RepRapMiniHost::onButtonResetHashCounter()
{
	repRapHost.setNextLineNumber(0);
	if(!repRapHost.getHashEnabled())
		cout<<"You have disabled hashes and you want me to reset the hash counter, that makes no sense but I will do what you told me to do."<<endl;
	if(debug)
		cout<<"Reset the hash counter"<<endl;
}

void RepRapMiniHost::onCheckEnableHashes(int status)
{
	if(status==Qt::Checked)
	{
		repRapHost.setHashEnabled(true);
		if(debug)
			cout<<"Turned on hashes"<<endl;
	}
	else
	{
		repRapHost.setHashEnabled(false);
		if(debug)
			cout<<"Turned off hashes"<<endl;
	}
	if(debug)
		cout<<"Changes will take effect on every new command in the queue"<<endl;
}

void RepRapMiniHost::onButtonTempExtruder()
{
	if(!repRapHost.isConnected())
		return;
	if(ui.buttonTempExtruder->text()==tr("Turn on"))
	{
		repRapHost.addCommand(string("M104 S")+repRapHost.double2String(targetTempExtruder));
		ui.buttonTempExtruder->setText(tr("Turn off"));
	}
	else
	{
		repRapHost.addCommand("M104 S0");
		ui.buttonTempExtruder->setText(tr("Turn on"));
	}
}

void RepRapMiniHost::editTempExtruderChanged(QString value)
{
	bool ok=true;
	float fValue=value.toFloat(&ok);
	if(!ok)
	{
		cout<<"The value in the edit box for extruder temperature is not a number!"<<endl;
		statusBar->showMessage("The value in the edit box for extruder temperature is not a number!", 4000);
		return;
	}
	targetTempExtruder=fValue;
	if(targetTempExtruder>300)
	{
		cout<<(string("WARNING: Are you shure you want to heat your nozzle to ")+repRapHost.double2String(targetTempExtruder)+" degrees???")<<endl;
		statusBar->showMessage(tr("WARNING: Are you shure you want to heat your nozzle to ")+QString::number(targetTempExtruder)+tr(" degrees???"), 4000);
	}
	if(ui.buttonTempExtruder->text()==tr("Turn off"))
		repRapHost.addCommand(string("M104 S")+repRapHost.int2String(targetTempExtruder));
}

void RepRapMiniHost::onButtonTempBed()
{
	if(!repRapHost.isConnected())
		return;
	if(ui.buttonTempBed->text()==tr("Turn on"))
	{
		repRapHost.addCommand(string("M140 S")+repRapHost.double2String(targetTempBed));
		ui.buttonTempBed->setText(tr("Turn off"));
	}
	else
	{
		repRapHost.addCommand("M140 S0");
		ui.buttonTempBed->setText(tr("Turn on"));
	}
}

void RepRapMiniHost::editTempBedChanged(QString value)
{
	bool ok=true;
	float fValue=value.toFloat(&ok);
	if(!ok)
	{
		cout<<"The value in the edit box for bed temperature is not a number!"<<endl;
		statusBar->showMessage("The value in the edit box for bed temperature is not a number!", 4000);
		return;
	}
	targetTempBed=fValue;
	if(targetTempBed>150)
	{
		cout<<(string("WARNING: Are you shure you want to heat your bed to ")+repRapHost.double2String(targetTempBed)+" degrees???")<<endl;
		statusBar->showMessage(tr("WARNING: Are you shure you want to heat your bed to ")+QString::number(targetTempBed)+tr(" degrees???"), 4000);
	}
	if(ui.buttonTempBed->text()==tr("Turn off"))
		repRapHost.addCommand(string("M104 S")+repRapHost.int2String(targetTempBed));
}

void RepRapMiniHost::onButtonBrowse()
{
	QString fileName=QFileDialog::getOpenFileName(this, "G-Code File to execute", ".", "*.*");
	if(fileName=="")
		return;
	ui.editFile->setText(fileName);
}

void RepRapMiniHost::onButtonExecute()
{
	QFile gCodeFile(ui.editFile->text());
	if(!gCodeFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		statusBar->showMessage(tr("Unable to open file ")+ui.editFile->text()+": No such file or directory", 4000);
		cout<<"Unable to open file "<<ui.editFile->text().toStdString()<<": No such file or directory"<<endl;
		return;
	}
	QTextStream stream(&gCodeFile);
	QString line=stream.readLine(0);
	while(!line.isNull())
	{
		if(line.isEmpty())
		{
			line=stream.readLine(0);
			continue;
		}
		// TODO: Remove comments
		int commentPos=line.indexOf(";");
		if(commentPos>=0)
		{
			line=line.left(commentPos);
			while(line.length() && (line[line.length()-1]==QChar(' ') || line[line.length()-1]==QChar('\t')))
				line=line.left(line.length()-1);
			if(line.length()<=2)
			{
				line=stream.readLine(0);
				continue;
			}
		}
		repRapHost.addCommand(line.toStdString());
		line=stream.readLine(0);
	}
	if(debug)
		cout<<"reading file finished..."<<endl;
}

void RepRapMiniHost::onButtonStop()
{
	repRapHost.clear();
}

void RepRapMiniHost::onCheckDebugging(int status)
{
	if(status==Qt::Checked)
	{
		debug=true;
		repRapHost.setDebug(true);
	}
	else
	{
		debug=false;
		repRapHost.setDebug(false);
	}
}

void RepRapMiniHost::onCheckAutoOpenPort(int value)
{
	if(value==Qt::Checked)
	{
		autoOpenPort=true;
	}
	else
	{
		autoOpenPort=false;
	}
}

