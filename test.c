#include <asm/types.h>         
#include <fcntl.h>             
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <linux/fb.h>
#include <sys/time.h>
typedef struct{
    int fd;
    void *buf;
    int bpp;
    int width;
    int height;
    int virtual_x;
    int virtual_y;
    int virtual_width;
    int virtual_height;
    int stride;
    char color_key[0];
}fb_info_t;
static void *fbbuf;
static fb_info_t *fb0;
unsigned int screen_size;
struct fb_var_screeninfo fbvar;
struct fb_fix_screeninfo finfo;
static inline unsigned int rgba565(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
static inline unsigned int rgba888(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
int openfb(char *devname)
{
    int fd;
    unsigned int color;
    fd = open(devname, O_RDWR);
    if (ioctl(fd, FBIOGET_VSCREENINFO, &fbvar) < 0)
        return -1;
    if(!fb0)
    {
        fb0 = (fb_info_t *)malloc(sizeof(fb_info_t) + fbvar.bits_per_pixel);
    }
    color = rgba888(0,0xff,0,0);
    fb0->fd = fd;
    fb0->width = fbvar.xres;
    fb0->height = fbvar.yres;
    fb0->virtual_width = fbvar.xres_virtual;
    fb0->virtual_height = fbvar.yres_virtual;
    fb0->virtual_x = fbvar.xoffset;
    fb0->virtual_y = fbvar.yoffset;
    fb0->bpp = fbvar.bits_per_pixel;
    fb0->stride = fb0->bpp * fb0->virtual_width >> 3;
    memcpy(fb0->color_key,&color,fb0->bpp >> 3);
    screen_size = fb0->height * fb0->stride;
    fb0->buf = mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return fd;
}
static inline unsigned int  rgba565(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
    return ((r << 11) | (g <<5) | b);
}
static inline unsigned int rgba888(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
    return ((r << 16) | (g << 8) | b);
}
static void fill_pixel(unsigned int pixel, int x0, int y0, int w, int h)
{
    int i, j;
    unsigned int *pbuf = (unsigned int *)fb0->buf;
    for (i = y0; i < h; i ++) {
        for (j = x0; j < w; j ++) {
            pbuf[i * 240 + j] = pixel;
        }
    }
}

static void clean_framebuffer(const fb_info_t *fb)
{
    int x,y;
    for(y = fb->virtual_y;y < fb->virtual_height;y ++)
    {
        for(x = fb->virtual_x;x < fb->stride;x +=(fb->bpp >> 3))
        {
            memcpy(&fb->buf[y * fb->stride + x],fb->color_key,fb->bpp >> 3);
        }
    }
    return ;
}
int main () {
    unsigned int r, g, b;
    int fp=0;
    unsigned int count;

    r = 0;
    g = 0;
    b = 0;
    fp = openfb ("/dev/fb0");

    if (fp < 0){
        printf("Error : Can not open framebuffer device\n");
        exit(1);
    }

    while(1) {
        for (count = 0; count < 100; count++) {
            r = 0xff;
            fill_pixel(rgba888(0, r, g, b), 0, 0, 100, 100);
        }
        break;
    }
    getchar();
    clean_framebuffer(fb0);
    return 0;
} 
