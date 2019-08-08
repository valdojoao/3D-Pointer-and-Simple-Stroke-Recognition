/*
------------------------------------------------------------------------------------------------------------------------------------------
Description   : Image processing functions
------------------------------------------------------------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------------------
Includes
--------------------------------------------------------------------------------------------------
*/

#include "ImageProcessing.h"
#include <numeric> //	For vector accumulation
#include <cstdlib>

/*
--------------------------------------------------------------------------------------------------
Functions definition
--------------------------------------------------------------------------------------------------
*/

ImageProc::ImageProc()
{
    //	Initialize all elements to UINT16_MAX (max depth distance) - CV_16UC1 => 16 - bits , 1 channel
    RawThreshImg = cv::Mat(240, 320, CV_16UC1, cv::Scalar::all(UINT16_MAX));
    RawWriter = cv::Mat(240, 320, CV_16UC3, cv::Scalar::all(UINT16_MAX));
    RawMeanBlob = cv::Mat(240, 320, CV_16UC3, cv::Scalar::all(UINT16_MAX));

    //	Hist, CumHist, HistEqu variables
    flag = false;
    EquIndex = 0;
    RandThresInit = 0;
    NewThresh = 0;
    Adder = 0;
}

// -------------------------------------------------------------------------------
// Calculates optimized threshold using histogram equilization by iterations
// -------------------------------------------------------------------------------
void ImageProc::HistEqu(cv::Mat* ImgSrc, int32_t* Thres, uint16_t bins)
{
    this->Hist(ImgSrc, &Equhist, bins); 							//	Calculate histogram
    this->CumHist(&Equhist, &EquCumHist); 				//	Calculate cumulative histogram
    RandThresInit = 0; //	Initialize Random Threshold

    for (int i = 0; i < discValues.cols - 1; i++) { 				//	Find the sum of discrete values matrix
        RandThresInit = RandThresInit + (uint32_t)discValues.at<uint16_t>(i);
    }

    RandThresInit = RandThresInit / discValues.cols; 	//	Calculate the init threshold (The mean value)

    while (NewThresh != RandThresInit) {
        NewThresh = RandThresInit; 		//	In every loop update the threshold with the new threshold
        EquIndex = 0;
        Adder = 0;

        for (int16_t i = 0; i < bins; i++) { 	// Find how many values in histogram are below the threshold.
            if (discValues.at<uint16_t>(i) < RandThresInit) {
                EquIndex += 1;
            }
        }

        if (EquIndex == 0)
            EquIndex = 1;

        // Dynamic allocate matrices for R1 histogram and R1 cumulative histogram
        cv::Mat R1_Histogram = cv::Mat::zeros(1, EquIndex, CV_32S);
        cv::Mat R1_Cumulative = cv::Mat::zeros(1, EquIndex, CV_32S);

        //	Fill R1 histogram with values that belong below the threshold
        for (int16_t i = 0; i < EquIndex; i++) {
            R1_Histogram.at<int32_t>(i) = Equhist.at<int32_t>(i);
        }
        this->CumHist(&R1_Histogram, &R1_Cumulative); 	// Calculate cumulative histogram (for normalization)

        //	Multiplay each bin value with the corresponding depths
        for (int16_t i = 0; i < EquIndex; i++) {
            Adder = (R1_Histogram.at<int32_t>(i) * discValues.at<uint16_t>(i)) + Adder;
        }

        //	Calculate center (mean value) of R1 histogram (Normalized)
        m1 = (float)Adder / R1_Cumulative.at<int32_t>(EquIndex - 1);
        EquIndex = 0;
        Adder = 0;

        // Find how many values in histogram are over or equal to the threshold.
        for (int16_t i = 0; i < bins; i++) {
            if (discValues.at<uint16_t>(i) >= RandThresInit) {
                EquIndex += 1;
            }
        }

        if (EquIndex == 0)
            EquIndex = 1;

        // Dynamic allocate matrices for R2 histogram and R2 cumulative histogram
        cv::Mat R2_Histogram = cv::Mat::zeros(1, EquIndex, CV_32S);
        cv::Mat R2_Cumulative = cv::Mat::zeros(1, EquIndex, CV_32S);

        //	Fill R2 histogram with values that belong over or equal to the threshold
        for (int16_t i = R1_Histogram.cols; i < bins; i++) {
            R2_Histogram.at<int32_t>(i - R1_Histogram.cols) = Equhist.at<int32_t>(i);
        }
        this->CumHist(&R2_Histogram, &R2_Cumulative); 	// Calculate cumulative histogram (for normalization)

        for (int16_t i = R1_Histogram.cols; i < discValues.cols; i++) {
            Adder = (R2_Histogram.at<int32_t>(i - R1_Histogram.cols) * discValues.at<uint16_t>(i)) + Adder;
        }

        //	Calculate center (mean value) of R1 histogram (Normalized)
        m2 = (float)Adder / R2_Cumulative.at<int32_t>(EquIndex - 1);

        //	Calculate mean of means (new threshlod)
        RandThresInit = (int32_t)(m1 + m2) / 2;

        //	Free the matrices from memory for re-allocation
        R1_Histogram.release();
        R2_Histogram.release();
        R1_Cumulative.release();
        R2_Cumulative.release();
    }
    *Thres = RandThresInit; //	Return the address of the final threshold
    return;
}

// -------------------------------------------------------------------------------
// Calculates histogram of a uint16 image using bins. Works like calcHist()
// -------------------------------------------------------------------------------
void ImageProc::Hist(cv::Mat* ImgSrc, cv::Mat* HistDst, uint16_t bins)
{
    cv::Mat HistDstTmp = cv::Mat::zeros(1, bins, CV_32S); 			// Range ‭(-2147483648~2147483647)‬ Initialize the hist Mat to all zeros (size of bins - Matlab style)
    discValues = cv::Mat::zeros(1, bins, CV_16UC1); 						// Initialize the discrete values zeros (size of bins)
    cv::minMaxLoc((*ImgSrc), &min, &max); 									// Find the maximum and the minimum value of source image to fix the range of Histogram
    steps = max / (bins); 																		// Steps hold the intermediate hist values for comparison
    discValues.at<uint16_t>(0) = 1; 													// Discrete values first element to one (avoid zero counting in histogram)

    for (int index_ = 1; index_ < bins; index_++) 								// Fill discValues with step gaps
        discValues.at<uint16_t>(index_) = (uint16_t)(index_ * steps);

    for (int r = 0; r < ImgSrc->rows; r++) { 										// Fill the histogram values
        for (int c = 0; c < ImgSrc->cols; c++) {
            flag = false;

            for (Index = 0; Index < bins - 1; Index++) { 							// For every bin (until bin-1) check if the pixels falls in the gap
                if (ImgSrc->at<uint16_t>(r, c) >= discValues.at<uint16_t>(Index) && ImgSrc->at<uint16_t>(r, c) < discValues.at<uint16_t>(Index + 1)) {
                    HistDstTmp.at<int32_t>(Index) += 1; 							//	(int32_t <=> CV_32S) increase the histogram in the specific gap
                    flag = true;
                }
            }

            if (flag == false) {                                       //	If flag is false, it means that the pixel value did not fall in the gap (Index increased in the last for loop)
                HistDstTmp.at<int32_t>(Index) += 1;    //	(int32_t=CV_32S) So it belongs to the final most right gap
            }
        }
    }
    (*HistDst) = HistDstTmp;
    HistDstTmp.release();
    return;
}

// ------------------------------------
// Calculates the cumulative histogram
// ------------------------------------
void ImageProc::CumHist(cv::Mat* HistSrc, cv::Mat* CumHistDst)
{
    (*HistSrc).copyTo(*CumHistDst); 									//	Copy the source histogram to the cumulative histogram
    for (Index = 1; Index < HistSrc->cols; Index++) { 			//	For every bin add the current src and the previous dst histograms
        CumHistDst->at<int32_t>(Index) = HistSrc->at<int32_t>(Index) + CumHistDst->at<int32_t>(Index - 1);
    }
    return;
}


// ------------------------------------
// Threshhold implementation
// ------------------------------------
void ImageProc::Thres(cv::Mat& ImgSrc, cv::Mat& ImgDest, int32_t* Thres)
{
	
    ImgSrc.copyTo(ImgDest);
    for (int r = 0; r < ImgSrc.rows; r++) { 								// Threshold ImgSrcCopy using Thres & copy the crusial pixels from ImgSrc to ImgDest
        for (int c = 0; c < ImgSrc.cols; c++) {
            if (ImgSrc.at<uint16_t>(r, c) < (*Thres - 14850)) { 	// Constant threshold should be adjusted with respect to image depths 
               ImgDest.at<uint16_t>(r, c) = UINT16_MAX;  		// increase the pixel intensity
            }
            else {
                ImgDest.at<uint16_t>(r, c) = ImgSrc.at<uint16_t>(r, c);
            }
        }
    }
    return;
}

// --------------------------------------------------------
// Center of Mass detection 
// --------------------------------------------------------
void ImageProc::BlobAnalysis(cv::Mat& ImgSrc, cv::Mat& ImgDest)
{
	
	//We will not use the threshold image directly we will use a copy in order to  not have intererence on the display window
	cv::Mat temp;
	ImgSrc.copyTo(ImgDest);
	
	cvtColor(ImgDest, ImgDest, CV_GRAY2RGB, 3);
	
	//do not use the entire window, we will use a ROI instead
	cv::Rect ROI_I = cv::Rect(ImgSrc.rows - (ImgSrc.rows - 20), ImgSrc.cols - (ImgSrc.cols - 20), (ImgSrc.cols - 40), (ImgSrc.rows - 40));
    temp = ImgSrc(ROI_I);
	
	//cv::rectangle(ImgDest, ROI_I, cv::Scalar(90000, 90000, 90000), 2, cv::LINE_8, 0);	  //see the ROI window
	
	
	// Divide the screen in two vertical parts
	cv::Point mid_bottom, mid_top;
    mid_bottom.x = ImgDest.rows/2 + 40;
    mid_bottom.y = 0;
    mid_top.x = ImgDest.rows/2 + 40;
    mid_top.y = ImgDest.cols;	
	//cv::line(ImgDest,mid_bottom,mid_top,cv::Scalar(90000, 90000, 90000));  //see the line diving the screen in two parts
	
	//binarization inRange function,  operator sets to 1 all the pixels contained between the low and high thresholds and to 0 all the other pixels.
	cv::inRange(temp, UINT16_MAX, UINT16_MAX, temp);  
	
	/*
	After binarization the image is a bit noisy because of false positives. 
	to clean the image and remove false positives, an opening operator is applied, with a 3x3 circular structuring element.
	a dilation is also applied, just in case parts of the hand have been detached after binarization 
	*/	
	 this->DilateAndErode(temp);	 
	 
	//Initializing two vectors to be used on the findContours function
	std::vector<std::vector<cv::Point> > contours;
	
	//Find contours i.e curve joining all the continuous points having same intensity
	cv::findContours(temp, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	int min_contourArea = 600, max_contourArea = 6000;     //Min and Max contour area 
	
	for (int i = 0; i < contours.size();) {
        
        if (contourArea(contours[i]) > min_contourArea && contourArea(contours[i]) < max_contourArea) {
			
            //draw all the contours found 
            cv::drawContours(ImgDest(ROI_I), contours, i, cv::Scalar(30000, 50000, 70000), CV_FILLED, 8);	
			i++;             
         } else 
         {
             contours.erase(contours.begin() + i);   //remove the contour from the list
          }
    }


	if (!contours.empty())
		{
			//sort in descending order the contours found taking into account its area
			std::sort(contours.begin(), contours.end(), [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2){
                return contourArea(c1, false) > contourArea(c2, false);
                });        					
			
			//get the biggest area
			std::vector<double> massOneHand = this->CalculateMoments(contours, 0);  //find the center of mass
				
			 int x = massOneHand.at(0);
			 int y = massOneHand.at(1);
				
			oneHand.push_back(std::make_pair(x, y));
			cv::circle(ImgDest(ROI_I), cv::Point(x, y), 5, cv::Scalar(90000, 10000, 20000), -1, 8, 0);
		
			//look for the second biggest area in order to manage the two hands case
			if(contours.size() > 1) {
					
					//get the second biggest area
					std::vector<double> massTwoHands = this->CalculateMoments(contours, 1); //find the center of mass
						
					 int x1 = massTwoHands.at(0);
					 int y2 = massTwoHands.at(1);
					 
					 //look for the points located in the left and right taking into account the line that  divies the screen in two vertical parts
					 if(x1 <= mid_bottom.x  && x > mid_bottom.x)
					{
						leftHand.push_back(std::make_pair(x1, y2));
						rightHand.push_back(std::make_pair(x, y));
						cv::circle(ImgDest(ROI_I), cv::Point(x1, y2), 5, cv::Scalar(30000, 90000, 70000), -1, 8, 0);
					}
					
					else
						{  
							if(x1 >= mid_bottom.x  && x < mid_bottom.x)
							{
							   leftHand.push_back(std::make_pair(x, y));
							   rightHand.push_back(std::make_pair(x1, y2));
							   cv::circle(ImgDest(ROI_I), cv::Point(x1, y2), 5, cv::Scalar(30000, 90000, 70000), -1, 8, 0);
						   }	
						}	
		    }
	 }	
	 //	Free the matrices from memory for re-allocation
	  temp.release();			
}


//Function to apply the erode and dilate features.
void ImageProc::DilateAndErode(cv::Mat &image) {
 
	//Defining the erode and dilate properties
	//the erode element chosen here is a 3x3 piexels rectangle.
	//dilate with 8x8 size element to make the threshold object more visible
 
	cv::Mat erodeElement = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::Mat dilateElement = getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));
 
	//Apply erode and dilate
	cv::erode(image, image, erodeElement);
	cv::dilate(image, image, dilateElement); 
}

// --------------------------------------------------------
// Center of Mass detection 
// --------------------------------------------------------
std::vector<double> ImageProc::CalculateMoments(std::vector<std::vector<cv::Point>>& contours, int i)
{
	cv::Moments m;
	m = moments((cv::Mat) contours[i]);
				
	double area = m.m00;
				
	int x = m.m10 / area; 		//center of mass column
	int y = m.m01 / area; 		//center of mass row
	
	std::vector<double> massCenter;
	
	massCenter.push_back(x);
	massCenter.push_back(y);
	
	return massCenter;	
}
	