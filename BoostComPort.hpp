/*
Copyright 2010 by Fabian Schwartau
E-Mail: fabian@opencode.eu

This file is part of BoostComPort.

BoostComPort is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HM2PC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BoostComPort.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BOOSTCOMPORT_HPP_
#define BOOSTCOMPORT_HPP_

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <boost/timer.hpp>

#define BUFFER_SIZE 1000000
#define EVENTBUFFER_SIZE 1000000

using namespace std;

/*
 * BoostComPort is a simple and small class providing access
 * to the computers com ports (serial ports).
 * Hardware and software flow control are not supported yet.
 * Non standard baud rates are supported (useable for example
 * with the FTDI FT232 chip).
 */
class BoostComPort
{
public:
	BoostComPort();
	~BoostComPort();
	int open(std::string port, int baud);
	int close();
	bool isOpended();
	int write(char* Data, unsigned int Length);
	int read(char* data, int length, bool blocking=false, int timeout=-1 /* timeout in ms */);
	int readUntil(char* data, int maxLength, char* searchValue, int searchSize, bool blocking=false, int timeout=-1);
	void poll();
	void clearBuffers();
	boost::system::error_code& getLastError();

private:
	void onPortRead(const boost::system::error_code& error, std::size_t bytes_transferred);

	char* buffer;
	char* eventBuffer;
	int currentContent;
	bool executed;
	boost::asio::io_service io_service;
	boost::asio::serial_port serialPort;
	boost::system::error_code ec;
	boost::system::error_code lastError;
protected:

};

#endif /* BOOSTCOMPORT_HPP_ */
