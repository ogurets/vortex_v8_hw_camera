#include <fcntl.h>      // open
#include <unistd.h>     // exit
#include <sys/ioctl.h>  // ioctl
#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <sys/mman.h>   // mmap, munmap

#define UINT32 unsigned int

#include "vc088x_cif_drv.h"
#include "android_pmem.h"

#define DEVICE_FILE_NAME "/dev/cif"
#define PMEM_VDEC_FILE_NAME "/dev/pmem-vdec"

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

    printf("Mapping memory...\n");
    unsigned char *framebuf;
    V8CIFPATHINFO cpath;
    cpath.dst_type = 0;
    cpath.dst_width = 1280;
    cpath.dst_height = 1024;

    framebuf = malloc(1280*1024*2*3);

    // Looks like these values are ignored by driver, setting these to 0 to crash if I'm wrong
    cpath.dst_addr[0] = 0;
    cpath.dst_addr[1] = 0;
    cpath.dst_addr[2] = 0;

    printf("Configuring grab...\n");
    ret_val = ioctl(file_desc, V8CIF_CAPPATH_CFG, &cpath);
    if (ret_val < 0) {
        printf ("ioctl grab failed:%d\n", ret_val);
        exit(-1);
    }

   /*ret_val = ioctl(file_desc, V8CIF_SETPREVIEWSTOP, 0);
   if (ret_val < 0) {
     printf ("ioctl previewstop failed:%d\n", ret_val);
     exit(-1);
   }*/

    printf("Begin cycle...\n");
    int gotdata = 0;
    while (!gotdata) {
        V8CIFFRMINFO frinfo;
        frinfo.dst_addr_y = (UINT32)framebuf;
        frinfo.dst_addr_uv = (UINT32)framebuf + 1280*1024;
        frinfo.cap_mode = CAP_NORMAL;

        printf(".");
        ret_val = ioctl(file_desc, V8CIF_CAPPATH_GETFRM, &frinfo);
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

    printf("Got the frame! Saving...\n");
    FILE *fp = fopen("./capture.raw", "wb");
    fwrite(framebuf, 1, 1280*1024*2*2, fp);
    free(framebuf);
    fclose(fp);
}

int opendev(const char *devname, int mode)
{
    int file_desc;
    file_desc = open(devname, 2, mode);  // [XXX] Manufacturer's code uses flag 2, but why?!

    if (file_desc < 0) {
        printf("Can't open device file: %s\n", devname);
        exit(-1);
    }
    
    return file_desc;
}

// Had to reverse-engineer this one from "libcamera.so", no manufacturer source code available
/* 
    Some hints and theory on using this mechanism:
        http://www.phonesdevelopers.info/1722480/
        https://android.googlesource.com/platform/frameworks/native/+/252778f/libs/binder/MemoryHeapPmem.cpp
*/
int PhyMemAlloc(int pmem_dev)
{
    // TODO
    return 0;
}

void PhyMemFree(int pmem_dev)
{
    // TODO
}

void PhyMemInfo(int pmem_dev)
{
    int ret_val, rsize;
    ret_val = ioctl(pmem_dev, PMEM_GET_TOTAL_SIZE, &rsize);
    if (ret_val < 0) {
        printf ("ioctl PMEM_GET_TOTAL_SIZE failed: %d\n", ret_val);
        exit(-1);
    }
    printf("Total physical memory size: %d, retval: %d\n", rsize, ret_val);
}

void main()
{
    int camera_dev, pmem_dev, ret_val;
    camera_dev = opendev(DEVICE_FILE_NAME, O_RDONLY);
    pmem_dev = opendev(PMEM_VDEC_FILE_NAME, O_RDONLY);

    // Get camera info
    //ioctl_power(camera_dev, 1);
    //ioctl_getinfo(camera_dev);
    
    // Poke physmem mapper
    PhyMemInfo(pmem_dev);
    
    // Grab some
    //ioctl_power(camera_dev, 1);  // Turn on
    //sleep(3);
    //ioctl_grab(camera_dev);
    //ioctl_power(camera_dev, 0);  // Turn off

    close(camera_dev);
    close(pmem_dev);
}
