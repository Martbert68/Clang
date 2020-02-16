#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
#define X_SIZE 1824
#define Y_SIZE 900 

/* here are our X variables */
Display *display;
int screen;
Window win;
GC gc;
XImage *x_image;
unsigned char *x_buffer;

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();

/* Jpegs */
int read_JPEG_file (char *, unsigned char *, int *);
int jayit(unsigned char *,int, int, char *);

void usage ()
{
	printf("usage: git\n");
	exit (1);
}


int main(int argc,char *argv[])
{
        FILE *outfile;          /* target file */
	short *wave;
        int chan,len,b,lx,ly;
        int size,*fhead,sample_rate,byte_rate,ba,bits_pers,f,vol,vol2;
	int xnodes,ynodes,x,y,turn,walong,p;
        double freq;
        char *string,*fname;
	xnodes=450;
	ynodes=450;
	double dis[390000];
	double dist[390000];
	double vel[390000];

        len=120;

        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;

        size=chan*len*sample_rate;

        wave=(short *)malloc(sizeof(short)*size);
        fhead=(int *)malloc(sizeof(int)*11);


	init_x();
	double ym,inc,tp,td;
	char dum;


	tp=340;
	td=90;
	ym=0.04;
	walong=0;
	lx=tp+10;
	ly=tp+10;

	for (x=0;x<xnodes;x++)
	{
		for (y=0;y<ynodes;y++)
		{
			p=(x*ynodes)+y;
			dis[p]=0;
			vel[p]=0;
		}
	}

	inc=td/(tp-1);
	for (y=1;y<tp;y++)
        {
                p=(tp*ynodes)+y;
                dis[p]=dis[p-1]+inc;
                vel[p]=0;
        }

        inc=td/(ynodes-tp-1);
        for (y=tp;y<ynodes;y++)
        {
                p=(tp*ynodes)+y;
                dis[p]=dis[p-1]+inc;
                vel[p]=0;
        }

        inc=td/(tp-1);
        for (x=1;x<tp;x++)
        {
                p=(x*ynodes)+tp;
                dis[p]=inc*x;
        }

        inc=-td/(xnodes-tp-1);
        for (x=tp;x<xnodes;x++)
        {
                p=(x*ynodes)+tp;
                dis[p]=td-(inc*x);
        }


	int xp,damp;
	int pp;
	int xs=X_SIZE*4;
	int point;
	damp=98000;
	for (turn=0;turn<1600000;turn++)
	{
		if ( turn %10==0){printf ("turn 1 %d \n",turn);}
		if ( turn %20000==0)
		{
			printf ("ping %d \n",turn);
        		inc=td/(tp-1);
			dis[0]=0;
        		for (y=1;y<tp;y++)
        		{
                		p=(150*ynodes)+y;
                		dis[p]=dis[p-1]+inc;
                		vel[p]=0;
			}
		}
	        for (x=1;x<xnodes-1;x++)
	        {
                	p=(x*ynodes);
                	for (y=1;y<ynodes-1;y++)
                	{
				pp=p+y;
				double force;
				force=(8*dis[pp])-(dis[pp-1]+dis[pp+1]+dis[pp+ynodes]+dis[pp-ynodes]
				+dis[pp+ynodes-1]+dis[pp+ynodes+1]+dis[pp-ynodes-1]+dis[pp-ynodes+1]);
				if (vel[pp]>0) 
				{
					force=force-(vel[pp]*vel[pp]/damp);
				} else {
					force=force+(vel[pp]*vel[pp]/damp);
				}
				vel[pp]=vel[pp]-(force*ym);
				dist[pp]=dis[pp]+(vel[pp]/20);
			}
		}
		pp=(ly*ynodes)+lx;
		wave[walong]=327*dist[pp];
		pp=((ly+10)*ynodes)+(lx+10);
		wave[walong+1]=327*dist[pp];
		walong+=2;

                for (x=1;x<xnodes-1;x++)
                {
                        p=(x*ynodes);
                        for (y=1;y<ynodes-1;y++)
                        {
                                pp=p+y;
                                double force;
                                force=(8*dist[pp])-(dist[pp-1]+dist[pp+1]+dist[pp+ynodes]+dist[pp-ynodes]
                                +dist[pp+ynodes-1]+dist[pp+ynodes+1]+dist[pp-ynodes-1]+dist[pp-ynodes+1]);
                                if (vel[pp]>0)
                                {
                                        force=force-(vel[pp]*vel[pp]/damp);
                                } else {
                                        force=force+(vel[pp]*vel[pp]/damp);
                                }
                                vel[pp]=vel[pp]-(force*ym);
                                dis[pp]=dist[pp]+(vel[pp]/20);
                        }
                }
                pp=(ly*ynodes)+lx;
                wave[walong]=327*dist[pp];
                pp=((ly+10)*ynodes)+(lx+10);
                wave[walong+1]=327*dist[pp];
                walong+=2;



	        /*for (x=1;x<xnodes-1;x++)
		{
                	p=(x*ynodes);
                	for (y=1;y<ynodes-1;y++)
			{
				int z;
				z=p+y;
				dis[z]=dist[z];
			}
		} */
		//XPutImage(display, win, gc, x_image, 0, 0, 0, 0, X_SIZE, Y_SIZE);
	}

	if ((outfile = fopen("mus.wav", "wb")) == NULL) { fprintf(stderr, "can't open file\n"); exit(1);}
	fhead[0]=0x46464952;
	fhead[1]=36;
	fhead[2]=0x45564157;
	fhead[3]=0x20746d66;
	fhead[4]=16;
	fhead[5]=65536*chan+1;
	fhead[6]=sample_rate;
	fhead[7]=byte_rate;
	fhead[8]=ba;
	fhead[9]=0x61746164;
	fhead[10]=(size*chan*bits_pers)/8;

	fwrite(fhead,11,sizeof(int),outfile);
	fwrite(wave,size,sizeof(short),outfile);
	scanf("%c",&dum);
	free (fhead);
	free (wave);
	fclose (outfile);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

GLOBAL(int)
read_JPEG_file (char * filename, unsigned char * dots, int * params)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */

  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy (dots+(row_stride*cinfo.output_scanline),buffer[0],row_stride);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    /* put_scanline_someplace(buffer[0], row_stride); */

  }
  /* Step 7: Finish decompression */
  params[0]=cinfo.output_width;
  params[1]=cinfo.output_height;
  params[2]=cinfo.output_components;

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  /* And we're done! */
  return 1;
}

int jayit(unsigned char *screen,int image_width, int image_height, char *name)
{

int row_stride,ex,why,cmp,div,set;
unsigned char *image,**row_pointer,*cr,*cg,*cb;
row_pointer=(unsigned char **)malloc(1);

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;
FILE * outfile;		/* target file */
cinfo.err = jpeg_std_error(&jerr);
jpeg_create_compress(&cinfo);
if ((outfile = fopen(name, "wb")) == NULL) { 
	fprintf(stderr, "can't open file\n");
	exit(1);
}
jpeg_stdio_dest(&cinfo, outfile);
cinfo.image_width = image_width; 	/* image width and height, in pixels */
cinfo.image_height = image_height;
cinfo.input_components = 3;		/* # of color components per pixel */
cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
jpeg_set_defaults(&cinfo);
jpeg_set_quality(&cinfo,100,TRUE); /* limit to baseline-JPEG values */
jpeg_start_compress(&cinfo, TRUE);

  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & screen[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
jpeg_finish_compress(&cinfo);
fclose(outfile);
jpeg_destroy_compress(&cinfo);
}

void init_x()
{
/* get the colors black and white (see section for details) */
        unsigned long black,white;

        x_buffer=(unsigned char *)malloc(sizeof(unsigned char)*4*X_SIZE*Y_SIZE);
        display=XOpenDisplay((char *)0);
        screen=DefaultScreen(display);
        black=BlackPixel(display,screen),
        white=WhitePixel(display,screen);
        win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,
                X_SIZE, Y_SIZE, 5, white,black);
        XSetStandardProperties(display,win,"image","images",None,NULL,0,NULL);
        gc=XCreateGC(display, win, 0,0);
        XSetBackground(display,gc,black); XSetForeground(display,gc,white);
        XClearWindow(display, win);
        XMapRaised(display, win);
        //XMoveWindow(dis, win,window_x,100);
        Visual *visual=DefaultVisual(display, 0);
        x_image=XCreateImage(display, visual, DefaultDepth(display,DefaultScreen(display)), ZPixmap, 0, x_buffer, X_SIZE, Y_SIZE, 32, 0);
};

void close_x() {
        XFreeGC(display, gc);
        XDestroyWindow(display,win);
        XCloseDisplay(display);
        exit(1);
};

void redraw() {
        XClearWindow(display, win);
};


