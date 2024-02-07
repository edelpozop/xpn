#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include "all_system.h"
#include "xpn.h"
#include <time.h>

int fd = 0;

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) 
{
    
    if (message->payloadlen > 0) 
    {
        char filename[1024];
        sprintf(filename,"%s", message->topic);
        
        int buflen = message->payloadlen;
        char payload [buflen+1024];
        sprintf(payload, "%s;%s\n", filename, (char*) message->payload);
        int ret = xpn_write(fd, payload, buflen);
        if (ret < 0) printf ("ERROR xpn_write\n");
    }
}

int main(int argc, char* argv[]) 
{
    struct mosquitto *mosq  = NULL;
    const char *host        = "localhost"; 
    int port                = 1883;
    int ret                 = 0;

    ret = xpn_init();


    //////////////////////////////////////////////

    char *mq_outenv = getenv("MQOUT_TXT");

    fd = xpn_creat(mq_outenv, 0777);

    if (fd < 0)
    {
        printf("[TCP-CLIENT] Error: main\n");
        return -1;
    }

    //////////////////////////////////////////////


    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) 
    {
        fprintf(stderr, "Error: No se pudo inicializar el cliente MQTT.\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, message_callback);

    #ifndef MOSQ_OPT_TCP_NODELAY
    #define MOSQ_OPT_TCP_NODELAY 1
    #endif

    mosquitto_int_option(mosq, MOSQ_OPT_TCP_NODELAY, 1);

    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) 
    {
        fprintf(stderr, "Error: No se pudo conectar al servidor MQTT.\n");
        return 1;
    }

//    int rc = mosquitto_loop_start(mosq);

    const char *topic = "/#";

    mosquitto_subscribe(mosq, NULL, topic, 2);

    //mosquitto_message_callback_set(mosq, message_callback);

    while (mosquitto_loop(mosq, -1, 1) == MOSQ_ERR_SUCCESS) {}

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    ret = xpn_close(fd);

    if (ret < 0) 
    {
        xpn_destroy();
        printf("ERROR: xpn_close()\n");
        return -1;
    }

    ret = xpn_destroy();

    if (ret < 0) 
    {
        printf("ERROR: xpn_destroy()\n");
        return -1;
    }

    return 0;
}
