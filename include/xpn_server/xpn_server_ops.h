
/*
 *  Copyright 2020-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
 *
 *  This file is part of Expand.
 *
 *  Expand is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Expand is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Expand.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef _XPN_SERVER_OPS_H_
#define _XPN_SERVER_OPS_H_

  #ifdef __cplusplus
    extern "C" {
  #endif

    /* ... Include / Inclusion ........................................... */

       #include "all_system.h"
       #include "base/filesystem.h"
       #include "base/urlstr.h"
       #include "base/utils.h"
       #include "base/workers.h"
       #include "xpn_metadata.h"
       #include <libgen.h>


    /* ... Const / Const ................................................. */

       /* Operations */

       // File operations
       #define XPN_SERVER_OPEN_FILE        0
       #define XPN_SERVER_CREAT_FILE       1
       #define XPN_SERVER_READ_FILE        2
       #define XPN_SERVER_WRITE_FILE       3
       #define XPN_SERVER_CLOSE_FILE       4
       #define XPN_SERVER_RM_FILE          5
       #define XPN_SERVER_RM_FILE_ASYNC    6
       #define XPN_SERVER_RENAME_FILE      7
       #define XPN_SERVER_GETATTR_FILE     8
       #define XPN_SERVER_SETATTR_FILE     9

       // Directory operations
       #define XPN_SERVER_MKDIR_DIR        20
       #define XPN_SERVER_RMDIR_DIR        21
       #define XPN_SERVER_RMDIR_DIR_ASYNC  22
       #define XPN_SERVER_OPENDIR_DIR      23
       #define XPN_SERVER_READDIR_DIR      24
       #define XPN_SERVER_CLOSEDIR_DIR     25

       // FS Operations
       #define XPN_SERVER_STATFS_DIR       60

       // Metadata
       #define XPN_SERVER_READ_MDATA             70
       #define XPN_SERVER_WRITE_MDATA            71
       #define XPN_SERVER_WRITE_MDATA_FILE_SIZE  72

       // Connection operatons
       #define XPN_SERVER_FINALIZE     80
       #define XPN_SERVER_DISCONNECT   81
       #define XPN_SERVER_END          -1


    /* ... Data structures / Estructuras de datos ........................ */

       /* Portable types for 32/64-bit interoperability */

       #if !defined(HAVE_64BITS)
           // 32-bit system: use fixed-size types for network communication
           typedef uint64_t xpn_long64_t;
           typedef uint64_t xpn_size_t;
           typedef int64_t  xpn_ssize_t;
           typedef uint64_t xpn_ptr_t;
           typedef uint64_t xpn_dirptr_t;
       #else
           // 64-bit system: use native types
           typedef ssize_t  xpn_long64_t;
           typedef size_t   xpn_size_t;
           typedef ssize_t  xpn_ssize_t;
           typedef uint64_t xpn_ptr_t;
           typedef DIR *    xpn_dirptr_t;
       #endif


       /* Message struct */

       struct st_xpn_server_status
       {
           int         ret;
           int         server_errno;
       };

       struct st_xpn_server_path_flags
       {
           int         flags;
           mode_t      mode;
           char        xpn_session;
           int         file_type; // 0 - SCK_FILE; 1 - MQ_FILE;
           int         path_len;
           char        path[XPN_PATH_MAX];
       };

       struct st_xpn_server_path {
           int         path_len;
           char        path[XPN_PATH_MAX];
       };

       struct st_xpn_server_close
       {
           int           fd;
           int           file_type; // 0 - SCK_FILE; 1 - MQ_FILE;
           xpn_dirptr_t  dir;  // 32-bit: store as 64-bit integer for network transfer
           int           path_len;
           char          path[XPN_PATH_MAX];
       };

       struct st_xpn_server_rw
       {
           int           fd;
           int           file_type; // 0 - SCK_FILE; 1 - MQ_FILE;
           offset_t      offset;
           xpn_size_t    size;  // 32-bit: use fixed 64-bit size
           char          xpn_session;
           int           path_len;
           char          path[XPN_PATH_MAX];
       };

       struct st_xpn_server_rw_req
       {
           xpn_ssize_t   size;  // 32-bit: use fixed 64-bit signed size
           char          last;
           struct        st_xpn_server_status status;
       };

       struct st_xpn_server_rename
       {
           int      old_url_len;
           int      new_url_len;
           char     old_url[XPN_PATH_MAX];
           char     new_url[XPN_PATH_MAX];
       };

       struct st_xpn_server_setattr
       {
           struct   stat attr;
           int      path_len;
           char     path[XPN_PATH_MAX];
       };

       struct st_xpn_server_attr_req
       {
           struct   stat attr;
           struct   st_xpn_server_status status_req;
           char     status;
       };

       struct st_xpn_server_readdir
       {
           long          telldir;
           xpn_dirptr_t  dir;  // 32-bit: store as 64-bit integer for network transfer
           char          xpn_session;
           int           path_len;
           char          path[XPN_PATH_MAX];
       };

       struct st_xpn_server_opendir_req
       {
           xpn_long64_t  fd;   // Portable 64-bit file descriptor
           xpn_dirptr_t  dir;  // 32-bit: store as 64-bit integer for network transfer
           struct        st_xpn_server_status status;
       };

       struct st_xpn_server_readdir_req
       {
           int       end; //If end = 1 exist entry; 0 not exist
           long      telldir;
           struct    dirent ret;
           struct    st_xpn_server_status status;
       };

       struct st_xpn_server_read_mdata_req
       {
           struct    xpn_metadata mdata;
           struct    st_xpn_server_status status;
       };

       struct st_xpn_server_write_mdata
       {
           struct   xpn_metadata mdata;
           int      path_len;
           char     path[XPN_PATH_MAX];
       };

       struct st_xpn_server_write_mdata_file_size
       {
           xpn_ssize_t  size;  // 32-bit: use fixed 64-bit signed size
           int          path_len;
           char         path[XPN_PATH_MAX];
       };

       struct st_xpn_server_end {
           char status;
       };

       struct st_xpn_server_msg
       {
            int type;
            union
            {
               struct st_xpn_server_path_flags op_open;
               struct st_xpn_server_path_flags op_creat;
               struct st_xpn_server_close op_close;
               struct st_xpn_server_rw op_read;
               struct st_xpn_server_rw op_write;
               struct st_xpn_server_path op_rm;
               struct st_xpn_server_rename op_rename;
               struct st_xpn_server_path op_getattr;
               struct st_xpn_server_setattr op_setattr;

               struct st_xpn_server_path_flags op_mkdir;
               struct st_xpn_server_path_flags op_opendir;
               struct st_xpn_server_readdir op_readdir;
               struct st_xpn_server_close op_closedir;
               struct st_xpn_server_path op_rmdir;

               struct st_xpn_server_path op_read_mdata;
               struct st_xpn_server_write_mdata op_write_mdata;
               struct st_xpn_server_write_mdata_file_size op_write_mdata_file_size;

               struct st_xpn_server_end op_end;
            }
           u_st_xpn_server_msg;
       };


    /* ... Functions / Funciones ......................................... */

       static inline
       const char * xpn_server_op2string(int op_code)
       {
           switch (op_code)
	   {
               // File operations
           case XPN_SERVER_OPEN_FILE:
               return "OPEN";
           case XPN_SERVER_CREAT_FILE:
               return "CREAT";
           case XPN_SERVER_READ_FILE:
               return "READ";
           case XPN_SERVER_WRITE_FILE:
               return "WRITE";
           case XPN_SERVER_CLOSE_FILE:
               return "CLOSE";
           case XPN_SERVER_RM_FILE:
               return "RM";
           case XPN_SERVER_RM_FILE_ASYNC:
               return "RM_ASYNC";
           case XPN_SERVER_RENAME_FILE:
               return "RENAME";
           case XPN_SERVER_GETATTR_FILE:
               return "GETATTR";
           case XPN_SERVER_SETATTR_FILE:
               return "SETATTR";
               // Directory operations
           case XPN_SERVER_MKDIR_DIR:
               return "MKDIR";
           case XPN_SERVER_RMDIR_DIR:
               return "RMDIR";
           case XPN_SERVER_RMDIR_DIR_ASYNC:
               return "RMDIR_ASYNC";
           case XPN_SERVER_OPENDIR_DIR:
               return "OPENDIR";
           case XPN_SERVER_READDIR_DIR:
               return "READDIR";
           case XPN_SERVER_CLOSEDIR_DIR:
               return "CLOSEDIR";
               // FS Operations
           case XPN_SERVER_STATFS_DIR:
               return "STATFS";
           case XPN_SERVER_FINALIZE:
               return "FINALIZE";
               // Metadata
           case XPN_SERVER_READ_MDATA:
               return "READ_METADATA";
           case XPN_SERVER_WRITE_MDATA:
               return "WRITE_METADATA";
           case XPN_SERVER_WRITE_MDATA_FILE_SIZE:
               return "WRITE_METADATA_FILE_SIZE";
               // Connection operatons
           case XPN_SERVER_DISCONNECT:
               return "DISCONNECT";
           case XPN_SERVER_END:
               return "END";
           default:
               return "Unknown";
           }
       }

       int xpn_server_do_operation ( int server_type, struct st_th * th, int * the_end );


    /* ................................................................... */


  #ifdef __cplusplus
    }
  #endif

#endif

