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

// false and true values for uchar masks
const uchar _FLS_ = 0;
const uchar _TRU_ = 1;

namespace vanilc {

using namespace std;
using namespace cv;

class StructuringElement {
public:
	StructuringElement() {};
	StructuringElement(const Mat& mask, const Point& anchor) : anchor(Point3i(anchor.x, anchor.y, 0)) { setMask(mask); };
	StructuringElement(const Mat& mask, const Point3i& anchor) : anchor(anchor) { setMask(mask); };

	static StructuringElement createHalfRectElement(unsigned int top, unsigned int left, unsigned int right, const bool centerPixel);
	static StructuringElement createHalfSquareElement(unsigned int radius, const bool centerPixel) { return createHalfRectElement(radius, radius, radius, centerPixel); };
	static StructuringElement createHalfCuboidElement(unsigned int top, unsigned int left, unsigned int right, unsigned int bottom, unsigned int front, const bool centerPixel);
	static StructuringElement createHalfCubeElement(unsigned int radius, const bool centerPixel) { return createHalfCuboidElement(radius, radius, radius, radius, radius, centerPixel); };
	static StructuringElement createHalfEllipseElement(double top, double left, double right, const bool centerPixel);
	static StructuringElement createHalfEllipsoidElement(double top, double left, double right, double bottom, double front, const bool centerPixel);
	static StructuringElement createHalfCircleElement(double radius, const bool centerPixel) { return createHalfEllipseElement(radius, radius, radius, centerPixel); };
	static StructuringElement createHalfEllipseElementMultichannel(double top, double left, double right, unsigned int channels, const bool centerPixel);
	static StructuringElement createHalfEllipseElementMultichannelForward(double top, double left, double right, unsigned int channels, const bool centerPixel);

	void setMask(const Mat& mask) { this->mask = mask; numel = countNonZero(mask); };
	const Mat& getMask() const { return mask; };
	void setAnchor(const Point3i& anchor) { this->anchor = anchor; };
	const Point3i& getAnchor() const { return anchor; };
	unsigned int getCols() const { return mask.size[mask.dims - 1]; }; // in contrast to mask.cols, this version works also for multi-dimensional arrays
	unsigned int getRows() const { return mask.size[mask.dims - 2]; };
	unsigned int getSlcs() const { return (mask.dims == 3 ? mask.size[0] : 1); }; // slices for 3-D
	unsigned int getLeft() const { return anchor.x; };
	unsigned int getRight() const { return mask.size[mask.dims - 1] - anchor.x - 1; };
	unsigned int getTop() const { return anchor.y; };
	unsigned int getBottom() const { return mask.size[mask.dims - 2] - anchor.y - 1; };
	unsigned int getFront() const { return anchor.z; };
	unsigned int getBack() const { return (mask.dims == 3 ? mask.size[0] - anchor.z - 1 : 0); };
	unsigned int getNumberOfElements() const { return numel; };
	int increment(Point3i& position) const;

	Mat extractVectorFromPatch(const Mat& patch) const; // extracts values from patch according to mask; matrix sizes need to match!
	void extractVectorFromPatch(const Mat& patch, double* destination) const; // efficient version; assumes that destination is already allocated!
	Mat extractVectorFromImage(const Mat& image, const Point3i& position) const;
	void extractVectorFromImage(const Mat& image, Point3i position, double* destination) const;
	void extractVectorFromImageBorderSafe(const Mat& image, const Point3i& position, Mat& destination);
	void computeHistogramFromImageBorderSafe(const Mat& image, Point3i position, Mat& histogram); // (integer) histogram needs to be allocated before!

protected:
	Mat mask;
	Point3i anchor;
	unsigned int numel;
};

} // end namespace vanilc

