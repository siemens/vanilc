// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcRiceGolombCoder.h"

namespace vanilc {

void RiceGolombCoder::finalize() {
	if(runlength) encodeGolomb(runlength, runM);
	EntropyCoder::finalize();
} // end RiceGolombCoder::finalize

double RiceGolombCoder::costs(unsigned int symbol) { // runlength encoding is not considered!
	unsigned int m = (unsigned int)ceil(MAGICFACTOR * (sqrt(SQUAREDSTRETCHMAPPING + variance) - STRETCHMAPPING));
	unsigned int p = (unsigned int)(mean + .5); // rounded mean (integer-valued prediction)
	symbol = (double)p > mean // map symbol so that it is approximately geometrically distributed
		? (symbol < p ? (p - symbol << 1) - 1 : symbol - p << 1) // zero and positives become even
		: (symbol > p ? (symbol - p << 1) - 1 : p - symbol << 1); // zero and negatives become even
	p = symbol / m; // quotient
	symbol -= p * m; // remainder
	int result = p + 1;
	p = -1;
	for(unsigned int mm = m; mm; ++p) mm >>= 1; // p = floor(ld(m));
	if(symbol >= (m = (2 << p) - m)) { ++p; }; // p+1 bits
	return result + p;
} // end RiceGolombCoder::costs

void RiceGolombCoder::encode(unsigned int symbol) {
	unsigned int m = (unsigned int)ceil(MAGICFACTOR * (sqrt(SQUAREDSTRETCHMAPPING + variance) - STRETCHMAPPING));
	unsigned int p = (unsigned int)(mean + .5); // rounded mean (integer-valued prediction)
	symbol = (double)p > mean // map symbol so that it is approximately geometrically distributed
		? (symbol < p ? (p - symbol << 1) - 1 : symbol - p << 1) // zero and positives become even
		: (symbol > p ? (symbol - p << 1) - 1 : p - symbol << 1); // zero and negatives become even
	if(runlength || variance < VARFORPROBZEROTOOLARGE) { // runlength encoding
		if(variance < .1) { variance = .1; m = 1; }
		if(!runlength) // start new run -> compute optimal variance
			runM = (unsigned int)floor(-1.0 / log2(1.0 - exp(MINUSONEDIVSQRT2 / sqrt(variance))));
		if(runlength >= 10 * runM) { // run is stopped to prevent long runs using small runM
			encodeGolomb(runlength, runM);
			runlength = 0;
		} else if(symbol) { // run is over
			encodeGolomb(runlength, runM);
			runlength = 0;
			--symbol; // save bits: symbol cannot be zero after run ends
		} else { // run continues
			++runlength;
			return;
		}
	}
	encodeGolomb(symbol, m);
} // end RiceGolombCoder::encode

void RiceGolombCoder::encodeGolomb(unsigned int symbol, unsigned int m) {
	int p = symbol / m; // quotient
	symbol -= p * m; // remainder
	while(p--)
		pushBin(true);
	pushBin(false); // unary coding of quotient
	for(unsigned int mm = m; mm; ++p) mm >>= 1; // p = floor(ld(m));
	if(symbol >= (m = (2 << p) - m)) { ++p; symbol += m; }; // p+1 bits
	while(--p > -1)
		pushBin((bool)(symbol & 1 << p)); // truncated binary coding of remainder in p or p+1 bits
} // end RiceGolombCoder::encodeGolomb

unsigned int RiceGolombCoder::decode() {
	unsigned int symbol;
	if(runlength || variance < VARFORPROBZEROTOOLARGE) { // runlength decoding
		if(variance < .1) variance = .1;
		if(!runlength) { // run starts
			runM = (unsigned int)floor(-1.0 / log2(1.0 - exp(MINUSONEDIVSQRT2 / sqrt(variance))));
			runlength = decodeGolomb(runM) + 1;
			if(runlength > 10 * runM) runM = 0; // remembers that run was stopped to prevent long runs using small runM
		}
		if(--runlength) symbol = 0; // run continued
		else { // run is over
			symbol = decodeGolomb((unsigned int)ceil(MAGICFACTOR * (sqrt(SQUAREDSTRETCHMAPPING + variance) - STRETCHMAPPING)));
			if(runM) ++symbol;
		}
	} else symbol = decodeGolomb((unsigned int)ceil(MAGICFACTOR * (sqrt(SQUAREDSTRETCHMAPPING + variance) - STRETCHMAPPING)));
	unsigned int p = (unsigned int)(mean + .5); // rounded mean (integer-valued prediction)
	return (double)p > mean // map geometrically distributed symbol back
		? (symbol != symbol >> 1 << 1 ? p - (symbol + 1 >> 1) : p + (symbol >> 1)) // odds become negative
		: (symbol != symbol >> 1 << 1 ? p + (symbol + 1 >> 1) : p - (symbol >> 1)); // odds become positive
} // end RiceGolombCoder::decode

unsigned int RiceGolombCoder::decodeGolomb(unsigned int m) {
	unsigned int symbol = 0, quotient = 0, mm = m;
	while(readBin()) ++quotient; // unary decoding of quotient
	int p = -1;
	for(; mm; ++p) mm >>= 1; // p = floor(ld(m));
	for(mm = p; mm; --mm) symbol = (symbol << 1) | (unsigned int)readBin(); // truncated binary decoding of remainder
	if(symbol >= (mm = (2 << p) - m)) { ++p; symbol = ((symbol << 1) | (unsigned int)readBin()) - mm; }; // p+1 bits
	return symbol + quotient * m;
} // end RiceGolombCoder::decodeGolomb

} // end namespace vanilc

// Creation of MAGICFACTOR and STRETCHMAPPING using Octave:
// % consider also Theorem 5 in [Ali and Murshed, 2013: Modified Rice-Golomb Code for Predictive Coding of Integers with Real-valued Predictions]
// % the mapping described there has been approximated by fitting a hyperbola with its center shifted to the origin
// for i = 0:99; s = roots([1 1 zeros(1,i) -1]); s = s(s>=0 & s<=1); [~,j] = min(abs(imag(s))); mys(i+1) = -(sqrt(2) * log(s(j))) ^ (-1); end
// for i = 2000; s = roots([1 1 zeros(1,i) -1]); s = s(s>=0 & s<=1); [~,j] = min(abs(imag(s))); a = -(sqrt(2) * log(s(j))) ^ (-1); end
// for i = 2001; s = roots([1 1 zeros(1,i) -1]); s = s(s>=0 & s<=1); [~,j] = min(abs(imag(s))); b = -(sqrt(2) * log(s(j))) ^ (-1); end
// MAGICFACTOR = 1/(b-a) % this is equal to sqrt(2)*log(2) (the probability of zero in a Laplacian distribution with variance one), thus
// MAGICFACTOR = sqrt(2) * log(2);
// STRETCHMAPPING = sqrt(2)*log(2)*mys(100)^2/200-100/(2*sqrt(2)*log(2))
// VARFORPROBZEROTOOLARGE = 1/MAGICFACTOR^2
// x=0:20; plot([0 mys(1:20)], x, 'r')
// hold on; plot(x, sqrt(2) * log(2) * (sqrt(myb^2 + x.^2) - myb), 'k')

