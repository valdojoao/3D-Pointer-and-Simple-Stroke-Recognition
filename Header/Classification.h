/*
------------------------------------------------------------------------------------------------------------------------------------------
Project       		: Depth Stroke Recognition
File          		: Classification.h
Description   	: This code uses unistroke $1 gesture recognition library created by Baylor Wetzeli
------------------------------------------------------------------------------------------------------------------------------------------
*/
#pragma once

/*
----------------------------------------------------------------------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------------------------------------------------------------------
*/
#include "GeometricRecognizer.h"
#include <array> // for std::array

/*
--------------------------------------------------------------------------------------------------
Class Declaration
--------------------------------------------------------------------------------------------------
*/
class Classification {

public:

	/*
	--------------------------------------------------------------------------------------------------
	Public Function Prototyping
	--------------------------------------------------------------------------------------------------
	*/
	Classification();

	void Classify(std::vector<double> DecPoints, const std::string& ClassifierStatus, const std::string& hand);
	void ClassifySample(std::vector<std::pair<double, double>> &handPoints);														//function added by myself
	std::vector<double> DecisionPoints;

	//	Recognition
	DollarRecognizer::GeometricRecognizer GeoRec;
	DollarRecognizer::SampleGestures SampleGest;
	DollarRecognizer::Path2D Path2DTest;
	DollarRecognizer::Point2D Point2DTest;

	DollarRecognizer::GeometricRecognizer *GeoR;
	DollarRecognizer::SampleGestures *SampleG;
	DollarRecognizer::Path2D *Path2DT;
	DollarRecognizer::Point2D *Point2DT;

};
