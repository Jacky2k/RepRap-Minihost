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

#ifndef REPRAPHOST_H_
#define REPRAPHOST_H_

#include <string>
#include <vector>
#include "BoostComPort.hpp"
#include <boost/regex.hpp>

using namespace std;

struct Command
{
	string command;
	double time;
	int m;
	int g;
};

enum ComStatus
{
	STANDBY=0,
	WAITING_FOR_OK,
	WAITING_FOR_TEMP
};

class RepRapHost {
public:
	RepRapHost();
	virtual ~RepRapHost();
	
	void setDebug(bool debug);
	int connect(string port, int baud);
	int disconnect();
	bool isConnected();
	void clear();
	
	void setNextLineNumber(int nextLineNumber);
	void refreshRemainingTime();
	double getRemainingTime();
	Command* addCommand(string command, bool putAtEnd=true);
	//Command* addCommandG1(double x, double y, double z, double f, bool extrude=true);
	
	void setXYZF(double x, double y, double z, double f, double e=0.0);
	double getX();
	double getY();
	double getZ();
	double getF();
	
	double getTempExtruder();
	double getTempBed();
	
	void timerTick(); // This function must be called frequently
	
	void setHashEnabled(bool enable);
	bool getHashEnabled();
	string getHash(string cmd);
	
	string toLower(string str);
	string toUpper(string str);
	string int2String(int value);
	string double2String(double value);
	double string2Double(string value);
	
protected:
    ComStatus comStatus;
    BoostComPort comPort;
	vector<Command> commands;
	double remainingTime;
	
	double tempExtruder;
	double tempBed;
	boost::regex tempExpression;
	
	int nextLineNumber;
	double x,y,z,f,e;
	// configuration
	bool hashEnabled;
	bool debug;
};

#endif /* REPRAPHOST_H_ */
