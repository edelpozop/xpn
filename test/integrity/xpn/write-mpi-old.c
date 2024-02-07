#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>
#include "all_system.h"
#include "xpn.h"
#include <signal.h>

#define BUFFER_SIZE 128

/* Catch Signal Handler functio */
void signal_callback_handler(int signum){

    printf("Caught signal SIGPIPE client %d\n",signum);
}

double get_time(void)

{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);

}

int main(int argc, char* argv[]) {
    signal(SIGPIPE, signal_callback_handler);
    int rank, size, ret;
    int fd1;
    char filename[PATH_MAX];
    char buffer[BUFFER_SIZE];
    double bef_creat, aft_clo, bef_wr, aft_wr, aft_creat ;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // 0..size-1
    MPI_Comm_size(MPI_COMM_WORLD, &size); // 40

    int ppn    = atoi(argv[2]);        // ppn

    // Semilla para generar números aleatorios
    srand(time(NULL));

    // Generar un número aleatorio entre 100 y 500
    //int tiempo = rand() % 401 + 100;
    //int tiempo2 = rand() % 201 + 100;

    // Convertir el tiempo a segundos y nanosegundos para la función 'nanosleep()'
    struct timespec duracion, duracion2;
    
    int node =      (rank / ppn) + 1;               //Este valor va a ser de 1 a 4
    int sensor =    (rank % ppn) + 1;               //Este valor es de 1 a 10

    duracion.tv_sec = 0;
    duracion.tv_nsec = ( sensor % 100 ) * 1000;
    duracion2.tv_sec = 0;
    duracion2.tv_nsec = ( sensor % 50 ) * 1000;

    bzero(filename, PATH_MAX);

    // Cada proceso MPI tiene su propio nombre de archivo único
    snprintf(filename, PATH_MAX, "/P1/test-%d-%d", sensor, node);

    //printf("path_creation %s\n", filename);

    MPI_Barrier(MPI_COMM_WORLD);
    //printf ("PreInit\n");
    nanosleep(&duracion2, NULL);
    // xpn-init
    ret = xpn_init();

    if (ret < 0) 
    {
        printf("%d = xpn_init() - %s\n", ret, filename);
        MPI_Finalize();
        return -1;
    }
    //printf ("PostInit\n");
    nanosleep(&duracion, NULL);
    

    //printf("xpn_creat_before %s\n", filename);
    //printf ("PreCreat\n");
    bef_creat = get_time();
    fd1 = xpn_creat(filename, 00777);
    aft_creat = get_time();
    //printf ("PostCreat\n");

    //printf("xpn_creat_after %s\n", filename);

    if ( fd1 < 0 ) 
    {
        MPI_Finalize();
        return -1;
    }
    //printf("%d = xpn_creat('%s', %o)\n", ret, argv[2], 00777);

    memset(buffer, 'a', BUFFER_SIZE) ;

    //nanosleep(&duracion, NULL);    

    // Sincronizar todos los procesos antes de comenzar a escribir
    MPI_Barrier(MPI_COMM_WORLD);

    bef_wr = get_time();
    //printf ("PreWrite\n");
    for (int i = 0; i < atoi(argv[1]); i++)
    {
        ret = xpn_write(fd1, buffer, BUFFER_SIZE);
        
        if (ret < 0)
        {
            //printf("Error write %d - %s\n", ret, filename);
            xpn_close(fd1);
            MPI_Finalize();
            return -1;
        }

        //printf("%d = xpn_write_%d(%d, %p, %lu)\n", ret, i, fd1, buffer, (unsigned long)BUFFER_SIZE);
        nanosleep((const struct timespec[]){{0, 500000L}}, NULL);
        //nanosleep((const struct timespec[]){{0, 250000000L}}, NULL);
    }
    //printf ("PostWrite\n");
    aft_wr = get_time();
    //printf ("PreClose\n");
    ret = xpn_close(fd1);

    if (ret < 0) 
    {
        //printf("Error close\n");
        MPI_Finalize();
        return -1;
    }
    //printf ("PostClose\n");
    aft_clo = get_time();

    //printf("%d = xpn_close(%d)\n", ret, fd1) ;

    printf("Tiempo Total: %f ms; Tiempo Escritura: %f ms\n", 
        ((aft_creat - bef_creat) + (aft_wr - bef_wr) + (aft_clo - aft_wr))  * 1000, (aft_wr - bef_wr) * 1000);


    // Sincronizar todos los procesos antes de terminar
    //MPI_Barrier(MPI_COMM_WORLD);


    // xpn-destroy
    ret = xpn_destroy();
    if (ret < 0) {
        printf("ERROR: xpn_destroy()\n");
        return -1;
    }

    MPI_Finalize();
    return 0;
}
