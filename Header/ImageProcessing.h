/*
------------------------------------------------------------------------------------------------------------------------------------------
Description   : Image Processing functions
------------------------------------------------------------------------------------------------------------------------------------------
*/
#pragma once

/*
----------------------------------------------------------------------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------------------------------------------------------------------
*/
#include <opencv2/opencv.hpp>
#include <array>

/*
--------------------------------------------------------------------------------------------------
Class Declaration
--------------------------------------------------------------------------------------------------
*/
class ImageProc {

public:
    ImageProc();                                                                                            // Class constructor
    void HistEqu(cv::Mat* ImgSrc, int32_t* Thres, uint16_t bins);              //	Histogram equilization by iterations
    void Thres(cv::Mat& ImgSrc, cv::Mat& ImgDest, int32_t* Thres);      //	Thresholding the image using histogram threshold as input
    void BlobAnalysis(cv::Mat& ImgSrc, cv::Mat& ImgDest);                   //	Detects the Hand and plots a rectangle	

    cv::Mat RawWriter, RawThreshImg, RawBlob, RawMeanBlob;
    int32_t FinalThres;
    std::vector<std::pair<double, double>> oneHand, leftHand, rightHand;

private:
    void Hist(cv::Mat* ImgSrc, cv::Mat* HistDst, uint16_t bins);                //	Calculates histogram of an uint16 image using bins. Works exactly like calcHist()
    void CumHist(cv::Mat* HistSrc, cv::Mat* CumHistDst);                       // Calculates the cumulative histogram
    std::vector<double> CalculateMoments(std::vector<std::vector<cv::Point>>& contours, int i);
    void DilateAndErode(cv::Mat &ImgSrc);                                               // Function to apply the erode and dilate features.

    // ------------------------
    // Hist, CumHist variables
    // ------------------------
    cv::Mat discValues;
    bool flag;
    uint16_t Index;
    double steps, min, max;

    // ------------------------
    // HistEqu variables
    // ------------------------
    cv::Mat Equhist, EquCumHist;
    int32_t EquIndex, RandThresInit, NewThresh;
    float m1, m2;
    uint32_t Adder;
    
};
