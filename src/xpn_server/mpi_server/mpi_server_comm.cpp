
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
#include "mpi_server_comm.hpp"
#include "base_c/debug_msg.h"
#include "base_cpp/timer.hpp"

namespace XPN
{

mpi_server_control_comm::mpi_server_control_comm(xpn_server_params &params) : m_thread_mode(params.have_threads())
{
  int ret, provided, claimed;

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] >> Begin\n", m_rank);

  //Get timestap
  timer timer;

  // MPI init
  // Threads disable
  if (!m_thread_mode)
  {
    debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] MPI Init without threads\n", m_rank);

    ret = MPI_Init(&(params.argc), &(params.argv));
    if (MPI_SUCCESS != ret)
    {
      printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] ERROR: MPI_Init fails\n", m_rank);
      return;
    }
  }
  // Threads enable
  else
  {
    debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] MPI Init with threads\n", m_rank);

    ret = MPI_Init_thread(&(params.argc), &(params.argv), MPI_THREAD_MULTIPLE, &provided);
    if (MPI_SUCCESS != ret)
    {
      printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] ERROR: MPI_Init_thread fails\n", m_rank);
      return;
    }

    MPI_Query_thread(&claimed);
    if (claimed == MPI_THREAD_SINGLE) {
      printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] INFO: your MPI implementation seem not supporting thereads\n", m_rank);
    }
  }

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] Get MPI rank\n", m_rank);

  ret = MPI_Comm_rank(MPI_COMM_WORLD, &(m_rank));
  if (MPI_SUCCESS != ret)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] ERROR: MPI_Comm_rank fails\n", m_rank);
    return;
  }

  ret = MPI_Comm_size(MPI_COMM_WORLD, &(m_size));
  if (MPI_SUCCESS != ret)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] ERROR: MPI_Comm_rank fails\n", m_rank);
    return;
  }
  // Open server port...
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] Open port\n", m_rank);

  ret = MPI_Open_port(MPI_INFO_NULL, m_port_name.data());
  if (MPI_SUCCESS != ret) {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] ERROR: MPI_Open_port fails\n", m_rank);
    return;
  }

  // Print server init information
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

  printf(" | * Time to initialize XPN MPI server: %f\n", timer.elapsed());

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] server %d available at %s\n", m_rank, m_rank, m_port_name.c_str());
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] server %d accepting...\n", m_rank, m_rank);

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_init] >> End\n", m_rank);
}

mpi_server_control_comm::~mpi_server_control_comm() 
{
  int ret;

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_destroy] >> Begin\n", 0);

  // Close port
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_destroy] Close port\n", 0);

  MPI_Close_port(m_port_name.data());

  // Finalize
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_destroy] MPI Finalize\n", 0);

  ret = MPI_Finalize();
  if (MPI_SUCCESS != ret)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_destroy] ERROR: MPI_Finalize fails\n", 0);
    return;
  }

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_destroy] << End\n", 0);
}

// accept, disconnect
xpn_server_comm* mpi_server_control_comm::accept ( )
{
  int ret;

  MPI_Comm comm;

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_accept] >> Begin\n", 0);

  // Accept
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_accept] Accept\n", 0);

  ret = MPI_Comm_accept(m_port_name.data(), MPI_INFO_NULL, 0, MPI_COMM_SELF, &comm);
  if (MPI_SUCCESS != ret)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_accept] ERROR: MPI_Comm_accept fails\n", 0);
    exit(EXIT_FAILURE);
  }

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_accept] << End\n", 0);
  
  return new (std::nothrow) mpi_server_comm(comm);
}

void mpi_server_control_comm::disconnect ( xpn_server_comm *comm )
{
  int ret;

  mpi_server_comm *in_comm = static_cast<mpi_server_comm*>(comm);

  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_disconnect] >> Begin\n", 0);

  if (in_comm->m_comm == MPI_COMM_NULL)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_disconnect] ERROR: The MPI_Comm is NULL\n", 0);
    return;
  }

  // Disconnect
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_disconnect] Disconnect\n", 0);

  ret = MPI_Comm_disconnect(&in_comm->m_comm);
  if (MPI_SUCCESS != ret)
  {
    printf("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_disconnect] ERROR: MPI_Comm_disconnect fails\n", 0);
    return;
  }

  delete comm;
  debug_info("[Server=%d] [MPI_SERVER_CONTROL_COMM] [mpi_server_control_comm_disconnect] << End\n", 0);
}

int64_t mpi_server_comm::read_operation ( int &op, int &rank_client_id, int &tag_client_id )
{
  int ret;
  MPI_Status status;
  int msg[2];

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_operation] >> Begin\n", 0);

  // Get message
  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_operation] Read operation\n", 0);

  ret = MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, 0, m_comm, &status);
  if (MPI_SUCCESS != ret) {
    debug_warning("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_operation] ERROR: MPI_Recv fails\n", 0);
  }

  rank_client_id = status.MPI_SOURCE;
  tag_client_id  = msg[0];
  op             = msg[1];

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_operation] MPI_Recv (MPI SOURCE %d, MPI_TAG %d, OP %d, MPI_ERROR %d)\n", 0, rank_client_id, rank_client_id, op, status.MPI_ERROR);
  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_operation] << End\n", 0);

  // Return OK
  return 0;
}


int64_t mpi_server_comm::read_data ( void *data, int64_t size, int rank_client_id, int tag_client_id )
{
  int ret;
  MPI_Status status;

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] >> Begin\n", 0);

  if (size == 0) {
    return  0;
  }
  if (size < 0)
  {
    printf("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] ERROR: size < 0\n", 0);
    return  -1;
  }

  // Get message
  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] Read data tag %d\n", 0, tag_client_id);

  ret = MPI_Recv(data, size, MPI_CHAR, rank_client_id, tag_client_id, m_comm, &status);
  if (MPI_SUCCESS != ret) {
    debug_warning("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] ERROR: MPI_Recv fails\n", 0);
  }

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] MPI_Recv (MPI SOURCE %d, MPI_TAG %d, MPI_ERROR %d)\n", 0, status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);
  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_read_data] << End\n", 0);

  // Return bytes read
  return size;
}

int64_t mpi_server_comm::write_data ( const void *data, int64_t size, int rank_client_id, int tag_client_id )
{
  int ret;

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_write_data] >> Begin\n", 0);

  if (size == 0) {
      return 0;
  }
  if (size < 0)
  {
    printf("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_write_data] ERROR: size < 0\n", 0);
    return -1;
  }

  // Send message
  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_write_data] Write data tag %d\n", 0, tag_client_id);

  ret = MPI_Send(data, size, MPI_CHAR, rank_client_id, tag_client_id, m_comm);
  if (MPI_SUCCESS != ret) {
    debug_warning("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_write_data] ERROR: MPI_Send fails\n", 0);
  }

  debug_info("[Server=%d] [MPI_SERVER_COMM] [mpi_server_comm_write_data] << End\n", 0);

  // Return bytes written
  return size;
}

/* ................................................................... */

} // namespace XPN
