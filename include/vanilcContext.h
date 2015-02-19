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
#include <cmath>

#include "vanilcStructuringElement.h"

// in border regions, neighborhood and training region need to share the available pixels: choose which ratio the neighborhood gets (in 2^-n)
const unsigned int    TOP_BORDER_NEIGHBORHOOD_RATIO = 1;
const unsigned int   LEFT_BORDER_NEIGHBORHOOD_RATIO = 1;
const unsigned int  RIGHT_BORDER_NEIGHBORHOOD_RATIO = 1;
const unsigned int BOTTOM_BORDER_NEIGHBORHOOD_RATIO = 1;
const unsigned int  FRONT_BORDER_NEIGHBORHOOD_RATIO = 0;

namespace vanilc {

using namespace std;
using namespace cv;

class Context {
public:
	Context() :
		neighborhood(StructuringElement(Mat(), Point3i(-1, -1, -1))), trainingregion(StructuringElement(Mat(), Point3i(-1, -1, -1))),
		fullNeighborhood(StructuringElement(Mat(), Point3i(-1, -1, -1))), fullTrainingregion(StructuringElement(Mat(), Point3i(-1, -1, -1))),
		image(NULL), buffer(NULL), imagePosition(Point3i(-1, -1, -1)), contextPosition(Point3i(-1, -1, -1)),
		border(false), croppedNeighborhood(false), useBuffer(false) {};
	Context(StructuringElement& neighborhood, StructuringElement& trainingregion) :
		neighborhood(neighborhood), trainingregion(trainingregion),
		fullNeighborhood(neighborhood), fullTrainingregion(trainingregion),
		image(NULL), buffer(NULL), imagePosition(Point3i(-1, -1, -1)), contextPosition(Point3i(-1, -1, -1)),
		border(false), croppedNeighborhood(false), useBuffer(false) {};
	~Context() { bufferOff(); };

	void setNeighborhood(const StructuringElement& neighborhood) { this->neighborhood = neighborhood; border = true; croppedNeighborhood = true; useBuffer = false; };
	void setFullNeighborhood(const StructuringElement& neighborhood) { bufferOff(); this->neighborhood = neighborhood; fullNeighborhood = neighborhood; };
	const StructuringElement& getNeighborhood() const { return neighborhood; };
	const StructuringElement& getFullNeighborhood() const { return fullNeighborhood; };
	void setTrainingregion(const StructuringElement& trainingregion) { this->trainingregion = trainingregion; border = true; };
	void setFullTrainingregion(const StructuringElement& trainingregion) { this->trainingregion = trainingregion; fullTrainingregion = trainingregion; };
	const StructuringElement& getTrainingregion() const { return trainingregion; };
	const StructuringElement& getFullTrainingregion() const { return fullTrainingregion; };
	void setImage(const Mat* image) { bufferOff(); this->image = image; };
	const Mat* getImage() const { return image; };
	bool isBorder() const { return border; };

	unsigned int getCols() const { return trainingregion.getCols() + neighborhood.getCols() - 1; };
	unsigned int getRows() const { return trainingregion.getRows() + neighborhood.getRows() - 1; };
	unsigned int getSlcs() const { return trainingregion.getSlcs() + neighborhood.getSlcs() - 1; };
	unsigned int getLeft() const { return trainingregion.getLeft() + neighborhood.getLeft(); };
	unsigned int getRight() const { return trainingregion.getRight() + neighborhood.getRight(); };
	unsigned int getTop() const { return trainingregion.getTop() + neighborhood.getTop(); };
	unsigned int getBottom() const { return trainingregion.getBottom() + neighborhood.getBottom(); };
	unsigned int getFront() const { return trainingregion.getFront() + neighborhood.getFront(); };
	unsigned int getBack() const { return trainingregion.getBack() + neighborhood.getBack(); };

	// buffers already computed context elements but needs lots of memory;
	// must be turned on manually after each change of image, neighborhood or training region
	void bufferOn() { if(!buffer) buffer = new Mat(image->total(), neighborhood.getNumberOfElements(), CV_64F, numeric_limits<double>::quiet_NaN()); };
	void bufferOff() { if(buffer) { delete buffer; buffer = NULL; } };
	bool getBuffered() const { return (bool)buffer; };

	// destination matrix is not required to be allocated
	void contextOf(const Point3i& position, Mat& destination) const;
	void getContextElementsOf(const Point3i& position = Point3i(-1, -1, -1)); // without argument use same position again as before
	int getNextContextElement(Mat& destination);
	void checkBorder(const Point3i& position);

private:
	StructuringElement neighborhood, trainingregion, fullNeighborhood, fullTrainingregion;
	const Mat* image;
	Mat* buffer;
	Point3i imagePosition, contextPosition;
	bool border, croppedNeighborhood, useBuffer;
};

class PositionNotSetException : public Exception {
	virtual const char* what() const throw() { return "You must set a new position using the function getContextElementsOf() before obtaining an element."; }
};

} // end namespace vanilc

