// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcArithmeticCoder.h"

namespace vanilc {

ArithmeticCoder::ArithmeticCoder(const EntropyCoder::bitqueue& bitstream) :
GenericDistributionCoder(bitstream),
rangemin(0),
rangemax(numeric_limits<RANGETYPE>::max() / 2 + rangemin / 2 + 1),
rangehalf(rangemax / 2 + rangemin / 2),
rangequarter(rangehalf / 2 + rangemin / 2),
rangethreequarter(rangemax / 2 + rangehalf / 2),
low(rangemin),
high(rangemax),
zoom(0) {
} // end ::ArithmeticCoder

void ArithmeticCoder::reset() {
	EntropyCoder::reset();
} // end ArithmeticCoder::reset

void ArithmeticCoder::finalize() {
	if(low >= rangemax - high) {
		this->pushBin(true);
		zoom += 2;
		while(--zoom) this->pushBin(false);
	} else {
		this->pushBin(false);
		zoom += 2;
		while(--zoom) this->pushBin(true);
	}
	low = rangemin; high = rangemax;
	EntropyCoder::finalize();
} // end ArithmeticCoder::finalize

double ArithmeticCoder::costs(unsigned int symbol) {
	ImplicitDistributionElement* pDistribution = this->distribution->data();
	#ifdef WIN32
		return log(pDistribution[symbol + 1].get() - pDistribution[symbol].get()) / log(0.5);
	#else
		return -log2(pDistribution[symbol + 1].get() - pDistribution[symbol].get());
	#endif
} // end ArithmeticCoder::costs

void ArithmeticCoder::encode(unsigned int symbol) {
	const RANGETYPE rangeLength = high - low;
	ImplicitDistributionElement* pDistribution = this->distribution->data();
	pDistribution->getDistributionMaker()->setRangeParameters(low, rangeLength);
	high = pDistribution[symbol + 1].getRangeValue();
	low = pDistribution[symbol].getRangeValue();
	if(low == high) throw ZeroProbabilityException();
	while(true) { // encode as much as possible
		if(high <= rangehalf) {
			this->pushBin(false);
			++zoom;
			while(--zoom) this->pushBin(true);
			low <<= 1; high <<= 1;
		} else if(low >= rangehalf) {
			this->pushBin(true);
			++zoom;
			while(--zoom) this->pushBin(false);
			low = (low - rangehalf) << 1; high = (high - rangehalf) << 1;
		} else break;
	}
	while(low >= rangequarter && high <= rangethreequarter) { // zoom in
		++zoom;
		low = (low - rangequarter) << 1; high = (high - rangequarter) << 1;
	}
} // end ArithmeticCoder::encode

unsigned int ArithmeticCoder::decode() {
	const RANGETYPE rangeLength = high - low;
	ImplicitDistributionElement* pDistribution = this->distribution->data();
	pDistribution->getDistributionMaker()->setRangeParameters(low, rangeLength);
	this->resetReader();
	unsigned int minSym = 0, maxSym = this->distribution->size() - 2;
	vector<ImplicitDistributionElement>::iterator minSymbol = this->distribution->begin();
	vector<ImplicitDistributionElement>::iterator maxSymbol = this->distribution->end() - 2;
	RANGETYPE lowbound = rangemin, highbound = rangemax, boundlength = highbound / 2 - lowbound / 2;
	if(minSymbol != maxSymbol) { // read first bin
		if(this->readBin()) {
			lowbound += boundlength;
			minSymbol = upper_bound(minSymbol + 1, maxSymbol + 1, lowbound, compUpper) - 1;
		} else {
			highbound -= boundlength;
			maxSymbol = lower_bound(minSymbol + 1, maxSymbol + 1, highbound, compLower) - 1;
		}
		for(int i = zoom; i; --i) this->readBin(); // after first read bin skip zoom bins
		boundlength >>= 1;
	}
	while(minSymbol != maxSymbol) { // read further bins as required
		if(this->readBin()) {
			lowbound += boundlength;
			minSymbol = upper_bound(minSymbol + 1, maxSymbol + 1, lowbound, compUpper) - 1;
		} else {
			highbound -= boundlength;
			maxSymbol = lower_bound(minSymbol + 1, maxSymbol + 1, highbound, compLower) - 1;
		}
		boundlength >>= 1;
	}
	low = minSymbol->getRangeValue();
	high = (++minSymbol)->getRangeValue();
	while(true) { // delete bins from bitstream and adapt boundaries
		if(high <= rangehalf) {
			this->popBin();
			++zoom;
			while(--zoom) this->popBin();
			low <<= 1; high <<= 1;
		} else if(low >= rangehalf) {
			this->popBin();
			++zoom;
			while(--zoom) this->popBin();
			low = (low - rangehalf) << 1; high = (high - rangehalf) << 1;
		} else break;
	}
	while(low >= rangequarter && high <= rangethreequarter) { // zoom in
		++zoom;
		low = (low - rangequarter) << 1; high = (high - rangequarter) << 1;
	}
	return maxSymbol->getPosition();
} // end ArithmeticCoder::decode

} // end namespace vanilc

