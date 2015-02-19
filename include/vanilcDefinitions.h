// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

// Attention: The parameters within this file are used to define algorithmic details. For most of them a change does not make sense. Doing so will probably result in a drop in compression performance or even a program crash, so be careful changing them.

// -------------------- General --------------------

// Use the boost library: on Linux, only the t distribution will not work and is therefore replaced by a normal distribution.
// On Windows, also the normal distribution will not work and is therefore replaced by a laplace distribution.
#define BOOST

// -------------------- Verbosity --------------------
// Debug output on cout.
//#define DEBUGOUT

// Observe encoding process in an image window.
//#define OBSERVEENCODING

// When OBSERVEENCODING is defined, choose what to show during encoding
//#define SHOW_PREDICTION
#define SHOW_PREDICTION_ERROR
//#define SHOW_ABS_PREDICTION_ERROR
//#define SHOW_STANDARD_DEVIATION
//#define SHOW_DOF
//#define SHOW_BITS

// When OBSERVEENCODING is defined, choose after how many rows the shown image is updated
const unsigned int OBSERVATION_UPDATE_INTERVAL = 64;

// -------------------- Least-Squares Settings --------------------
// Order in which color channels are processed (e.g., { 0, 1, 2 } for BGR).
const unsigned int CHANNEL_ORDER[] = { 1, 2, 0 };

// -------------------- Entropy Coding --------------------
// Use Rice-Golomb entropy coding or arithmetic coding.
//#define GOLOMB_CODING
#define ARITHMETIC_CODING

// Internal data type which holds the bits of the bitstream: use uchar for platform-independece (-> byte order) and best compression ratio (unsigned int is slightly faster).
typedef unsigned char STREAMTYPE;

// Data type for arithmetic coding boundaries: defines the resolution and therefore the efficiency of arithmetic coding.
// Attention: use only unsigned data types; non-native data types probably have bad impact on performance
typedef unsigned int RANGETYPE;
//typedef unsigned long long RANGETYPE; // _maybe_ improves compression efficiency by (only) a few bits on 64 bit systems

