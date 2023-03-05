 #include <stdlib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <linux/fb.h>
 #include <sys/mman.h>
 #include <sys/ioctl.h>

int main()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = (vinfo.xres * vinfo.yres * vinfo.bits_per_pixel) / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbfd, 0);

    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
     exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    // make the background clean
    for(y=0;y<vinfo.yres;y++)
    {
        for(x=0;x<vinfo.xres;x++)
        {
            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;

                *(fbp + location) = 0xFF;
                *(fbp + location + 1) = 0xFF;
                *(fbp + location + 2) = 0xFF;
                *(fbp + location + 3) = 0; // no transparency
        }
    }

    int y_mid=vinfo.yres/2;
    int x_mid=vinfo.xres/2;

    int radius;
    if(y_mid<x_mid)
        radius=(y_mid*2)/3;
    else
        radius=(x_mid*2)/3;
    printf("radius is %d\n",radius);

    int top_left_y=y_mid-radius;
    int top_left_x=x_mid-radius;

    for(y=top_left_y;y<top_left_y+radius*2;y++)
    {
        for(x=top_left_x;x<top_left_x+2*radius;x++)
        {
            if((x-x_mid)*(x-x_mid)+(y-y_mid)*(y-y_mid)<radius*radius) // then draw
            {
                location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;

                *(fbp + location) = 0;
                *(fbp + location + 1) = 0;
                *(fbp + location + 2) = 0;
                *(fbp + location + 3) = 0; // no transparency
            }
        }
    }


    /* location = (x_mid+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y_mid+vinfo.yoffset) * finfo.line_length; */
    /*             *(fbp + location) = 0; */
    /*             *(fbp + location + 1) = 0; */
    /*             *(fbp + location + 2) = 0; */
    /*             *(fbp + location + 3) = 0; // no transparency */

/*     // Figure out where in memory to put the pixel */
/* /\*     for (y = 100; y < 300; y++) *\/ */
/* /\*         for (x = 100; x < 300; x++) { *\/ */

/* /\*             location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + *\/ */
/* /\*                 (y+vinfo.yoffset) * finfo.line_length; *\/ */

/* /\*             if(location+3>=screensize) *\/ */
/* /\*                 goto end; *\/ */

/* /\*             if (vinfo.bits_per_pixel == 32) { // 32 bits per pixel, black *\/ */
/* /\*                 *(fbp + location) = 0; *\/ */
/* /\*                 *(fbp + location + 1) = 0; *\/ */
/*                 *(fbp + location + 2) = 0; */
/*                 *(fbp + location + 3) = 0;      // No transparency */

/*             } else  { //assume 16bpp */
/*                 int b = 10; */
/*                 int g = (x-100)/6;     // A little green */
/*                 int r = 31-(y-100)/16;    // A lot of red */
/*                 unsigned short int t = r<<11 | g << 5 | b; */
/*                 *((unsigned short int*)(fbp + location)) = t; */
/*             } */

/*         } */
/* end: */
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
