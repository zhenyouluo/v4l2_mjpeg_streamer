#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "global.h"

#include <unistd.h>

unsigned short int debug_level;

volatile int finish = 0;

void sigint_handler(int signo)
{
	finish = 1;
}

#define WIDTH   1920
#define HEIGHT  1080

int main(int argc, char* argv[])
{
	static struct context ctxt = {0};
	static char in_buf[WIDTH*HEIGHT*2];
	static char out_buf[WIDTH*HEIGHT*2];
	int seq, ret;
	time_t t;
	long last_sec;

	/* 
	 * setup signal
	 */
	if (signal(SIGINT,  sigint_handler) == SIG_ERR) {
        fprintf(stderr, "unable to register signal handler\n");
        exit(0);
    }

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "unable to disable pipe error handler\n");
        exit(0);
    }

	ctxt.conf.setup_mode = 0;
	ctxt.conf.webcam_limit = 0;
	ctxt.conf.webcam_port = 8081;
	ctxt.conf.webcam_localhost = 0;
	ctxt.conf.webcam_quality = 50;
	ctxt.conf.webcam_maxrate = 30;	
	ctxt.conf.v4l2_palette = 8;
	ctxt.conf.brightness = 128;
	ctxt.conf.frame_limit = 30;
	ctxt.conf.input = 8;
	ctxt.conf.roundrobin_frames = 1;
	ctxt.conf.roundrobin_skip = 1;
	ctxt.conf.width = WIDTH;
	ctxt.conf.height = HEIGHT;
	ctxt.conf.video_device = "/dev/video0";

	//ctxt.imgs.type assigned in vid_v4l2_start()
	//also type is set statically to VIDEO_PALETTE_YUV420P in v4l2_start()

	/*
	 * mjpeg streaming init
	 */
	ret = webcam_init(&ctxt);
	if (ret == -1) {
		fprintf(stderr, "unable to open webcam\n");
		exit(0);
	}
	printf("websocket fd = %d\n", ret);

	/*
	 * webcam init
	 */
	vid_init();
	ret = vid_v4l2_start(&ctxt);
	if (ret == -1) {
		fprintf(stderr, "unable to open start v4l2\n");
		exit(0);
	}

	last_sec = time(NULL);
	for (seq = 0; !finish; seq++) {
		long sec;

		vid_next(&ctxt, in_buf);

		webcam_put(&ctxt, in_buf);

		if (last_sec != (sec = time(NULL))) {
			printf("%d fps\n", seq);
			seq = 0;
			last_sec = sec;
		}

		usleep(1000);

		//printf("webcam_put\n");
		//fflush(stdout);
		//
	}

	vid_close(&ctxt);

    return 0;
}
