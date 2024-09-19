
/*
 *  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
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

#include "xpn/xpn_api.hpp"
#include "base_cpp/xpn_path.hpp"

namespace XPN
{
    int xpn_api::fstat(int fd, struct ::stat *sb)
    {
        XPN_DEBUG_BEGIN_CUSTOM(fd);
        int res = 0;

        if (m_files.find(fd) == m_files.end())
        {
            errno = EBADF;
            XPN_DEBUG_END_CUSTOM(fd);
            return -1;
        }

        // Redirect to stat to not duplicate code
        res = stat(m_files.at(fd).m_path.c_str(), sb);

        XPN_DEBUG_END_CUSTOM(fd);
        return res;
    }

    int xpn_api::stat(const char *path, struct ::stat *sb)
    {
        XPN_DEBUG_BEGIN_CUSTOM(path);
        int res = 0;
        std::string name_part = xpn_path::get_first_dir(path);
        if (m_partitions.find(name_part) == m_partitions.end())
        {
            errno = EBADF;
            XPN_DEBUG_END_CUSTOM(path);
            return -1;
        }
        xpn_partition& part = m_partitions.at(name_part);

        xpn_file file(path, part);

        if (file.m_mdata.read() < 0){
            XPN_DEBUG_END_CUSTOM(path);
            return -1;
        }
        
        auto& server = part.m_data_serv[file.m_mdata.master_file()];

        m_worker->launch([&server, &file, sb, &res](){res = server->nfi_getattr(file.m_path, *sb);});

        m_worker->wait();

        // Update file_size
        if (S_ISREG(sb->st_mode)){
            sb->st_size = file.m_mdata.m_data.file_size;
        }

        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::chown([[maybe_unused]] const char *path, [[maybe_unused]] uid_t owner, [[maybe_unused]] gid_t group)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::fchown([[maybe_unused]] int fd, [[maybe_unused]] uid_t owner, [[maybe_unused]] gid_t group)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::chmod([[maybe_unused]] const char *path, [[maybe_unused]] mode_t mode)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::fchmod([[maybe_unused]] int fd, [[maybe_unused]] mode_t mode)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::truncate([[maybe_unused]] const char *path, [[maybe_unused]] off_t length)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::ftruncate([[maybe_unused]] int fd, [[maybe_unused]] off_t length)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::dup([[maybe_unused]] int fd)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::dup2([[maybe_unused]] int fd, [[maybe_unused]] int fd2)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // TODO
        XPN_DEBUG_END;
        return res;
    }

    int xpn_api::statvfs(const char * path, struct ::statvfs *buf)
    {
        XPN_DEBUG_BEGIN_CUSTOM(path);
        int res = 0;

        std::string name_part = xpn_path::get_first_dir(path);
        if (m_partitions.find(name_part) == m_partitions.end())
        {
            errno = EBADF;
            res = -1;
            XPN_DEBUG_END_CUSTOM(path);
            return res;
        }
        xpn_partition& part = m_partitions.at(name_part);

        xpn_file file(path, part);

        auto& server = part.m_data_serv[file.m_mdata.master_file()];
                
        m_worker->launch([&server, buf, &res](){res = server->nfi_statvfs(*buf);});

        for (size_t i = 0; i < part.m_data_serv.size(); i++)
        {
            if (static_cast<int>(i) == file.m_mdata.master_file()) continue;
            struct ::statvfs aux_buf;
            m_worker->launch([&server, &aux_buf, &res](){res = server->nfi_statvfs(aux_buf);});
            buf->f_blocks += aux_buf.f_blocks;
            buf->f_bfree += aux_buf.f_bfree;
            buf->f_bavail += aux_buf.f_bavail;
        }
        

        m_worker->wait();

        XPN_DEBUG_END_CUSTOM(path);
        return res;
    }

    int xpn_api::fstatvfs(int fd, struct ::statvfs *buf)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        
        if (m_files.find(fd) == m_files.end())
        {
            errno = EBADF;
            XPN_DEBUG_END_CUSTOM(fd);
            return -1;
        }

        // Redirect to statvfs to not duplicate code
        res = statvfs(m_files.at(fd).m_path.c_str(), buf);

        XPN_DEBUG_END;
        return res;
    }
} // namespace XPN
