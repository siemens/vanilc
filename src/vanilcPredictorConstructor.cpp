// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcPredictorConstructor.h"

namespace vanilc {

Predictor* PredictorConstructor::constructMeanpredictor(Config& config, const Context& context) {
	Predictor* meanpredictor = new Predictor(context);
	meanpredictor->setPredictionComputer(new MeanPredictionComputer);
	if(config.get<string>("variance") == "RESIDUAL")
		meanpredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		meanpredictor->setVarianceComputer(new ExponentialVarianceComputer);
	return meanpredictor;
} // end PredictorConstructor::constructMeanpredictor

Predictor* PredictorConstructor::constructMEDpredictor(Config& config, const Context& context) {
	Predictor* medpredictor = new Predictor(context);
	medpredictor->setPredictionComputer(new MEDPredictionComputer);
	if(config.get<string>("variance") == "RESIDUAL")
		medpredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		medpredictor->setVarianceComputer(new ExponentialVarianceComputer);
	return medpredictor;
} // end PredictorConstructor::constructMEDpredictor

Predictor* PredictorConstructor::constructNLMpredictor(Config& config, const Context& context) {
	Predictor* nlmpredictor = new Predictor(context);
	nlmpredictor->setPredictionComputer(new NLMPredictionComputer(ExponentialSADWeightingFunction(.146))); // decay = .146 is the value from the paper, however smaller values (~.05) seem better
	if(config.get<string>("variance") == "RESIDUAL")
		nlmpredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		nlmpredictor->setVarianceComputer(new ExponentialVarianceComputer);
	return nlmpredictor;
} // end PredictorConstructor::constructNLMpredictor

Predictor* PredictorConstructor::constructFastLSpredictor(Config& config, const Context& context) {
	Predictor* fastlspredictor = new Predictor(context);
	Mat* covMat = new Mat; Mat* coefficients = new Mat; Mat* weights = new Mat;
	fastlspredictor->setPredictionComputer(new FastLSPredictionComputer(covMat, coefficients, weights,
		config.get<double>("border_regularization"), config.get<double>("inner_regularization"), config.get<int>("solver")));
	if(config.get<string>("variance") == "LS")
		fastlspredictor->setVarianceComputer(new LSVarianceComputer(covMat, coefficients, weights, 0));
	else if(config.get<string>("variance") == "RESIDUAL")
		fastlspredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		fastlspredictor->setVarianceComputer(new ExponentialVarianceComputer);
	fastlspredictor->setDegreesOfFreedomComputer(new LSDegreesOfFreedomComputer(covMat));
	return fastlspredictor;
} // end PredictorConstructor::constructFastLSpredictor

Predictor* PredictorConstructor::constructLSpredictor(Config& config, const Context& context) {
	Predictor* lspredictor = new Predictor(context);
	Mat* covMat = new Mat; Mat* coefficients = new Mat; Mat* weights = new Mat;
	lspredictor->setPredictionComputer(new LSPredictionComputer(covMat, coefficients, weights, IdentityWeightingFunction(),
		config.get<double>("border_regularization"), config.get<double>("inner_regularization"), config.get<int>("wls_variance_equation"), config.get<int>("solver"), 0));
	if(config.get<string>("variance") == "LS")
		lspredictor->setVarianceComputer(new LSVarianceComputer(covMat, coefficients, weights, 0));
	else if(config.get<string>("variance") == "RESIDUAL")
		lspredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		lspredictor->setVarianceComputer(new ExponentialVarianceComputer);
	lspredictor->setDegreesOfFreedomComputer(new LSDegreesOfFreedomComputer(covMat));
	return lspredictor;
} // end PredictorConstructor::constructWLSpredictor

Predictor* PredictorConstructor::constructWLSpredictor(Config& config, const Context& context, Context* weightingContext) {
	Predictor* wlspredictor = new Predictor(context);
	Mat* covMat = new Mat; Mat* coefficients = new Mat; Mat* weights = new Mat;
	WeightingFunction *weightingFunction, *otherWeightingFunction;
	if(config.get<int>("max_training_vectors")) { // don't use all training positions within training region
		weightingFunction = new CroppedPriorizedSSDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(context.getNeighborhood()) * 60.0,
			config.get<int>("max_training_vectors"));
		if(weightingContext) otherWeightingFunction = new CroppedPriorizedSSDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(weightingContext->getNeighborhood()) * 60.0,
			config.get<int>("max_training_vectors"));
	} else if(config.get<int>("wls_variance_equation")) { // squared weights for variance estimation
		weightingFunction = new InversePriorizedSSDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(context.getNeighborhood()) * 2.0 * 60.0);
		if(weightingContext) otherWeightingFunction = new InversePriorizedSSDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(weightingContext->getNeighborhood()) * 2.0 * 60.0);
	} else { // non-squared weights for variance estimation
		weightingFunction = new InversePriorizedSQDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(context.getNeighborhood()) * 60.0);
		if(weightingContext) otherWeightingFunction = new InversePriorizedSQDWeightingFunction(
			InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(weightingContext->getNeighborhood()) * 60.0);
	}
	if(weightingContext) { // different context for weight computation
		wlspredictor->setPredictionComputer(new LSPredictionComputer(covMat, coefficients, weights, *weightingFunction,
			weightingContext, *otherWeightingFunction,
			config.get<double>("border_regularization"), config.get<double>("inner_regularization"),
			config.get<int>("wls_variance_equation"), config.get<int>("solver"), config.get<int>("max_training_vectors")));
		delete otherWeightingFunction;
	} else
		wlspredictor->setPredictionComputer(new LSPredictionComputer(covMat, coefficients, weights, *weightingFunction,
			config.get<double>("border_regularization"), config.get<double>("inner_regularization"),
			config.get<int>("wls_variance_equation"), config.get<int>("solver"), config.get<int>("max_training_vectors")));
	delete weightingFunction;
	if(config.get<string>("variance") == "LS")
		wlspredictor->setVarianceComputer(new LSVarianceComputer(covMat, coefficients, weights, config.get<int>("wls_variance_equation")));
	else if(config.get<string>("variance") == "RESIDUAL")
		wlspredictor->setVarianceComputer(new ResidualVarianceComputer(config.get<double>("variance_radius")));
	else
		wlspredictor->setVarianceComputer(new ExponentialVarianceComputer);
	wlspredictor->setDegreesOfFreedomComputer(new LSDegreesOfFreedomComputer(covMat));
	return wlspredictor;
} // end PredictorConstructor::constructWLSpredictor

} // end namespace vanilc

