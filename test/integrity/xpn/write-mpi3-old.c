#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <mpi.h>

#include "all_system.h"

#include "xpn.h"

#include <time.h>

//#define PATH_MAX 256

double get_time(void)

{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday( & tp, & tzp);
    return ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);

}

int main(int argc, char * argv[]) 
{
    int rank, ret;
    int fd1;
    char filename[1024];

    double start_time;
    double used_time;
    double avg_time;
    double us_rate;

    char * slurm_procid = getenv("SLURM_PROCID");
    rank = atoi(slurm_procid);
    int ppn = atoi(argv[1]); 
    int node = (rank / ppn) + 1; 
    int sensor = (rank % ppn) + 1; 


    //sprintf(filename, "/P1/test-%d-%d", sensor, node);
    sprintf(filename, "/P1/test");
    int num_its = 10000;
    int payload_size = atoi(argv[2]);
    char buf1[payload_size + 1];
    memset(buf1, 'a', payload_size);
    buf1[payload_size] = '\n';

    /*char bufferFirst[payload_size + 1 + 8 + 6];
    sprintf(bufferFirst,"C%d-S%03d;First;", node, sensor);
    strcat (bufferFirst, buf1);

    char bufferLast[payload_size + 1 + 8 + 5];
    sprintf(bufferLast, "C%d-S%03d;Last;", node, sensor);
    strcat(bufferLast, buf1);*/

    char buffer[payload_size + 1 + 8];
    sprintf(buffer, "C%d-S%03d;", node, sensor);
    strcat (buffer, buf1);
    //strcat (buffer, '\n');

    struct timespec dur, dur2;
    dur.tv_sec = 0;
    dur.tv_nsec = (sensor % 100) * 1000;
    dur2.tv_sec = 0;
    dur2.tv_nsec = (sensor % 50) * 1000;

    nanosleep( & dur2, NULL);

    // xpn-init
    ret = xpn_init();
    if (ret < 0) 
    {
        return -1;
    }

    nanosleep( & dur, NULL);

    int index = (ppn * (node - 1)) + (sensor - 1);
    int position = (((payload_size + 1 + 8) * (num_its - 2)) + ((payload_size + 1 + 8 + 4) * 2)) * index;

    //char *mq_outenv = getenv("MQOUT_TXT");

    fd1 = xpn_open(filename, O_WRONLY);

    if (fd1 < 0)
    {
        printf("[TCP-CLIENT] Error: main\n");
        return -1;
    }

    xpn_lseek(fd1, position, SEEK_SET);

    // Iniciar el reloj
    avg_time = 0.0;
    start_time = get_time();

    char bufStart[payload_size + 1 + 8 + 4];
    sprintf(bufStart, "C%d-S%03d;INI;", node, sensor);
    strcat (bufStart, buf1);
    
    ret = xpn_write(fd1, bufStart, payload_size + 1 + 8 + 4);
    nanosleep((const struct timespec[]) {{0,10000000L}}, NULL);

    for (int i = 1; i < num_its - 1; i++) 
    {
        ret = xpn_write(fd1, buffer, payload_size + 1 + 8);
        //printf("%d = xpn_write_%d(%d, %p, %lu)\n", ret, i, fd1, buffer, (unsigned long)BUFF_SIZE);
        nanosleep((const struct timespec[]) {{0,10000000L}}, NULL);
    }

    char bufFin[payload_size + 1 + 8 + 4];
    sprintf(bufFin, "C%d-S%03d;FIN;", node, sensor);
    strcat (bufFin, buf1);
	
    ret = xpn_write(fd1, bufFin, payload_size + 1 + 8 + 4);
    nanosleep((const struct timespec[]) {{0,10000000L}}, NULL);

    used_time = (get_time() - start_time) - (0.010 * num_its);
    //used_time = (get_time() - start_time);

    avg_time = used_time;

    avg_time = avg_time / (float) num_its;

    if (avg_time > 0) /* rate is megabytes per second */
        us_rate = (double)((payload_size) / (avg_time * (double) 1000000));
    else
        us_rate = 0.0;

    printf("%d;%.8f;%.8f\n", payload_size, avg_time, us_rate);

    ret = xpn_close(fd1);

    // xpn-destroy

    ret = xpn_destroy();
    if (ret < 0) {
        //MPI_Finalize();
        printf("ERROR: xpn_destroy()\n");
        return -1;
    }
    return 0;
}
