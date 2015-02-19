// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <deque>

#include "vanilcDefinitions.h"

namespace vanilc {

using namespace std;
using namespace cv;

class EntropyCoder {
public:
	typedef deque<STREAMTYPE> bitqueue;

	EntropyCoder() : bitstream(bitqueue()), streamFront(8 * sizeof(STREAMTYPE) - 1), streamBack(0) {};
	// reference used to avoid passing data through inheritance chain
	EntropyCoder(const bitqueue& bitstream) : bitstream(bitstream), streamFront(8 * sizeof(STREAMTYPE) - 1), streamBack(0)  {};
	void setBitstream(bitqueue bitstream) { this->bitstream = bitstream; };
	bitqueue getBitstream() { return bitstream; };
	unsigned int readBitstream(ifstream& fs, int numberOfElements = -1); // return bytes read
	unsigned int writeBitstream(ofstream& fs); // return bytes written
	virtual void reset();
	virtual void finalize();
	virtual double costs(unsigned int symbol) = 0;
	virtual void encode(unsigned int symbol) = 0;
	virtual unsigned int decode() = 0;
	void code(unsigned int& symbol, bool encoding) { if(encoding) encode(symbol); else symbol = decode(); };
	void code(double& symbol, bool encoding) { if(encoding) encode((unsigned int)symbol); else symbol = (double)decode(); };

protected:
	void pushBin(bool bin);
	void popBin();
	void resetReader();
	bool readBin();

private:
	bitqueue bitstream;
	bitqueue::iterator streamReaderIterator;
	unsigned int streamReaderIndex; // index of next bin to be read in one STREAMTYPE element
	unsigned int streamFront; // index of next bin to be read in one STREAMTYPE element
	unsigned int streamBack; // index of last written bin in one STREAMTYPE element
};

class EndOfBitstreamException : public Exception {
	virtual const char* what() const throw() { return "End of Bitstream reached."; }
};

} // end namespace vanilc

