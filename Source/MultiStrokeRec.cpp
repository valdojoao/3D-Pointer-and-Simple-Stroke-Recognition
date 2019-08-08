/*
------------------------------------------------------------------------------------------------------------------------------------------
Description   : Stroke Classification based on depth images
------------------------------------------------------------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------------------
Includes
--------------------------------------------------------------------------------------------------
*/
#include "ImageProcessing.h"
#include <iostream>
#include "DirAccess.h"	
#include "Classification.h"

/*
--------------------------------------------------------------------------------------------------
Main Process
--------------------------------------------------------------------------------------------------
*/
int main(int argc, char * argv[])
{

  /*
	--------------------------------------------------------------------------------------------------
	Class Objects
	--------------------------------------------------------------------------------------------------
	*/
  DirAccess dirAccess;
  ImageProc imageProc;
  Classification classification;

for (int Fcounter = 0; Fcounter < dirAccess.Num_Of_Folders; Fcounter++) {               // iterates over all the folders

      // Get images paths
      dirAccess.GetPaths(Fcounter);
      if (dirAccess.DepthPath.empty()){
        std::cout << "Couldn't read images" << '\n';
        return 1;
      }
      
      // sort names in ascend order.
      sort(dirAccess.DepthPath.begin(),dirAccess.DepthPath.end());

      // get images of the specific stroke
      dirAccess.Getimages();

      for (unsigned int Vcounter = 0; Vcounter < dirAccess.DepthImgs.size(); Vcounter++) {  // iterates inside a folder 

      //  Histogram equilization by iterations Threshold calculation + constant threshold in depth
			imageProc.HistEqu(&dirAccess.DepthImgs.at(Vcounter), &imageProc.FinalThres, 32);

      //	Execute thresholding with the calculated value
			imageProc.Thres(dirAccess.DepthImgs.at(Vcounter), imageProc.RawThreshImg, &imageProc.FinalThres);

      //	Track moving nearest object
			imageProc.BlobAnalysis(imageProc.RawThreshImg, imageProc.RawMeanBlob);	

      // Show segmented image + center of mass 
      cv::imshow( dirAccess.stroke_list.at(Fcounter), imageProc.RawMeanBlob );
      cv::waitKey(10);
      }
      
       std::cout << " =========  Stroke " << dirAccess.stroke_list.at(Fcounter) <<  " ============"<<'\n';     
      
      // Recognize one or two hands and make the prediction using $1 Multistroke recognizer
      if(imageProc.leftHand.size() > 10)        //if the list holding the second hand coordinates is greater than 10, then we are on a two hands situation
       {
            //two hands so classify each of them singularly 
           classification.ClassifySample(imageProc.leftHand);
           classification.Classify(classification.DecisionPoints, "Predict", "Left-hand");
           
           classification.ClassifySample(imageProc.rightHand);
           classification.Classify(classification.DecisionPoints, "Predict", "Right-hand");
       }else
       {
           //one hand 
           classification.ClassifySample(imageProc.oneHand);
           classification.Classify(classification.DecisionPoints, "Predict", "One hand");
       }
 
       //	Free memory 
      cv::destroyWindow(dirAccess.stroke_list.at(Fcounter));
      
      imageProc.oneHand.clear();
      imageProc.leftHand.clear();
      imageProc.rightHand.clear();
      
      dirAccess.DepthPath.clear();
      dirAccess.DepthImgs.clear();
      }

  return 0;
}
