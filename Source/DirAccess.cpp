/*
------------------------------------------------------------------------------------------------------------------------------------------
Description   : Directory access. Stores the dir and the file names.
------------------------------------------------------------------------------------------------------------------------------------------
*/
/*
----------------------------------------------------------------------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------------------------------------------------------------------
*/
#include "DirAccess.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;

/*
--------------------------------------------------------------------------------------------------
Functions definition
--------------------------------------------------------------------------------------------------
*/
DirAccess::DirAccess()
    : Dirname_("./Data")       //	Constant dir name
{
    // Hand stroke use cases  //

    // Dataset1 ds325	
	
    stroke_list.push_back("fast_circles");  
    stroke_list.push_back("gestures_two_hands");  
    stroke_list.push_back("gestures_two_hands_swap");  
    stroke_list.push_back("sequence_closed_hand");  
    stroke_list.push_back("sequence_open_hand"); 
    stroke_list.push_back("sequence_small_shapes");  
	
    // Dataset1 ds536
    
    stroke_list.push_back("circle_ccw");  
    stroke_list.push_back("circle_ccw_far");   
    stroke_list.push_back("circle_ccw_hand");  
    stroke_list.push_back("circle_sequence");  
    stroke_list.push_back("multiple_shapes_1"); 
	stroke_list.push_back("rectangle_ccw");  
    stroke_list.push_back("rectangle_cw"); 
	stroke_list.push_back("star"); 
    stroke_list.push_back("zigzag"); 
	

    //	Find the total number of folders (for iterations)
    Num_Of_Folders = stroke_list.size();
};

// Stores Depth images to a vector
void DirAccess::Getimages(void)
{
    for (int i = 0; i < DepthPath.size(); i++) {
        DepthImgs.push_back(cv::imread(DepthPath.at(i), CV_LOAD_IMAGE_ANYDEPTH));
    }
}

//	Stores the directory and the file names in vectors
void DirAccess::GetPaths(int Fcounter)
{
    //	Initialize buffer and append the buffer to complete the path
    StrPth = Dirname_;
    StrPth.append("/");
    StrPth.append(stroke_list.at(Fcounter));  //Append foldername
   
    dir = opendir(StrPth.c_str());      //Open directory stream - Treat StrPth buffer as const char* argument

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {

            //	Initialize the buffer and append the buffer to complete the path
            StrPth = Dirname_;
            StrPth.append("/");
            StrPth.append(stroke_list.at(Fcounter)); //	Append foldername

            //	Check for navigation symbols '.' && '..' and avoid appending
            if (strcmp(ent->d_name, ".") != false && strcmp(ent->d_name, "..") != false) {
                last = StrPth.substr(StrPth.length() - 1); //	Get the last character

                //	Append directory separator if not already there, check for the last character
                if (last.compare(":") != false || last.compare("/") != false || last.compare("/") != false) {
                    StrPth.append("/");
                    StrPth.append(ent->d_name);
                }
                
                DepthPath.push_back(StrPth);
                StrPth.clear(); // Clear the buffer from the annexations to reuse it
            }
        }
    }
}
