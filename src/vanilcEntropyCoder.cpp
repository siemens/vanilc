// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcEntropyCoder.h"

namespace vanilc {

void EntropyCoder::pushBin(bool bin) {
	if(!streamBack) {
		bitstream.push_back(0); // add another STREAMTYPE element to the stream
		streamBack = 8 * sizeof(STREAMTYPE);
	}
	--streamBack;
	if(bin) bitstream.back() |= (1 << streamBack);
} // end EntropyCoder::pushBin

void EntropyCoder::popBin() {
	if(!streamFront) {
		bitstream.pop_front();
		streamFront = 8 * sizeof(STREAMTYPE);
	}
	--streamFront;
} // end EntropyCoder::popBin

void EntropyCoder::resetReader() {
	streamReaderIterator = bitstream.begin();
	streamReaderIndex = streamFront;
} // end EntropyCoder::readFistBin

bool EntropyCoder::readBin() {
	if(streamReaderIterator == bitstream.end()) throw EndOfBitstreamException();
	bool result = (bool)(*streamReaderIterator & (1 << streamReaderIndex));
	if(!streamReaderIndex) {
		++streamReaderIterator;
		streamReaderIndex = 8 * sizeof(STREAMTYPE);
	}
	--streamReaderIndex;
	return result;
} // end EntropyCoder::readNextBin

unsigned int EntropyCoder::readBitstream(ifstream& fs, int numberOfElements) {
	STREAMTYPE buffer;
	int i;
	for(i = 0; i != numberOfElements; ++i) {
		fs.read((char*) &buffer, sizeof(STREAMTYPE));
		if(!fs) break;
		bitstream.push_back(buffer);
	}
	resetReader();
	return i * sizeof(STREAMTYPE);
} // end EntropyCoder::readBitstream

// writes bitstream up to current position (only writes last element if it is finished)
unsigned int EntropyCoder::writeBitstream(ofstream& fs) {
	bitqueue::iterator currentElement = bitstream.begin();
	bitqueue::iterator previousElement = bitstream.begin();
	int i = 0;
	if(previousElement != bitstream.end()) {
		++currentElement;
		for(; currentElement != bitstream.end(); ++currentElement, ++previousElement, ++i)
			fs.write((char*) &(*previousElement), sizeof(STREAMTYPE));
		if(!streamBack) {
			fs.write((char*) &(*previousElement), sizeof(STREAMTYPE));
			++i;
			bitstream.erase(bitstream.begin(), currentElement);
		} else bitstream.erase(bitstream.begin(), previousElement);
	}
	return i * sizeof(STREAMTYPE);
} // end EntropyCoder::writeBitstream

void EntropyCoder::reset() {
	streamFront = 8 * sizeof(STREAMTYPE) - 1;
	streamBack = 0;
	bitstream.clear();
	resetReader();
} // end EntropyCoder::reset

void EntropyCoder::finalize() {
	streamBack = 0;
} // end EntropyCoder::finalize

} // end namespace vanilc

