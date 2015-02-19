// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcContext.h"

namespace vanilc {

void Context::contextOf(const Point3i& position, Mat& destination) const {
	if(useBuffer) {
		destination = buffer->row(position.z * image->size[1] * image->size[2] + position.y * image->size[2] + position.x); // create matrix wrapper around buffer row as return value
		double* bufPtr = destination.ptr<double>();
		#ifdef WIN32
			if(_isnan(bufPtr[neighborhood.getNumberOfElements() - 1])) neighborhood.extractVectorFromImage(*image, position, bufPtr); // fill it with neighborhood of pixel at position
		#else
			if(isnan(bufPtr[neighborhood.getNumberOfElements() - 1])) neighborhood.extractVectorFromImage(*image, position, bufPtr); // fill it with neighborhood of pixel at position
		#endif
	} else destination = neighborhood.extractVectorFromImage(*image, position);
} // end Context::contextOf

void Context::getContextElementsOf(const Point3i& position) {
	if(position.x != -1) imagePosition = position; // without argument leave position the same as before
	else if(imagePosition.x == -1) throw PositionNotSetException(); // ...but giving no argument is only allowed if position was set at least once before
	contextPosition = Point3i(-1, 0, 0);
} // end Context::getFirstContextElementOf

int Context::getNextContextElement(Mat& destination) {
	if(contextPosition.y == -1) throw PositionNotSetException();
	if(trainingregion.increment(contextPosition)) { // no more elements left in training region
		contextPosition = Point3i(-1, -1, -1);
		return 1;
	}
	contextOf(imagePosition - trainingregion.getAnchor() + contextPosition, destination);
	return 0;
} // end Context::getNextContextElement

// in border regions shrink neighborhood and training region
void Context::checkBorder(const Point3i& position) {
	if(border) { neighborhood = fullNeighborhood; trainingregion = fullTrainingregion; border = false; croppedNeighborhood = false; } // reset context if previous prediction was at a border pixel
	// contextProtrusion: number of pixels the context (neighborhood plus trainingregion) protrudes beyond the image border - and must therefore be removed from the context
	int contextProtrusion = getLeft() - position.x;
	if(contextProtrusion > 0) { // left border treatment
		border = true;
		int colsToRemove = neighborhood.getLeft() - (position.x - 1 >> LEFT_BORDER_NEIGHBORHOOD_RATIO) - 1; // number of cols to be removed from left of neighborhood mask
		if(colsToRemove <= 0) colsToRemove = 0; else croppedNeighborhood = true;
		Range r[] = { Range::all(), Range::all(), Range(colsToRemove, neighborhood.getCols()) };
		neighborhood = StructuringElement( // give a part of the available cols to neighborhood mask...
			neighborhood.getMask()(r), neighborhood.getAnchor() - Point3i(colsToRemove, 0, 0)); // set new neighborhood mask and anchor
		colsToRemove = contextProtrusion - colsToRemove; // number of cols to be removed from left of training mask
		r[2] = Range(colsToRemove, trainingregion.getCols());
		trainingregion = StructuringElement( // ...and remove the remaining protrusion from the trainingregion mask
			trainingregion.getMask()(r), trainingregion.getAnchor() - Point3i(colsToRemove, 0, 0)); // set new trainingregion mask and anchor
	}
	contextProtrusion = getRight() - (getImage()->size[2] - position.x - 1);
	if(contextProtrusion > 0) { // right border treatment
		border = true;
		int colsToRemove = neighborhood.getRight() - (getImage()->size[2] - position.x - 2 >> RIGHT_BORDER_NEIGHBORHOOD_RATIO) - 1; // number of cols to be removed from right of neighborhood mask
		if(colsToRemove <= 0) colsToRemove = 0; else croppedNeighborhood = true;
		Range r[] = { Range::all(), Range::all(), Range(0, neighborhood.getCols() - colsToRemove) };
		neighborhood = StructuringElement( // give a part of the available cols to neighborhood mask...
			neighborhood.getMask()(r), neighborhood.getAnchor()); // set new neighborhood mask and anchor
		colsToRemove = contextProtrusion - colsToRemove; // number of cols to be removed from right of training mask
		r[2] = Range(0, trainingregion.getCols() - colsToRemove);
		trainingregion = StructuringElement( // ...and remove the remaining protrusion from the trainingregion mask
			trainingregion.getMask()(r), trainingregion.getAnchor()); // set new trainingregion mask and anchor
	}
	contextProtrusion = getTop() - position.y;
	if(contextProtrusion > 0) { // top border treatment
		border = true;
		int rowsToRemove = neighborhood.getTop() - (position.y - 1 >> TOP_BORDER_NEIGHBORHOOD_RATIO) - 1; // number of rows to be removed from top of neighborhood mask
		if(rowsToRemove <= 0) rowsToRemove = 0; else croppedNeighborhood = true;
		Range r[] = { Range::all(), Range(rowsToRemove, neighborhood.getRows()), Range::all() };
		neighborhood = StructuringElement( // give a part of the available rows to neighborhood mask...
			neighborhood.getMask()(r), neighborhood.getAnchor() - Point3i(0, rowsToRemove, 0)); // set new neighborhood and anchor
		rowsToRemove = contextProtrusion - rowsToRemove; // number of rows to be removed from top of training mask
		r[1] = Range(rowsToRemove, trainingregion.getRows());
		trainingregion = StructuringElement( // ...and remove the remaining protrusion from the trainingregion mask
			trainingregion.getMask()(r), trainingregion.getAnchor() - Point3i(0, rowsToRemove, 0)); // set new trainingregion mask and anchor
	}
	if(getFront()) { // 3-D (this if-condition accelerates 2-D coders)
		contextProtrusion = getBottom() - (getImage()->size[1] - position.y - 1);
		if(contextProtrusion > 0) { // bottom border treatment
			border = true;
			int rowsToRemove = neighborhood.getBottom() - (getImage()->size[1] - position.y - 2 >> BOTTOM_BORDER_NEIGHBORHOOD_RATIO) - 1; // number of rows to be removed from bottom of neighborhood mask
			if(rowsToRemove <= 0) rowsToRemove = 0; else croppedNeighborhood = true;
			Range r[] = { Range::all(), Range(0, neighborhood.getRows() - rowsToRemove), Range::all() };
			neighborhood = StructuringElement( // give a part of the available rows to neighborhood mask...
				neighborhood.getMask()(r), neighborhood.getAnchor()); // set new neighborhood and anchor
			rowsToRemove = contextProtrusion - rowsToRemove; // number of rows to be removed from bottom of training mask
			r[1] = Range(0, trainingregion.getRows() - rowsToRemove);
			trainingregion = StructuringElement( // ...and remove the remaining protrusion from the trainingregion mask
				trainingregion.getMask()(r), trainingregion.getAnchor()); // set new trainingregion mask and anchor
		}
		contextProtrusion = getFront() - position.z;
		if(contextProtrusion > 0) { // front border treatment
			border = true;
			int slcsToRemove = neighborhood.getFront() - (position.z - 1 >> FRONT_BORDER_NEIGHBORHOOD_RATIO) - 1; // number of slices to be removed from front of neighborhood mask
			if(slcsToRemove <= 0) slcsToRemove = 0; else croppedNeighborhood = true;
			Range r[] = { Range(slcsToRemove, neighborhood.getSlcs()), Range::all(), Range::all() };
			neighborhood = StructuringElement( // give a part of the available slcs to neighborhood mask...
				neighborhood.getMask()(r), neighborhood.getAnchor() - Point3i(0, 0, slcsToRemove)); // set new neighborhood and anchor
			slcsToRemove = contextProtrusion - slcsToRemove; // number of slcs to be removed from front of training mask
			r[0] = Range(slcsToRemove, trainingregion.getSlcs());
			trainingregion = StructuringElement( // ...and remove the remaining protrusion from the trainingregion mask
				trainingregion.getMask()(r), trainingregion.getAnchor() - Point3i(0, 0, slcsToRemove)); // set new trainingregion mask and anchor
		}
	}
	useBuffer = buffer && !croppedNeighborhood;
} // end Context::checkBorder

} // end namespace vanilc

