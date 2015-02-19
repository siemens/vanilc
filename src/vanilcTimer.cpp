// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcTimer.h"

namespace vanilc {

double Timer::getCPUtime() const {
	return (double)(clock() - cpuTime) / CLOCKS_PER_SEC;
} // end Timer::getCPUtime

double Timer::getREALtime() const {
	return (double)(getCurrentTime() - realTime) / 1000.0;
} // end Timer::getREALtime

unsigned long long Timer::getCurrentTime() const {
#ifdef WIN32
	// Windows
	FILETIME ft;
	LARGE_INTEGER mytime;
	GetSystemTimeAsFileTime(&ft);
	mytime.LowPart = ft.dwLowDateTime;
	mytime.HighPart = ft.dwHighDateTime;
	return ((unsigned long long)mytime.QuadPart - 116444736000000000LL) / 10000LL;
#else
	// Linux
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long long)tv.tv_sec * 1000LL + (unsigned long long)tv.tv_usec / 1000LL;
#endif
} // end Timer::getCurrentTime

} // end namespace vanilc

