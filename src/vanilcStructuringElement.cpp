// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcStructuringElement.h"

namespace vanilc {

StructuringElement StructuringElement::createHalfRectElement(unsigned int top, unsigned int left, unsigned int right, const bool centerPixel) {
	int centerPixelInt = (centerPixel ? 1 : 0);
	int sz[] = { 1, top + 1, left + right + 1 };
	Mat newMask(3, sz, CV_8U, Scalar_<uchar>(_TRU_));
	Range r[] = { Range::all(), Range(top, top + 1), Range(left + centerPixelInt, left + right + 1) };
	if(right || !centerPixelInt) newMask(r) = Scalar_<uchar>(_FLS_);
	return StructuringElement(newMask, Point3i(left, top, 0));
} // end StructuringElement::createHalfRectElement

StructuringElement StructuringElement::createHalfCuboidElement(unsigned int top, unsigned int left, unsigned int right, unsigned int bottom, unsigned int front, const bool centerPixel) {
	int centerPixelInt = (centerPixel ? 1 : 0);
	int sz[] = { front + 1, top + bottom + 1, left + right + 1 };
	Mat newMask(3, sz, CV_8U, Scalar_<uchar>(_TRU_));
	Range r[] = { Range(front, front + 1), Range(top, top + 1), Range(left + centerPixelInt, left + right + 1) };
	if(right || !centerPixelInt) newMask(r) = Scalar_<uchar>(_FLS_);
	r[1] = Range(top + 1, top + bottom + 1); r[2] = Range(0, left + right + 1);
	if(bottom) newMask(r) = Scalar_<uchar>(_FLS_);
	return StructuringElement(newMask, Point3i(left, top, front));
} // end StructuringElement::createHalfCuboidElement

StructuringElement StructuringElement::createHalfEllipseElement(double top, double left, double right, const bool centerPixel) {
	int centerPixelInt = (centerPixel ? 1 : 0);
	int topInt = (int)top, leftInt = (int)left, rightInt = (int)right;
	top = 1.0 / (double)(top * top); left = 1.0 / (double)(left * left); right = 1.0 / (double)(right * right);
	int sz[] = { 1, topInt + 1, leftInt + rightInt + 1 };
	Mat newMask(3, sz, CV_8U);
	for(int k = -topInt; k <= 0; ++k) {
		uchar* newMaskPtr = &(newMask.at<uchar>(0, topInt + k, 0));
		for(int l = -leftInt; l; ++l) // left side
			*(newMaskPtr++) = top * (double)(k * k) + left * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
		for(int l = 0; l <= rightInt; ++l) // right side
			*(newMaskPtr++) = top * (double)(k * k) + right * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
	}
	Range r[] = { Range::all(), Range(topInt, topInt + 1), Range(leftInt + centerPixelInt, leftInt + rightInt + 1) };
	if(rightInt || !centerPixelInt) newMask(r) = Scalar_<uchar>(_FLS_);
	return StructuringElement(newMask, Point3i(leftInt, topInt, 0));
} // end StructuringElement::createHalfEllipseElement

StructuringElement StructuringElement::createHalfEllipsoidElement(double top, double left, double right, double bottom, double front, const bool centerPixel) {
	int centerPixelInt = (centerPixel ? 1 : 0);
	int topInt = (int)top, leftInt = (int)left, rightInt = (int)right, bottomInt = (int)bottom, frontInt = (int)front;
	top = 1.0 / (double)(top * top); left = 1.0 / (double)(left * left); right = 1.0 / (double)(right * right);
	bottom = 1.0 / (double)(bottom * bottom); front = 1.0 / (double)(front * front);
	int sz[] = { frontInt + 1, topInt + bottomInt + 1, leftInt + rightInt + 1 };
	Mat newMask(3, sz, CV_8U);
	for(int j = -frontInt; j <= 0; ++j) {
		for(int k = -topInt; k; ++k) {
			uchar* newMaskPtr = &(newMask.at<uchar>(frontInt + j, topInt + k, 0));
			for(int l = -leftInt; l; ++l) // top left side
				*(newMaskPtr++) = front * (double)(j * j) + top * (double)(k * k) + left * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
			for(int l = 0; l <= rightInt; ++l) // top right side
				*(newMaskPtr++) = front * (double)(j * j) + top * (double)(k * k) + right * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
		}
		for(int k = 0; k <= bottomInt; ++k) {
			uchar* newMaskPtr = &(newMask.at<uchar>(frontInt + j, topInt + k, 0));
			for(int l = -leftInt; l; ++l) // bottom left side
				*(newMaskPtr++) = front * (double)(j * j) + bottom * (double)(k * k) + left * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
			for(int l = 0; l <= rightInt; ++l) // bottom right side
				*(newMaskPtr++) = front * (double)(j * j) + bottom * (double)(k * k) + right * (double)(l * l) <= 1.0 ? _TRU_ : _FLS_;
		}
	}
	Range r[] = { Range(frontInt, frontInt + 1), Range(topInt, topInt + 1), Range(leftInt + centerPixelInt, leftInt + rightInt + 1) };
	if(rightInt || !centerPixelInt) newMask(r) = Scalar_<uchar>(_FLS_);
	r[1] = Range(topInt + 1, topInt + bottomInt + 1); r[2] = Range(0, leftInt + rightInt + 1);
	if(bottomInt) newMask(r) = Scalar_<uchar>(_FLS_);
	return StructuringElement(newMask, Point3i(leftInt, topInt, frontInt));
} // end StructuringElement::createHalfEllipsoidElement

StructuringElement StructuringElement::createHalfEllipseElementMultichannel(double top, double left, double right, unsigned int channels, const bool centerPixel) {
	StructuringElement elipse2d = createHalfEllipseElement(top, left, right, centerPixel);
	int sz[] = {channels + 1, elipse2d.getRows(), elipse2d.getCols()};
	Mat newMask(3, sz, CV_8U, Scalar_<uchar>(_FLS_));
	Range r[] = {Range(channels, channels + 1), Range::all(), Range::all()};
	elipse2d.getMask().copyTo(newMask(r));
	for(unsigned int j = 0; j < channels; ++j) // for each channel to predict from, add one neighborhood element
		newMask.at<uchar>(j, elipse2d.getTop(), elipse2d.getLeft()) = _TRU_;
	return StructuringElement(newMask, Point3i(elipse2d.getLeft(), elipse2d.getTop(), channels));
} // end StructuringElement::createHalfEllipseElementMultichannel

StructuringElement StructuringElement::createHalfEllipseElementMultichannelForward(double top, double left, double right, unsigned int channels, const bool centerPixel) {
	StructuringElement elipse2d = createHalfEllipseElement(top, left, right, centerPixel);
	int sz[] = {channels + 1, elipse2d.getRows(), elipse2d.getCols()};
	Mat newMask(3, sz, CV_8U, Scalar_<uchar>(_FLS_));
	Range r[] = {Range(channels, channels + 1), Range(0, elipse2d.getRows()), Range::all()};
	elipse2d.getMask().copyTo(newMask(r));
	for(unsigned int j = 0; j < channels; ++j) { // for each channel to predict from, add five neighborhood elements
		newMask.at<uchar>(j, elipse2d.getTop(), elipse2d.getLeft()) = _TRU_;
		newMask.at<uchar>(j, elipse2d.getTop() - 2, elipse2d.getLeft()) = _TRU_;
		newMask.at<uchar>(j, elipse2d.getTop() - 1, elipse2d.getLeft()) = _TRU_;
		newMask.at<uchar>(j, elipse2d.getTop(), elipse2d.getLeft() + 1) = _TRU_;
		newMask.at<uchar>(j, elipse2d.getTop(), elipse2d.getLeft() - 1) = _TRU_;
	}
	newMask.at<uchar>(0, elipse2d.getTop() - 2, elipse2d.getLeft()) = _FLS_;
	return StructuringElement(newMask, Point3i(elipse2d.getLeft(), elipse2d.getTop(), channels));
} // end StructuringElement::createHalfEllipseElementMultichannelForward

int StructuringElement::increment(Point3i& position) const {
	const uchar* maskPtr = position.x < 0 ? mask.ptr<uchar>() - 1 : &(mask.at<uchar>(position.z, position.y, position.x));
	while(++(position.x) < mask.size[2]) if(*(++maskPtr)) return 0; // check current row for next _TRU_ element
	while(++(position.y) < mask.size[1]) { // check other rows in current slice
		maskPtr = &(mask.at<uchar>(position.z, position.y, 0));
		for(position.x = 0; position.x < mask.size[2]; position.x++) if(*(maskPtr++)) return 0;
	}
	while(++(position.z) < mask.size[0]) { // check other slices
		for(position.y = 0; position.y < mask.size[1]; position.y++) {
			maskPtr = &(mask.at<uchar>(position.z, position.y, 0));
			for(position.x = 0; position.x < mask.size[2]; position.x++) if(*(maskPtr++)) return 0;
		}
	}
	return 1; // no more _TRU_ elements found
} // end StructuringElement::increment

Mat StructuringElement::extractVectorFromPatch(const Mat& patch) const {
	Mat destination = Mat(1, numel, CV_64F);
	extractVectorFromImage(patch, anchor, destination.ptr<double>());
	return destination;
} // end StructuringElement::extractVectorFromPatch

void StructuringElement::extractVectorFromPatch(const Mat& patch, double* destination) const {
	extractVectorFromImage(patch, anchor, destination);
} // end StructuringElement::extractVectorFromPatch

Mat StructuringElement::extractVectorFromImage(const Mat& image, const Point3i& position) const {
	Mat destination = Mat(1, numel, CV_64F);
	extractVectorFromImage(image, position, destination.ptr<double>());
	return destination;
} // end StructuringElement::extractVectorFromImage

void StructuringElement::extractVectorFromImage(const Mat& image, Point3i position, double* destination) const {
	position -= anchor;
	for(int j = 0; j < mask.size[0]; ++j)
		for(int k = 0; k < mask.size[1]; ++k) {
			const uchar* maskPtr = &(mask.at<uchar>(j, k, 0));
			const double* patchPtr = &(image.at<double>(position.z + j, position.y + k, position.x));
			for(int l = 0; l < mask.size[2]; ++l) {
				if(*(maskPtr++)) *(destination++) = *(patchPtr++);
				else ++patchPtr;
			}
		}
} // end StructuringElement::extractVectorFromImage

void StructuringElement::extractVectorFromImageBorderSafe(const Mat& image, const Point3i& position, Mat& destination) {
	destination.create(1, numel, CV_64F);
	Mat oldMask = mask;
	Point3i oldAnchor = anchor;
	int total = mask.total();
	if(position.x < anchor.x) { // crop left
		Range r[] = { Range::all(), Range::all(), Range(anchor.x - position.x, mask.size[2]) }; setMask(mask(r)); anchor.x = position.x; }
	if(image.size[2] - position.x <= (int)getRight()) { // crop right
		Range r[] = { Range::all(), Range::all(), Range(0, anchor.x + image.size[2] - position.x) }; setMask(mask(r)); }
	if(position.y < anchor.y) { // crop top
		Range r[] = { Range::all(), Range(anchor.y - position.y, mask.size[1]), Range::all() }; setMask(mask(r)); anchor.y = position.y; }
	if(image.size[1] - position.y <= (int)getBottom()) { // crop bottom
		Range r[] = { Range::all(), Range(0, anchor.y + image.size[1] - position.y), Range::all() }; setMask(mask(r)); }
	if(position.z < anchor.z) { // crop front
		Range r[] = { Range(anchor.z - position.z, mask.size[0]), Range::all(), Range::all() }; setMask(mask(r)); anchor.z = position.z; }
	destination = destination.colRange(0, numel);
	extractVectorFromImage(image, position, destination.ptr<double>());
	if(total != mask.total()) { setMask(oldMask); anchor = oldAnchor; }
} // end StructuringElement::extractVectorFromImageBorderSafe

void StructuringElement::computeHistogramFromImageBorderSafe(const Mat& image, Point3i position, Mat& histogram) {
	Mat oldMask = mask;
	Point3i oldAnchor = anchor;
	int total = mask.total();
	if(position.x < anchor.x) { // crop left
		Range r[] = { Range::all(), Range::all(), Range(anchor.x - position.x, mask.size[2]) }; setMask(mask(r)); anchor.x = position.x; }
	if(image.size[2] - position.x <= (int)getRight()) { // crop right
		Range r[] = { Range::all(), Range::all(), Range(0, anchor.x + image.size[2] - position.x) }; setMask(mask(r)); }
	if(position.y < anchor.y) { // crop top
		Range r[] = { Range::all(), Range(anchor.y - position.y, mask.size[1]), Range::all() }; setMask(mask(r)); anchor.y = position.y; }
	if(image.size[1] - position.y <= (int)getBottom()) { // crop bottom
		Range r[] = { Range::all(), Range(0, anchor.y + image.size[1] - position.y), Range::all() }; setMask(mask(r)); }
	if(position.z < anchor.z) { // crop front
		Range r[] = { Range(anchor.z - position.z, mask.size[0]), Range::all(), Range::all() }; setMask(mask(r)); anchor.z = position.z; }
	histogram = Scalar_<int>(0);
	int* histogramPtr = histogram.ptr<int>();
	position -= anchor;
	for(int j = 0; j < mask.size[0]; ++j)
		for(int k = 0; k < mask.size[1]; ++k) {
			const uchar* maskPtr = &(mask.at<uchar>(j, k, 0));
			const double* patchPtr = &(image.at<double>(position.z + j, position.y + k, position.x));
			for(int l = 0; l < mask.size[2]; ++l) {
				if(*(maskPtr++)) histogramPtr[(int)*(patchPtr++)]++;
				else ++patchPtr;
			}
		}
	if(total != mask.total()) { setMask(oldMask); anchor = oldAnchor; }
} // end StructuringElement::computeHistogramFromImageBorderSafe

} // end namespace vanilc

