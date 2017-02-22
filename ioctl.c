#include <fcntl.h>      // open
#include <unistd.h>     // exit, getpagesize
#include <sys/ioctl.h>  // ioctl
#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <sys/mman.h>   // mmap, munmap
#include <memory.h>     // memset

#define UINT32 unsigned int

#include "vc088x_cif_drv.h"
#include "android_pmem.h"

#define DEVICE_FILE_NAME "/dev/cif"
#define PMEM_VDEC_FILE_NAME "/dev/pmem-vdec"
#define FRAME_WIDTH  1280
#define FRAME_HEIGHT 1024

// Manufacturer's way
//#define FRAME_SIZE_BYTES (3 * FRAME_WIDTH * FRAME_HEIGHT >> 1)
#define FRAME_SIZE_BYTES (2 * FRAME_WIDTH * FRAME_HEIGHT)

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

void ioctl_grab(int file_desc, unsigned char *framebuf, UINT32 hwoffset)
{
    int ret_val;

    printf("Setting resolution...\n");
    ret_val = ioctl(file_desc, V8CIF_SNR_CFG, 1);  // Set max resolution
    if (ret_val < 0) {
        printf ("ioctl set resolution failed:%d\n", ret_val);
        exit(-1);
    }

    printf("Configuring grab...\n");
    V8CIFPATHINFO cpath;
    cpath.dst_type = 0;
    cpath.dst_width = FRAME_WIDTH;
    cpath.dst_height = FRAME_HEIGHT;

    // Looks like these values are ignored by driver, setting these to 0 to crash if I'm wrong
    cpath.dst_addr[0] = hwoffset;
    cpath.dst_addr[1] = hwoffset;
    cpath.dst_addr[2] = hwoffset;

    ret_val = ioctl(file_desc, V8CIF_PREPATH_CFG, &cpath);
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
        frinfo.dst_addr_y = (UINT32)hwoffset;
        frinfo.dst_addr_uv = (UINT32)hwoffset + FRAME_WIDTH * FRAME_HEIGHT;
        frinfo.cap_mode = CAP_NORMAL; // Manufacturer uses 0

        printf(".");
        ret_val = ioctl(file_desc, V8CIF_PREPATH_GETFRM, &frinfo);
        printf("[D] ioctl packet V8CIF_PREPATH_GETFRM our: %08X, manuf: %08X\n", V8CIF_PREPATH_GETFRM, 0x800C6308);
        if (ret_val < 0) {
            printf ("ioctl grab failed:%d\n", ret_val);
            exit(-1);
        }

        printf("[D] dst_addr: %08X\n", frinfo.dst_addr_y);
        if (framebuf[0] || framebuf[100] || framebuf[200]) {  // Randomly test some pixels for data
            printf("We have the juice!\n");
            gotdata = 1;
        } else {
            sleep(1);
        }
    }

    printf("Got the frame! Saving...\n");
    FILE *fp = fopen("./capture.raw", "wb");
    fwrite(framebuf, 1, FRAME_SIZE_BYTES, fp);
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
void *PhyMemAlloc(int pmem_dev, struct pmem_region *sub)
{
    const size_t pagesize = getpagesize();
    printf("[D] Page size: %d\n", pagesize);
    
    int size = sub->len;
    size = (size + pagesize-1) & ~(pagesize-1);
    
    void *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, pmem_dev, 0);  // prot == 3, flags == 1
    printf("[D] ioctl command packet (should be 0x40047001): %08X\n", PMEM_MAP);

    // Should be PMEM_MAP, but manufacturer uses "read" version: 0x40047001, and a weird argument
    //int err = ioctl(pmem_dev, PMEM_MAP, sub);
    UINT32 hwptr;
    int err = ioctl(pmem_dev, 0x40047001, &hwptr);
    sub->offset = hwptr;
    if (err < 0) {
        printf("PMEM_MAP failed (%s), mFD=%d, sub.offset=%lu, sub.size=%lu",
                err, pmem_dev, sub->offset, sub->len);
        exit(-1);
    } else {
        printf("PMEM_MAP success, sub.offset=%lu, sub.size=%lu\n",
                sub->offset, sub->len);
    }
    
    printf("Allocated userland ptr: %08X, hw offset: %08X\n", ptr, hwptr);
    if (ptr) {
        printf("Wiping %d bytes of memory...\n", sub->len);
        memset(ptr, 0, size);
    }
    
    return ptr;
}

void PhyMemFree(int pmem_dev, void *ptr, struct pmem_region *sub)
{
    if (munmap(ptr, sub->len) > 0) {
        printf("Cannot munmap ptr: %08X, size: %d\n", ptr, sub->len);
    }
    
    // Manufacturer doesn't use this part
    /*int err = ioctl(pmem_dev, PMEM_UNMAP, &sub);
    if (err < 0) {
        printf("PMEM_UNMAP failed (%d), mFD=%d, sub.offset=%lu, sub.size=%lu\n",
                err, pmem_dev, sub->offset, sub->len);
        exit(-1);
    }*/
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

    // Poke physmem mapper
    printf("Mapping memory...\n");
    struct pmem_region sub = { 0, FRAME_SIZE_BYTES };
    void *ptr = PhyMemAlloc(pmem_dev, &sub);

    PhyMemInfo(pmem_dev);
    
    // Grab some
    ioctl_power(camera_dev, 1);  // Turn on
    ioctl_getinfo(camera_dev);  // Get camera info

    //sleep(3);
    ioctl_grab(camera_dev, ptr, sub.offset);
    ioctl_power(camera_dev, 0);  // Turn off

    PhyMemFree(pmem_dev, ptr, &sub);

    close(camera_dev);
    close(pmem_dev);
}
