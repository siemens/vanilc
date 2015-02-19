// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcRawIO.h"

namespace vanilc {

Mat RawIO::imread(const string filename, const string dimensions) {
	char dimstr[32];
	strcpy(dimstr, dimensions.c_str());
	int bitdepth = atoi(strtok(dimstr, "x"));
	int sz[3];
	sz[2] = atoi(strtok(NULL, "x"));
	sz[1] = atoi(strtok(NULL, "x"));
	sz[0] = atoi(strtok(NULL, "x"));
	ifstream fs(filename.c_str(), ios::in | ios::binary);
	if(bitdepth == 8) {
		Mat image(3, sz, CV_8U);
		fs.read(image.ptr<char>(), sizeof(uchar) * image.total());
		fs.close();
		return image;
	} else if(bitdepth == 16) {
		Mat image(3, sz, CV_16U);
		fs.read(image.ptr<char>(), sizeof(ushort) * image.total());
		fs.close();
		return image;
	}
	else throw NoValidBitdepthException();
} // end RawIO::imread

void RawIO::imwrite(const string filename, const Mat image) {
	ofstream fs(filename.c_str(), ios::out | ios::binary);
	if(image.depth() == CV_64F) {
		fs.write(image.ptr<char>(), sizeof(double) * image.total());
	} else if(image.depth() == CV_8U) {
		fs.write(image.ptr<char>(), sizeof(uchar) * image.total());
	} else {
		fs.write(image.ptr<char>(), sizeof(ushort) * image.total());
	}
	fs.close();
} // end RawIO::imwrite

} // end namespace vanilc

