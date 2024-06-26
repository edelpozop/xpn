#ifndef _NFI_LOCAL_ERR_H_
#define _NFI_LOCAL_ERR_H_


 #ifdef  __cplusplus
    extern "C" {
 #endif


enum nfi_local_err{
	LOCALERR_PARAM = 0,
	LOCALERR_MEMORY = 1,
	LOCALERR_URL = 2,
	LOCALERR_MNTCONNECTION = 3,
	LOCALERR_MOUNT = 4,
	LOCALERR_NFSCONNECTION = 5,		
	LOCALERR_GETATTR = 6,
	LOCALERR_LOOKUP = 7,
	LOCALERR_READ = 8,
	LOCALERR_WRITE = 9,
	LOCALERR_CREATE = 10,
	LOCALERR_REMOVE = 11,
	LOCALERR_MKDIR = 12,
	LOCALERR_READDIR = 13,
	LOCALERR_STATFS = 14,
	LOCALERR_NOTDIR = 15,
};


void local_err(int err);


 #ifdef  __cplusplus
     }
 #endif


#endif
