#ifdef OPENCV

#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "image.h"

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "errno.h"
#include "linux/uinput.h"

using namespace cv;

class mouseHandler{
public:

	mouseHandler();
	~mouseHandler();

	void move_mouse (int dx, int dy);
	void click();
	void mouseButton();
	int getButton();

private:

	// Mouse status
	int buttonState;
	Display* d;
	Window w, r;
	int wx, wy, rx, ry;
	unsigned m;

	int fd;
	struct uinput_user_dev uidev;
	struct input_event ev;
	
};

mouseHandler::mouseHandler(){

	// Mouse status
	d = XOpenDisplay(NULL);
	w = r = DefaultRootWindow (d);

	/* Simulate relative move mouse movement and click */
	if ((fd = open ("/dev/uinput", O_WRONLY | O_NONBLOCK)) < 0) perror ("error: open 2");

	if(ioctl(fd, UI_SET_EVBIT,  EV_KEY) < 0)    perror ("error: ioctl");
	if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)  perror ("error: ioctl");

	if(ioctl(fd, UI_SET_EVBIT,  EV_REL) < 0)    perror ("error: ioctl");
	if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)     perror ("error: ioctl");
	if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)     perror ("error: ioctl");

	memset (&uidev, 0, sizeof(uidev));
	snprintf (uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");

	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	if (write (fd, &uidev, sizeof(uidev)) < 0) perror("error: write");
	if (ioctl(fd, UI_DEV_CREATE) < 0) perror("error: ioctl");

	sleep(1);

}

mouseHandler::~mouseHandler(){

	if (ioctl (fd, UI_DEV_DESTROY) < 0) perror ("error: ioctl");
	close(fd);
	XCloseDisplay(d);

}

void mouseHandler::move_mouse(int dx, int dy){

	struct input_event ev;

	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_REL;
	ev.code = REL_X;
	ev.value = dx;
	if (write (fd, &ev, sizeof(struct input_event)) < 0)
		perror("error: write");

	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_REL;
	ev.code = REL_Y;
	ev.value = dy;
	if (write (fd, &ev, sizeof(struct input_event)) < 0)
		perror("error: write");

	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;
	if(write(fd, &ev, sizeof(struct input_event)) < 0)
		perror("error: write");

	//usleep(15000);
	usleep(500);
}

void mouseHandler::click(){
  
	struct input_event     ev;

	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_KEY;
	ev.code = BTN_LEFT;
	ev.value = 1;
	if (write (fd, &ev, sizeof(struct input_event)) < 0)
		perror("error: write");

	usleep (500000);

	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_KEY;
	ev.code = BTN_LEFT;
	ev.value = 0;
		if (write (fd, &ev, sizeof(struct input_event)) < 0)
	perror("error: write");


	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;
	if(write(fd, &ev, sizeof(struct input_event)) < 0)
	perror("error: write");

	usleep(10000);
  
}

void mouseHandler::mouseButton(){

	XQueryPointer (d, r, &r, &w, &rx, &ry, &wx, &wy, &m);

	//int right     = !!(m & Button1Mask);
	int middle  = !!(m & Button2Mask); // Scroll button
	//int left  = !!(m & Button3Mask);

	buttonState = middle;

	//printf("%d\n", middle);

}

int mouseHandler::getButton(){
	this->mouseButton();
	return buttonState;
}


struct ScreenShot {

	ScreenShot(int x, int y, int width, int height):
		x(x),
		y(y),
		width(width),
		height(height)
	{
		display = XOpenDisplay(NULL);
		root = DefaultRootWindow(display);

		init = true;
	}

	void operator() (Mat& cvImg)
	{
		if(init == true)
			init = false;
		else
			XDestroyImage(img);

		img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);

		cvImg = Mat(height, width, CV_8UC4, img->data);
	}

	~ScreenShot()
	{
		if(init == false)
			XDestroyImage(img);

		XCloseDisplay(display);
	}

	Display* display;
	Window root;
	int x,y,width,height;
	XImage* img;

	bool init;
};


mouseHandler m;

extern "C" {

int buttonListner(){
	return m.getButton();
}

void movePointer(int x, int y) {

	// Relative move pointer
	m.move_mouse(x, y);

}

IplImage *image_to_ipl(image im)
{
	int x,y,c;
	IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
	int step = disp->widthStep;
	for(y = 0; y < im.h; ++y){
		for(x = 0; x < im.w; ++x){
			for(c= 0; c < im.c; ++c){
				float val = im.data[c*im.h*im.w + y*im.w + x];
				disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
			}
		}
	}
	return disp;
}

image ipl_to_image(IplImage* src)
{
	int h = src->height;
	int w = src->width;
	int c = src->nChannels;
	image im = make_image(w, h, c);
	unsigned char *data = (unsigned char *)src->imageData;
	int step = src->widthStep;
	int i, j, k;

	for(i = 0; i < h; ++i){
		for(k= 0; k < c; ++k){
			for(j = 0; j < w; ++j){
				im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
			}
		}
	}
	return im;
}

Mat image_to_mat(image im)
{
	image copy = copy_image(im);
	constrain_image(copy);
	if(im.c == 3) rgbgr_image(copy);

	IplImage *ipl = image_to_ipl(copy);
	Mat m = cvarrToMat(ipl, true);
	cvReleaseImage(&ipl);
	free_image(copy);
	return m;
}

image mat_to_image(Mat m)
{
	IplImage ipl = m;
	image im = ipl_to_image(&ipl);
	rgbgr_image(im);
	return im;
}

void *open_video_stream(const char *f, int c, int w, int h, int fps)
{
	VideoCapture *cap;
	if(f) cap = new VideoCapture(f);
	else cap = new VideoCapture(c);
	if(!cap->isOpened()) return 0;
	if(w) cap->set(CV_CAP_PROP_FRAME_WIDTH, w);
	if(h) cap->set(CV_CAP_PROP_FRAME_HEIGHT, w);
	if(fps) cap->set(CV_CAP_PROP_FPS, w);
	return (void *) cap;
}

image get_image_from_stream(void *p)
{
	VideoCapture *cap = (VideoCapture *)p;
	Mat m;
	*cap >> m;

	if(m.empty()) return make_empty_image(0,0,0);
	return mat_to_image(m);
}


image get_image_from_ss(int dual, int netS)
{

	//fetch screen size
	Display* disp = XOpenDisplay(NULL);
	Screen*  scrn = DefaultScreenOfDisplay(disp);
	int height = scrn->height;
	int width  = scrn->width;
	XCloseDisplay(disp);

	//printf("H: %i W: %i\n", height, width);

	int x = 0;
	int y = 0;


	if(dual){
		// use right screen as default
		//x = int(width/2); 
		//width = x;

		// use left screen
		x = 0;
		width = int(width/2); 

	}

	if(netS){

		// use 416 as cut values
		int n = 416;

		int midpoint_x = int(width/2);
		int midpoint_y = int(height/2);

		x = midpoint_x - int(n/2);
		y = midpoint_y - int(n/2);

		width = n;
		height = n;


	}



	ScreenShot screen(x, y, width, height);
	Mat m;
	screen(m);

	if(!netS){
		cv::resize(m, m, cv::Size(), 0.50, 0.50);
	}

	if(m.empty()) return make_empty_image(0,0,0);
	return mat_to_image(m);
}

image load_image_cv(char *filename, int channels)
{
	int flag = -1;
	if (channels == 0) flag = -1;
	else if (channels == 1) flag = 0;
	else if (channels == 3) flag = 1;
	else {
		fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
	}
	Mat m;
	m = imread(filename, flag);
	if(!m.data){
		fprintf(stderr, "Cannot load image \"%s\"\n", filename);
		char buff[256];
		sprintf(buff, "echo %s >> bad.list", filename);
		system(buff);
		return make_image(10,10,3);
		//exit(0);
	}
	image im = mat_to_image(m);
	return im;
}

int show_image_cv(image im, const char* name, int ms)
{
	Mat m = image_to_mat(im);
	imshow(name, m);
	int c = waitKey(ms);
	//if (c != -1) c = c%256;
	return c;
}

void make_window(char *name, int w, int h)
{
	namedWindow(name, WINDOW_NORMAL); 
	resizeWindow(name, w, h);

	/*
	if (fullscreen) {
		setWindowProperty(name, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	} else {
		resizeWindow(name, w, h);
		if(strcmp(name, "Demo") == 0) moveWindow(name, 0, 0);
	}
	*/
	//destroyWindow(name);


}

}

#endif
