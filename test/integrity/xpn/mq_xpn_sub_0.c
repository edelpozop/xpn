#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include "all_system.h"
#include "xpn.h"
#include <time.h>

pthread_mutex_t m;

double get_time(void)
{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) 
{
    
    if (message->payloadlen > 0) 
    {

        double start_time = 0.0, total_time = 0.0;

        char filename[PATH_MAX];
        sprintf(filename,"%s", message->topic);

        start_time = get_time();

        pthread_mutex_lock(&m);
        int fd = xpn_open(filename, O_WRONLY);
        if (fd < 0)
        {
            printf("[CLIENT] Error: creating file %s\n", filename);
            return;
        }
        pthread_mutex_unlock(&m);

        pthread_mutex_lock(&m);
        int ret = xpn_lseek(fd, 0, SEEK_END);
        pthread_mutex_unlock(&m);
        if (ret < 0)
        {
            printf("[CLIENT] Error: lseek file %s\n",filename);
        }

        int buflen = message->payloadlen;
        char payload [buflen];
        sprintf(payload, "%s", (char*) message->payload);

        pthread_mutex_lock(&m);
        ret = xpn_write(fd, payload, buflen);
        pthread_mutex_unlock(&m);
        if (ret < 0)
        {
            printf("[CLIENT] Error: writing file %s\n",filename);
        }

        pthread_mutex_lock(&m);
        ret = xpn_close(fd);
        if (ret < 0)
        {
            printf("[CLIENT] Error: closing file %s\n", filename);
            return;
        }
        pthread_mutex_unlock(&m);

        total_time = (get_time() - start_time);
        printf("%s;%.5f\n", message->topic, total_time);
    }
}

int main(int argc, char* argv[]) 
{
    struct mosquitto *mosq  = NULL;
    const char *host        = "localhost"; 
    int port                = 1883;
    int ret                 = 0;

    if (pthread_mutex_init(&m, NULL) != 0)
    {
        printf("Mutex init failed\n");
        return 1;
    }

    printf("SUBSCRIBER MQ-XPN QOS0\n");

    ret = xpn_init();

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) 
    {
        fprintf(stderr, "Error: No se pudo inicializar el cliente MQTT.\n");
        return 1;
    }

    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) 
    {
        fprintf(stderr, "Error: No se pudo conectar al servidor MQTT.\n");
        return 1;
    }

    const char *topic = "/#";

    mosquitto_subscribe(mosq, NULL, topic, 0);

    mosquitto_message_callback_set(mosq, message_callback);

    while (mosquitto_loop(mosq, -1, 1) == MOSQ_ERR_SUCCESS) {}

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    ret = xpn_destroy();

    if (ret < 0) 
    {
        printf("ERROR: xpn_destroy()\n");
        return -1;
    }

    pthread_mutex_destroy(&m);

    return 0;
}
