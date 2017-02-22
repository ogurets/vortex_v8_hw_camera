#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>

#define DEVICE_FILE_NAME "/dev/cif"
#define UINT32 unsigned int

/****************************************************************************************************/
#define	SNR_ITEM_NUM	5

/****************************************************************************************************/
typedef enum _CAM_CAP_MODE
{
	CAP_NORMAL  =   0,
	CAP_TRIGER,
	CAP_WAITDONE
}CAM_CAP_MODE;

typedef struct _v8cif_snr_info_
{
	UINT32 totalItems;
	UINT32 type;
	UINT32 jpegSupported;
	UINT32 width[SNR_ITEM_NUM];
	UINT32 height[SNR_ITEM_NUM];

} V8CIFSNRINFO, *PV8CIFSNRINFO;

typedef struct _v8cif_path_info_
{
	UINT32 dst_type;
	UINT32 dst_width;
	UINT32 dst_height;
	UINT32 dst_addr[3];

} V8CIFPATHINFO, *PV8CIFPATHINFO;

typedef struct _v8cif_frame_info_
{
	UINT32 dst_addr_y;
	UINT32 dst_addr_uv;
	CAM_CAP_MODE cap_mode;
} V8CIFFRMINFO, *PV8CIFFRMINFO;

struct vtools_reg {
	unsigned int addr;
	unsigned int value;
};

/****************************************************************************************************/
#define V8CIF_IOC_MAGIC				'c'


/*--------------------------------------------------------------------------
	power on the front or back sensor according to the parameter
	0: Main(Back) sensor
	1: Front sensor
--------------------------------------------------------------------------*/
#define V8CIF_SNR_PWRON				_IOR( V8CIF_IOC_MAGIC, 1, UINT32 )

/*--------------------------------------------------------------------------
	Get the current sensor's info, such as yuv type, jpeg supported,
	item num and size from kernel to hal of android. So the android
	could select and set the right item of sensor according to the
	application.
--------------------------------------------------------------------------*/
#define V8CIF_SNR_GETINFO			_IOR( V8CIF_IOC_MAGIC, 2, V8CIFSNRINFO )

/*--------------------------------------------------------------------------
	Config the current sensor's right item according to the parameter.
--------------------------------------------------------------------------*/
#define V8CIF_SNR_CFG				_IOR( V8CIF_IOC_MAGIC, 3, UINT32 )

/*--------------------------------------------------------------------------
	power down the front or back sensor according to the parameter
	0: Main(Back) sensor
	1: Front sensor
--------------------------------------------------------------------------*/
#define V8CIF_SNR_PWRDOWN			_IOR( V8CIF_IOC_MAGIC, 4, UINT32 )

/*--------------------------------------------------------------------------
	Config the cif's preview path according to the parameter.
--------------------------------------------------------------------------*/
#define V8CIF_PREPATH_CFG			_IOR( V8CIF_IOC_MAGIC, 5, V8CIFPATHINFO )

/*--------------------------------------------------------------------------
	Config the cif's capture path according to the parameter.
--------------------------------------------------------------------------*/
#define V8CIF_CAPPATH_CFG			_IOR( V8CIF_IOC_MAGIC, 6, V8CIFPATHINFO )

/*--------------------------------------------------------------------------
	Get one frame on the preview path.
--------------------------------------------------------------------------*/
#define V8CIF_PREPATH_GETFRM		_IOR( V8CIF_IOC_MAGIC, 7, V8CIFFRMINFO )

/*--------------------------------------------------------------------------
	Get one frame on the capture path.
--------------------------------------------------------------------------*/
#define V8CIF_CAPPATH_GETFRM		_IOR( V8CIF_IOC_MAGIC, 8, V8CIFFRMINFO )

/*--------------------------------------------------------------------------
	Set the preview path special effect.

	input:
	mode: 	0 -- bypass
			1 -- specia effect
			2 -- special color
			3 -- negative
			4 -- mono color
			5 -- four block
			6 -- grid color
			7 -- embossing
			8 -- selhouette
			9 -- pencil draw
			10 -- binary effect
--------------------------------------------------------------------------*/
#define V8CIF_PREPATH_SETSE			_IOR( V8CIF_IOC_MAGIC, 9, UINT32 )

/*--------------------------------------------------------------------------
	Set the capture path special effect.

	input:
	mode: 	0 -- bypass
			1 -- specia effect
			2 -- special color
			3 -- negative
			4 -- mono color
			5 -- four block
			6 -- grid color
			7 -- embossing
			8 -- selhouette
			9 -- pencil draw
			10 -- binary effect
--------------------------------------------------------------------------*/
#define V8CIF_CAPPATH_SETSE			_IOR( V8CIF_IOC_MAGIC, 10, UINT32 )

/*--------------------------------------------------------------------------
	Set the both cap and pre path special effect.

	input:
	mode: 	0 -- bypass
			1 -- specia effect
			2 -- special color
			3 -- negative
			4 -- mono color
			5 -- four block
			6 -- grid color
			7 -- embossing
			8 -- selhouette
			9 -- pencil draw
			10 -- binary effect
--------------------------------------------------------------------------*/
#define V8CIF_BOTHPATH_SETSE		_IOR( V8CIF_IOC_MAGIC, 11, UINT32 )

/*--------------------------------------------------------------------------
	Set both preview and capture path zoom step.
--------------------------------------------------------------------------*/
#define V8CIF_SETZOOM				_IOR( V8CIF_IOC_MAGIC, 12, UINT32 )

/*--------------------------------------------------------------------------
	Set preview stop.
--------------------------------------------------------------------------*/
#define V8CIF_SETPREVIEWSTOP				_IOR( V8CIF_IOC_MAGIC, 13, UINT32 )
#define V8CIF_RECORD_GETFRM		_IOR( V8CIF_IOC_MAGIC, 14, V8CIFFRMINFO )

#define V8CIF_REG_RD				_IOR( V8CIF_IOC_MAGIC, 55, struct vtools_reg )
#define V8CIF_REG_WR				_IOW( V8CIF_IOC_MAGIC, 56, struct vtools_reg )
#define V8CIF_SNR_REG_RD			_IOR( V8CIF_IOC_MAGIC, 57, struct vtools_reg )
#define V8CIF_SNR_REG_WR			_IOW( V8CIF_IOC_MAGIC, 58, struct vtools_reg )


#define V8CIF_IOC_MAXNR				60




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
    ret_val = ioctl(file_desc, V8CIF_SNR_PWRON, 0);  // 1 means "front sensor"
  } else {
    ret_val = ioctl(file_desc, V8CIF_SNR_PWRDOWN, 0);
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
  } else {
   V8CIFPATHINFO cpath;
   cpath.dst_type = 0;
   cpath.dst_width = 1280;
   cpath.dst_height = 1024;
   cpath.dst_addr[0] = framebuf;
   cpath.dst_addr[1] = framebuf + 1280*1024*2;
   cpath.dst_addr[2] = framebuf + 1280*1024*2*2;

   framebuf = malloc(1280*1024*2*3);

   ret_val = ioctl(file_desc, V8CIF_CAPPATH_CFG, &cpath);
   if (ret_val < 0) {
     printf ("ioctl grab failed:%d\n", ret_val);
     exit(-1);
   }
  }

  printf("Got the frame! Saving...\n");
  FILE *fp = fopen("./capture.raw", "wb");
  fwrite(framebuf, 1, 1280*1024*2, fp);
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

  ioctl_power(file_desc, 1);
  ioctl_getinfo(file_desc);
  ioctl_grab(file_desc);
  ioctl_power(file_desc, 0);  // Turn off

  close(file_desc);
}
