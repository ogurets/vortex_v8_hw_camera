#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>     /* exit */

#include "vc088x_cif_drv.h"

#define DEVICE_FILE_NAME "/dev/cif"
#define UINT32 unsigned int

void ioctl_getinfo(int file_desc)
{
  int ret_val;
  V8CIFSNRINFO sinfo;
  sinfo.type = 1;

  ret_val = ioctl(file_desc, V8CIF_SNR_GETINFO, &sinfo);
  if (ret_val < 0) {
    printf ("ioctl req getinfo failed:%d\n", ret_val);
    exit(-1);
  }

  printf("Command succeeded, info:\n");

  printf(" totalItems: %d\n", sinfo.totalItems);
  printf(" type: %d", sinfo.type);
  printf(" jpegSupported: %d\n", sinfo.jpegSupported);
  printf("\n Modes:\n");

  int i;
  if (sinfo.totalItems > SNR_ITEM_NUM) {
    sinfo.totalItems = SNR_ITEM_NUM;
  }
  for (i = 0; i < sinfo.totalItems; ++i) {
     printf("  %d x %d\n", sinfo.width[i], sinfo.height[i]);
  }
}

void ioctl_power(int file_desc, int on)
{
  int ret_val;

  if (on) {
    ret_val = ioctl(file_desc, V8CIF_SNR_PWRON, 1);  // 1 means "front sensor", doesn't really used by Viewpad 10e driver
  } else {
    ret_val = ioctl(file_desc, V8CIF_SNR_PWRDOWN, 1);
  }

  if (ret_val < 0) {
    printf ("ioctl req power failed:%d\n", ret_val);
    exit(-1);
  }
}

void ioctl_grab(int file_desc)
{
  int ret_val;

  printf("Setting resolution...\n");
  ret_val = ioctl(file_desc, V8CIF_SNR_CFG, 1);  // Set max resolution
  if (ret_val < 0) {
    printf ("ioctl set resolution failed:%d\n", ret_val);
    exit(-1);
  }

  printf("Grabbing...\n");
  unsigned char *framebuf;
  if (0) {
   V8CIFFRMINFO frinfo;
   framebuf = malloc(1280*1024*2);
   printf("[D] malloc ret: %08X\n", framebuf);

   frinfo.dst_addr_y = framebuf;
   frinfo.dst_addr_uv = framebuf + 1280*1024;
   frinfo.cap_mode = CAP_TRIGER;

   ret_val = ioctl(file_desc, V8CIF_CAPPATH_GETFRM, &frinfo);
   if (ret_val < 0) {
     printf ("ioctl grab failed:%d\n", ret_val);
     exit(-1);
   }

   printf("[D] dst_addr: %08X\n", frinfo.dst_addr_y);
   //sleep(3);
  } else {
   V8CIFPATHINFO cpath;
   cpath.dst_type = 0;
   cpath.dst_width = 1280;
   cpath.dst_height = 1024;

   framebuf = malloc(1280*1024*2*3);

   cpath.dst_addr[0] = framebuf;
   cpath.dst_addr[1] = framebuf + 1280*1024*2;
   cpath.dst_addr[2] = framebuf + 1280*1024*2*2;

   /*printf("Configuring grab...\n");
   ret_val = ioctl(file_desc, V8CIF_PREPATH_CFG, &cpath);
   if (ret_val < 0) {
     printf ("ioctl grab failed:%d\n", ret_val);
     exit(-1);
   }

   ret_val = ioctl(file_desc, V8CIF_SETPREVIEWSTOP, 0);
   if (ret_val < 0) {
     printf ("ioctl grab failed:%d\n", ret_val);
     exit(-1);
   }/**/

   //sleep(1);
   printf("Begin cycle...\n");
   int gotdata = 0;
   while (!gotdata) {
    V8CIFFRMINFO frinfo;
    frinfo.dst_addr_y = (UINT32)framebuf;
    frinfo.dst_addr_uv = (UINT32)framebuf + 1280*1024;
    frinfo.cap_mode = CAP_NORMAL;

    printf(".");
    ret_val = ioctl(file_desc, V8CIF_PREPATH_GETFRM, &frinfo);
    if (ret_val < 0) {
      printf ("ioctl grab failed:%d\n", ret_val);
      exit(-1);
    }

    printf("[D] dst_addr: %08X\n", frinfo.dst_addr_y);
    if (framebuf[0] || framebuf[100] || framebuf[200]) {
      printf("We have the juice!\n");
      gotdata = 1;
    } else {
      sleep(1);
    }
   }


   /*frinfo.dst_addr_y = framebuf + 1280*1024*2;
   frinfo.dst_addr_uv = framebuf + 1280*1024*3;
   frinfo.cap_mode = CAP_NORMAL;

   ret_val = ioctl(file_desc, V8CIF_CAPPATH_GETFRM, &frinfo);
   if (ret_val < 0) {
     printf ("ioctl grab failed:%d\n", ret_val);
     exit(-1);
   }

   printf("[D] dst_addr: %08X\n", frinfo.dst_addr_y);*/
  }

  printf("Got the frame! Saving...\n");
  FILE *fp = fopen("./capture.raw", "wb");
  fwrite(framebuf, 1, 1280*1024*2*2, fp);
  free(framebuf);
  fclose(fp);
}


void main()
{
  int file_desc, ret_val;
  file_desc = open(DEVICE_FILE_NAME, 0);

  if (file_desc < 0) {
   printf ("Can't open device file: %s\n", DEVICE_FILE_NAME);
   exit(-1);
  }

  //ioctl_power(file_desc, 1);
  //ioctl_getinfo(file_desc);
  ioctl_power(file_desc, 1);
  //sleep(3);
  ioctl_grab(file_desc);
  ioctl_power(file_desc, 0);  // Turn off

  close(file_desc);
}
