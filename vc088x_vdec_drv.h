#ifndef _VC088X_VDEC_H_
#define _VC088X_VDEC_H_

#include <linux/ioctl.h>    															/* needed for the _IOW etc stuff used later */

struct _de_info_
{
	unsigned int width;
	unsigned int height;
	unsigned int type;
	unsigned int y;
	unsigned int uv;
};

#define V8VDEC_IOC_MAGIC  				'k'												/* Use 'k' as magic number */

#define V8VDEC_IOCS_PP_INSTANCE			_IO(V8VDEC_IOC_MAGIC, 1)   						/* the client is pp instance */
#define V8VDEC_IOCS_HW_PERFORMANCE		_IO(V8VDEC_IOC_MAGIC, 2)   						/* decode/pp time for HW performance */
#define V8VDEC_IOCG_HW_OFFSET			_IOR(V8VDEC_IOC_MAGIC, 3, unsigned int *)
#define V8VDEC_IOCG_HW_IOSIZE			_IOR(V8VDEC_IOC_MAGIC, 4, unsigned int *)
#define V8VDEC_IOCS_IRQ_DISABLE			_IO(V8VDEC_IOC_MAGIC, 5)
#define V8VDEC_IOCS_IRQ_ENABLE			_IO(V8VDEC_IOC_MAGIC, 6)
#define V8VDEC_IOCS_DE_ENABLE			_IOR(V8VDEC_IOC_MAGIC, 7, struct _de_info_ )
#define V8VDEC_IOCS_DE_CFG				_IOR(V8VDEC_IOC_MAGIC, 8, struct _de_info_ )

#define V8VDEC_IOCS_POWERON				_IO(V8VDEC_IOC_MAGIC, 9)
#define V8VDEC_IOCS_POWEROFF			_IO(V8VDEC_IOC_MAGIC, 10)

#define V8VDEC_IOCS_DINAMIC_FREQ        _IOR(V8VDEC_IOC_MAGIC, 11, unsigned int)
#define V8VDEC_IOCS_GAMMA               _IOR(V8VDEC_IOC_MAGIC, 12, unsigned char *)



#define V8VDEC_IOC_MAXNR 				12

#endif /* !_VC088X_VDEC_H_ */
