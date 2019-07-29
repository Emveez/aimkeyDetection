# Aimkey based on object detection algorithm
This project aims to create an object detection type aimkey on Linux. It's based on [Darknet](https://pjreddie.com/darknet/) framework and does not read and write to any process memory as conventional external aimkeys. The logic behind the aimkey is that we capture a screenshot and pass is to the detection algorithm which returns the coordinates of a person object with the highest probability, then set the aim to this location.

For screenshot and mouse button state the Xlib is used and for relative mouse move the device uinput is used. 

### Dependencies
* CUDA (Tested with Cuda 8.0)
* CUDNN (Tested with CUDNN 7.1.4)
* OpenCV (Tested with OpenCV 3.3.0)

### Installation/Usage
* Install dependencies according to [this](https://pjreddie.com/darknet/install/)
* Get the weights
* Compile with make and run with admin rights
* For dual screen use the flag -dual
* Use mouse button 3 i.e. push mouse scroll to aim 


### Demo
![Alt Text](https://media.giphy.com/media/dAFTGMtnBzQhYlZTrs/giphy.gif)

The demo was performed with a GTX 1080ti gpu and Intel Core i5-6600 cpu @ 3.30GHz. With this hardware detection is done at around 22 fps.
