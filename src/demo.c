#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "image.h"
#include "demo.h"
#include <sys/time.h>


#define DEMO 1

static char **demo_names;
static image **demo_alphabet;
static int demo_classes;

static network *net;
static float demo_thresh = 0;
static float demo_hier = .5;
static float fps = 0;

double demo_time;

detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num);

void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int delay, char *prefix, int avg_frames, float hier, int w, int h, int frames, int fullscreen, int netsize) {
	

	demo_names = names;
	demo_alphabet = NULL;
	demo_classes = classes;
	demo_thresh = thresh;
	demo_hier = hier;
	net = load_network(cfgfile, weightfile, 0);


	make_window("Demo", 800, 600);

	demo_time = what_time_is_it_now();

	int buttonStatus;

	int x_middle = -1;
	int y_middle = -1;

	int moved = 0;

	while(1){ 

		image ss = get_image_from_ss(fullscreen, netsize);

		buttonStatus = buttonListner();

		if (buttonStatus){

			image sized = letterbox_image(ss, net->w, net->h);
			float *X = sized.data;
			network_predict(net, X);

			int nboxes = 0;
			detection *dets = get_network_boxes(net, ss.w, ss.h, .5, .5, 0, 1, &nboxes);

			draw_detections(ss, dets, nboxes, demo_thresh, demo_names, demo_alphabet, demo_classes, &x_middle, &y_middle);
			free_detections(dets, nboxes);
			free_image(sized);

			if (x_middle != -1 || y_middle != -1){

				movePointer(x_middle, y_middle);

			}


		}

		show_image(ss, "Demo", 1);
		free_image(ss);

		fps = 1./(what_time_is_it_now() - demo_time);
		demo_time = what_time_is_it_now();
		
		//printf("%f\n", fps);
		if (fps < 200){
			printf("FPS: %i x: %i y: %i \n", (int)fps, x_middle, y_middle);

		}

	}

}