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
#include <deque>

#include "vanilcDefinitions.h"
#include "vanilcGenericDistributionCoder.h"

namespace vanilc {

using namespace std;
using namespace cv;

class ArithmeticCoder : public GenericDistributionCoder {
public:
	ArithmeticCoder(const EntropyCoder::bitqueue& bitstream = EntropyCoder::bitqueue());
	void reset();
	void finalize();
	double costs(unsigned int symbol);
	void encode(unsigned int symbol);
	unsigned int decode();
	static bool compUpper(RANGETYPE value, DistributionElement& d) { return value < d.getRangeValue(); };
	static bool compLower(DistributionElement& d, RANGETYPE value) { return d.getRangeValue() < value; };

private:
	const RANGETYPE rangemin, rangemax, rangehalf, rangequarter, rangethreequarter;
	RANGETYPE low, high;
	unsigned int zoom;
};

class ZeroProbabilityException : public Exception {
	virtual const char* what() const throw() { return "Probability of symbol to be encoded is too low for resolution of arithmetic coder (maybe zero?). This would lead to an infinite file size which is typically not desirable. Try to add a regularization distribution!"; }
};

} // end namespace vanilc

