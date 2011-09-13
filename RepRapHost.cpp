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
 * along with RepRap Minihost.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * The RepRapHost class is a simple g-code transmitter to a RepRap
 * or a similar 3D printer.
 * It requires boost library for interpreting the g-code and the
 * BoostComPort class with gives a basic platform independent serial 
 * port connection.
 * Basically you create a instance of that class, call connect() with
 * a specified serial port and baud rate. To add a command to the queue
 * call addCommand() with a raw g-code command like "G1 X10 F100".
 * Then you need to call timerTick() frequently to communicate with the
 * RepRap. Quite simple ;)
 */

#include "RepRapHost.h"
//#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

RepRapHost::RepRapHost() :
comStatus(STANDBY),
remainingTime(0.0),
tempExtruder(0.0),
tempBed(0.0),
tempExpression("([A-Z]): *([0-9]+.?[0-9]*)"),
nextLineNumber(0),
hashEnabled(true),
debug(false),
hardwareX(0.0),
hardwareY(0.0),
hardwareZ(0.0),
hardwareF(0.0),
lastX(0.0),
lastY(0.0),
lastZ(0.0),
lastF(0.0),
lastE(0.0)
{
	
}

RepRapHost::~RepRapHost()
{

}

void RepRapHost::setDebug(bool debug)
{
	this->debug=debug;
}

int RepRapHost::connect(string port, int baud)
{
	if(comPort.isOpended())
		comPort.close();
	return comPort.open(port, baud);
}

int RepRapHost::disconnect()
{
	return comPort.close();
}

bool RepRapHost::isConnected()
{
	return comPort.isOpended();
}

void RepRapHost::setNextLineNumber(int nextLineNumber)
{
	this->nextLineNumber=nextLineNumber;
}

void RepRapHost::refreshRemainingTime()
{
	remainingTime=0.0;
	for(unsigned int i=0; i<commands.size(); i++)
		remainingTime+=commands[i].time;
}

double RepRapHost::getRemainingTime()
{
	return remainingTime;
}

Command* RepRapHost::addCommand(string cmdStr, bool putAtEnd, bool removeWhenDouble)
{
	if(removeWhenDouble && commands.size()>0 && commands[0].command==cmdStr)
		return &commands[0];
		
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace phoenix = boost::phoenix;
	using qi::double_;
	using qi::int_;
	using qi::phrase_parse;
	using qi::_1;
	using ascii::space;
	using qi::lit;
	using phoenix::ref;
	
	int g=-1;
	int m=-1;
	double newX=lastX;
	double newY=lastY;
	double newZ=lastZ;
	double newF=lastF;
	double newE=lastE;
	if(debug)
		cout<<"New command: "<<cmdStr<<endl;
	cmdStr=toUpper(cmdStr);
	//cout<<"upper case command: "<<cmdStr<<endl;
	string::iterator begin=cmdStr.begin();
	string::iterator end=cmdStr.end();
	qi::phrase_parse(begin, end,
			//  Begin grammar
			(
					((lit('G') > int_[ref(g)=_1]) | (lit('M') > int_[ref(m)=_1]))
					>>
					(
							(lit('X')>double_[ref(newX)=_1]) ^
							(lit('Y')>double_[ref(newY)=_1]) ^
							(lit('Z')>double_[ref(newZ)=_1]) ^
							(lit('F')>double_[ref(newF)=_1]) ^
							(lit('E')>double_[ref(newE)=_1])
					)
			)
			,
			//  End grammar
			space);
	//cout<<"x: "<<x<<", y: "<<y<<", z: "<<z<<", e: "<<e<<", f: "<<f<<", m: "<<m<<", g: "<<g<<endl;
	
	if(cmdStr.find("*")!=string::npos)
	{
		cout<<"The gcode file you choosed is not (yet) supported because it already contains hashes. Remove the hashes or fix this program to remove them ;)";
		//commands.clear();
		return NULL;
	}
	string hashedCommand;
	if(hashEnabled)
	{
		hashedCommand="N";
		hashedCommand+=int2String(nextLineNumber++)+" "+cmdStr+" *";
		hashedCommand+=getHash(hashedCommand);
		cout<<"Converted command to: "<<hashedCommand<<endl;
	}
	else
	{
		hashedCommand=cmdStr;
	}
	Command commandStruct;
	commandStruct.time=0.0;
	commandStruct.command=hashedCommand;
	commandStruct.m=m;
	commandStruct.g=g;
	commandStruct.x=newX;
	commandStruct.y=newY;
	commandStruct.z=newZ;
	commandStruct.f=newF;
	
	// Calculate the time this command will need
	double dx;
	double dy;
	double dz;
	double distance;
	
	if(newF==0.0)
	{
		cout<<"The feedrate is not set, assuming 100mm/sec"<<endl;
		newF=100.0;
	}
	switch(m)
	{
	case -1:
		break;
	default:
		commandStruct.time=0.0;
	}
	switch(g)
	{
	case -1:
		break;
	case 1:  // G1 command
	case 0:  // G0 command
		dx=lastX-newX;
		dy=lastY-newY;
		dz=lastZ-newZ;
		distance=sqrt(dx*dx+dy*dy+dz*dz);
		commandStruct.time=distance/newF*60.0;
		break;
	default:
		commandStruct.time=0.0;
	}
	if(debug)
		cout<<"Calculated time for command: "<<commandStruct.time<<" seconds"<<endl;
	lastX=newX;
	lastY=newY;
	lastZ=newZ;
	lastF=newF;
	lastE=newE;
	remainingTime+=commandStruct.time;
	if(putAtEnd)
	{
		commands.push_back(commandStruct);
		return &commands.back();
	}
	else
	{
		commands.insert(commands.begin(), commandStruct);
		return &(*commands.begin());
	}
}

void RepRapHost::timerTick()
{
	if(!comPort.isOpended())
	{
		comStatus=STANDBY;
		return;
	}
	Command command;
	string answer;
	char* buffer;
	int size;
	
	comPort.poll();
	
	if(comStatus==STANDBY)
	{
		if(!commands.size())
			return;
		command=commands[0];
		command.command+="\n";
		comPort.clearBuffers();  // Make shure there is nothing old left in the buffer
		comPort.write((char*)command.command.c_str(), command.command.length());
		hardwareX=command.x;
		hardwareY=command.y;
		hardwareZ=command.z;
		hardwareF=command.f;
		
		
		remainingTime-=command.time;
		commands.erase(commands.begin());
		if(command.m==105)
			comStatus=WAITING_FOR_TEMP;
		else if(command.m==109 || command.m==116)
			comStatus=WAITING_FOR_TEMP_ACHIEVED;
		else
			comStatus=WAITING_FOR_OK;
		if(debug)
			cout<<"Send command: "<<command.command.substr(0, command.command.length()-1)<<endl;
	}
	else if(comStatus==WAITING_FOR_TEMP)
	{
		// TODO: A timeout is missing
		buffer=new char[1024];
		size=comPort.readUntil(buffer, 1000, (char*)"\n", 1, false);
		if(size<=0)
		{
			delete[] buffer;
			return;
		}
		buffer[size]=0;
		if(string(buffer).find("ok")!=string::npos)
		{
			delete[] buffer;
			return;
		}
		//cout<<"answer size: "<<size<<endl;
		buffer[size-1]=0;
		answer=buffer;
		delete[] buffer;
		// Waiting for temperature is a little bit complicated because
		// there are different answers depending on the firmware and
		// firmware version. We will try to guess the correct method.
		// Because of the fact that I have no idea of regular expressions
		// I have no idea whether this will work everywhere.
		if(debug)
			cout<<"Got answer: "<<answer<<endl;
		string::const_iterator start, end;
		start=answer.begin();
		end=answer.end();
		boost::match_results<std::string::const_iterator> what;
		boost::match_flag_type flags = boost::match_default;
		//boost::regex tempExpression(re);
		while(boost::regex_search(start, end, what, tempExpression, flags))
		{
			string strTempType=what[1];
			string strTemp=what[2];
			double temp=string2Double(strTemp);
			if(strTempType=="T")
				tempExtruder=temp;
			else if(strTempType=="B")
				tempBed=temp;
			else
				cout<<"Unable to interprete thw temperature answer: "<<what[0]<<endl;
			if(debug)
				cout<<"Temp type: "<<strTempType<<", Temp: "<<temp<<endl;
			start = what[0].second;
			flags |= boost::match_prev_avail;
			flags |= boost::match_not_bob;
		}
		if(debug)
			cout<<"Finished interpreting the answer..."<<endl;
		// TODO: This is not a very good way to obmit the ok answer
		if(debug)
			cout<<"Now I will delete the current buffer because some firmware will send a ok\\n after the temperature"<<endl;
		comPort.clearBuffers();
		comStatus=STANDBY;
	}
	else if(comStatus==WAITING_FOR_TEMP_ACHIEVED)
	{
		// TODO: A timeout is missing
		buffer=new char[1024];
		size=comPort.readUntil(buffer, 1000, (char*)"achieved", 1, false);
		if(size<=0)
		{
			delete[] buffer;
			return;
		}
		if(debug)
			cout<<"Temperature is achieved, continuing..."<<endl;
		comPort.clearBuffers();
		comStatus=STANDBY;
	}
	else if(comStatus==WAITING_FOR_OK)
	{
		// TODO: A timeout is missing
		buffer=new char[1024];
		size=comPort.readUntil(buffer, 1000, (char*)"\n", 1, false);
		if(size<=0)
		{
			delete[] buffer;
			return;
		}
		buffer[size-1]=0;
		answer=buffer;
		delete[] buffer;
		if(toLower(answer).find("ok")!=string::npos)
		{
			comStatus=STANDBY;
			if(debug)
			{
				cout<<"Got answer: "<<answer<<endl;;
				cout<<"This answer was interpreted as \"ok\""<<endl;
			}
		}
		else
		{
			if(debug)
			{
				cout<<"Got unknown answer when waiting for \"ok\": "<<answer<<endl;
				cout<<"Will treat this answer as ok too, no idea what else to do..."<<endl;
				// TODO: Try catch some default errors like "bad M/G-Code", ...
			}
		}
	}
}

void RepRapHost::setHashEnabled(bool enable)
{
	hashEnabled=enable;
}

bool RepRapHost::getHashEnabled()
{
	return hashEnabled;
}

string RepRapHost::toLower(string str)
{
	for (unsigned int i=0;i<str.length();i++)
		if (str[i] >= 0x41 && str[i] <= 0x5A)
			str[i] = str[i] + 0x20;
	return str;
}

string RepRapHost::toUpper(string str)
{
	for (unsigned int i=0;i<str.length();i++)
		if (str[i] >= 0x61 && str[i] <= 0x7A)
			str[i] = str[i] - 0x20;
	return str;
}

string RepRapHost::int2String(int value)
{
	ostringstream Buffer;
	Buffer<<value;
	return Buffer.str();
}

string RepRapHost::double2String(double value)
{
	ostringstream Buffer;
	Buffer<<value;
	return Buffer.str();
}

double RepRapHost::string2Double(string value)
{
  istringstream buffer(value);
  double returncode;
  buffer >> returncode;
  return returncode;
}

string RepRapHost::getHash(string cmd)
{
	int cs = 0;
	for(int i=0; cmd[i]!='*' && cmd[i]!=0; i++)
		cs = cs ^ cmd[i];
	cs &= 0xff;
	ostringstream stream;
	stream<<cs;
	string hash=stream.str();
	return hash;
}

double RepRapHost::getTempExtruder()
{
	return tempExtruder;
}

double RepRapHost::getTempBed()
{
	return tempBed;
}

void RepRapHost::clear()
{
	commands.clear();
	remainingTime=0.0;
}

iostream& RepRapHost::enableConsoleStream()
{
	return comPort.enableStream();
}

void RepRapHost::disableConsoleStream()
{
	comPort.disableStream();
}

int RepRapHost::commandsLeft()
{
	return commands.size();
}

void RepRapHost::getXYZF(double& x, double& y, double& z, double& f)
{
	x=this->hardwareX;
	y=this->hardwareY;
	z=this->hardwareZ;
	f=this->hardwareF;
}
