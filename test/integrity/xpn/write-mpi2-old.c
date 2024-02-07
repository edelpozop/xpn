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

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);

}

int main(int argc, char* argv[]) {
    int rank, ret;
    int fd1;
    char filename[PATH_MAX];
    
    
    double start_time;
    double used_time;
    double avg_time;
    double us_rate;    

    //MPI_Init(&argc, &argv);
    //MPI_Comm_rank(MPI_COMM_WORLD, &rank); // 0..size-1
    //MPI_Comm_size(MPI_COMM_WORLD, &size); // 40
    char * slurm_procid = getenv("SLURM_PROCID");
    rank = atoi(slurm_procid);
    int ppn =       atoi(argv[1]);                  //ppn
    int node =      (rank / ppn) + 1;               //Este valor va a ser de 1 a 4
    int sensor =    (rank % ppn) + 1;               //Este valor es de 1 a 10

    // Cada proceso MPI tiene su propio nombre de archivo único
    snprintf(filename, sizeof(filename), "/P1/test-%d-%d", sensor, node);

    int num_its             = 10000;
    int payload_size        = atoi (argv[2]);
    char buffer[payload_size];

	struct timespec dur, dur2;
	dur.tv_sec = 0;
	dur.tv_nsec = (sensor % 100) * 1000;
	dur2.tv_sec = 0;
	dur2.tv_nsec = (sensor % 50) * 1000;

//	MPI_Barrier(MPI_COMM_WORLD);

	nanosleep(&dur2, NULL);

    // xpn-init
    ret = xpn_init();
//	printf("preinit\n");    
    if (ret < 0) 
    {
        //MPI_Finalize();
        return -1;
    }
//	printf("postinit\n");

	nanosleep(&dur, NULL);

    fd1 = xpn_creat(filename, 00777);

	if (fd1 < 0)
	{
		//MPI_Finalize();
		return -1;
	}


//printf("postcreat\n");
    memset(buffer, 'a', payload_size) ;

    // Sincronizar todos los procesos antes de comenzar a escribir
  //  MPI_Barrier(MPI_COMM_WORLD);
//	printf("%s\n",filename);
    // Iniciar el reloj
    avg_time = 0.0;
    start_time = get_time();

    for (int i = 0; i < num_its; i++)
    {
        ret = xpn_write(fd1, buffer, payload_size);
	//if (ret < 0 )printf("ERROR ---------------------------------------------- xpn_write\n");
        //printf("%d = xpn_write_%d(%d, %p, %lu)\n", ret, i, fd1, buffer, (unsigned long)BUFF_SIZE);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
    }

    used_time = (get_time() - start_time) - (0.005 * num_its);
	//used_time = (get_time() - start_time);

    avg_time = used_time;

    avg_time = avg_time / (float) num_its;

    if (avg_time > 0) /* rate is megabytes per second */
        us_rate = (double)((payload_size) / (avg_time * (double) 1000000));
    else
        us_rate = 0.0;

    printf("%d;%.8f;%.8f\n", payload_size, avg_time, us_rate);

    ret = xpn_close(fd1);


    //printf("%d = xpn_close(%d)\n", ret, fd1) ;

    //printf("Prueba: %d (128B), Tiempo Total: %f ms; Tiempo Escritura: %f ms\n", 
    //    atoi(argv[1]), (aft_clo - bef_wr) + (aft - bef_creat) * 1000, (aft_wr - bef_wr) * 1000);


    // Sincronizar todos los procesos antes de terminar
  //  MPI_Barrier(MPI_COMM_WORLD);


        // xpn-destroy
    //printf("xpn_destroy()\n");
    ret = xpn_destroy();
    if (ret < 0) {
	//MPI_Finalize();
        printf("ERROR: xpn_destroy()\n");
        return -1;
    }

    //MPI_Finalize();
    return 0;
}
