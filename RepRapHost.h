/*
 * RepRapHost.h
 *
 *  Created on: 18.02.2011
 *      Author: fabian
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
