/*
 * InputStream.cpp
 *
 *  Created on: 16/09/2016
 *      Author: Martin
 */

#include "InputStreamA.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

InputStreamA::InputStreamA() {
	filedesc = -1;
}

InputStreamA::~InputStreamA() {
	// TODO Auto-generated destructor stub
}

void InputStreamA::open(const char* s) {
	filedesc = ::open(s, O_RDONLY);
}

int InputStreamA::readNext() {
	int ret;
	int bytesRead = ::read(filedesc, &ret , sizeof(int));
	return bytesRead > 0 ? ret : bytesRead;
}

bool InputStreamA::endOfStream() {
	bool b = false;
	int val;
	int bytesRead = ::read(filedesc, &val, sizeof(int));
	if (bytesRead == 0) b = true;
	lseek(filedesc, -bytesRead, SEEK_CUR);
	return b;
}

void InputStreamA::close() {
	::close(filedesc);
}
