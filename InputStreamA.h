/*
 * InputStream.h
 *
 *  Created on: 16/09/2016
 *      Author: Martin
 */

#include "InputStream.h"

#ifndef INPUTSTREAM_H_
#define INPUTSTREAM_H_

using namespace std;

class InputStreamA : public InputStream {
public:
	int filedesc;
	InputStreamA();
	virtual ~InputStreamA();
	void open(char* s);
	int readNext();
	bool endOfStream();
	void close();
};

#endif /* INPUTSTREAM_H_ */
