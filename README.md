# 3D-Pointer-and-Simple-Stroke-Recognition: July 2019

Given a sequence of depth and confidence image pairs recorded from two models of DepthSense
cameras the goal is to create an application that isolates the hand in the scene, computes a pointer located in that
hand and uses the sequence of positions to classify the stroke using $1 Unistroke Recognizer.
An impotant aspect was to pay attention to the different scenarios in the given image sequences: some gestures are performed with two hands.

The report <b>ValdoJoaoReport.pdf</b> contains a description of the technical challenges encountered,
the various paths explored, including those that were discarded, and a quick review of the code structure and how the different
pieces interact together.


## Implementation Guide
<b>Dependencies</b> <br>
OpenCV https://docs.opencv.org/3.3.1/d7/d9f/tutorial_linux_install.html
<br><br>
Go to the project dir<br>
open CMakeLists.txt and adjust the path to your OpenCV dir<br>
in command line cd <root project dir>
  
<b>Build</b> <br>
cmake CMakeLists.txt <br>
make<br>
Run<br>
./bin/MultistrokeRec
