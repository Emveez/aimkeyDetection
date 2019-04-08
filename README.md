# Aimkey based on object detection algorithm
This project aims to create an objecte detection type aimkey on Linux. It's based on [Darknet](https://pjreddie.com/darknet/) framework and does not read and write to any process memory as conventional aimkeys. The logic behind the aimkey is that we capture a screenshot and pass is to the detection algorithm which returns the coordinates of a person object with the highest probability, then set the aim to this location.

For screenshot and mousebutton state the Xlib are used and for relative mouse move the device uinput is used. 

### Dependencies
* CUDA (Tested with Cuda 8.0)
* OpenCV (Tested with OpenCV 3.3.0)

### Installation/Usage
* Install dependencies according to [this](https://pjreddie.com/darknet/install/)
* Get the weights
* Compile with make and run with admin rights
* Use mousebutton 3 i.e. push mouse scroll to aim 


### Demo
![Alt Text](https://www.dropbox.com/s/2mngkaec8dm0c1p/giphy.gif?dl=0)
