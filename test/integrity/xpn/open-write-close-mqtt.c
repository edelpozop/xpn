
#include "all_system.h"
#include "xpn.h"
#include <sys/time.h>
#include "mosquitto.h"

#define BUFF_SIZE (128)
#define MQTT_HOST "localhost"
#define MQTT_PORT 1883
#define PATH_MAX 256

double get_time(void)

{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);

}


void on_message_callback(struct mosquitto *mqtt, void *obj, const struct mosquitto_message *msg)
{
    if (NULL == obj) 
    {
        printf("ERROR: obj is NULL :-( \n") ;
    }
    //printf("%s\t%d\n", msg->topic, msg->payloadlen);

    // Copiar el mensaje en una variable local para manipularla
    char topic[PATH_MAX], path[PATH_MAX];

    int to_write1, offset;

    strncpy(topic, msg->topic, PATH_MAX);

    // Encontrar la posición del último y el penúltimo slash
    int last_slash = -1;
    int penultimate_slash = -1;
    for (int i = 0; topic[i] != '\0'; i++) {
        if (topic[i] == '/') {
            penultimate_slash = last_slash;
            last_slash = i;
        }
    }

    // Extraer el path y los dos enteros usando sscanf y las posiciones de los slashes
  
    if (penultimate_slash >= 0 && last_slash > penultimate_slash)
    {
        // Si hay dos slashes, extraer el path y ambos enteros
        strncpy(path, topic, penultimate_slash);
        path[penultimate_slash] = '\0';
        sscanf(&topic[penultimate_slash + 1], "%d/%d", &to_write1, &offset);

    }
    else if (last_slash >= 0)
    {
        // Si solo hay un slash, extraer solo el path y el primer entero
        strncpy(path, topic, last_slash);
        path[last_slash] = '\0';
        sscanf(&topic[last_slash + 1], "%d", &to_write1);
        offset = 0;

    } else {
        // Si no hay slashes, asumir que todo es el path
        strncpy(path, topic, PATH_MAX - 1);
        path[PATH_MAX - 1] = '\0';
        to_write1 = 0;
        offset = 0;
    }

    char * buffer = NULL;
    int size, diff, cont = 0, to_write = 0, size_written = 0;

    // initialize counters
    size = to_write1;
    if (size > MAX_BUFFER_SIZE) {
        size = MAX_BUFFER_SIZE;
    }
    diff = size - cont;

    //Open file
    int fd = xpn_open(path, O_CREAT|O_APPEND);
    if (fd < 0) {
        return;
    }

    // loop...
    do {
        if (diff > size) to_write = size;
        else to_write = diff;

        // read data from TCP and write into the file
        xpn_lseek(fd, offset + cont, SEEK_SET);
        size_written = xpn_write(fd, msg->payload, to_write);
        //nanosleep((const struct timespec[]){{0, 500000L}}, NULL);

        // update counters
        cont = cont + size_written; // Received bytes
        diff = to_write - cont;

    } while ((diff > 0) && (size_written != 0));


   	xpn_close(fd);
    free(buffer);
    //mosquitto_unsubscribe(mqtt, NULL, path);
}


int main ( int argc, char *argv[] )
{
	int  ret ;

	// xpn-init
	ret = xpn_init();
	printf("%d = xpn_init()\n", ret);
	if (ret < 0) {
		return -1;
	}


	struct mosquitto *mosq = NULL;

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Failed to create mosquitto instance.\n");
        return 1;
    }

    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 0) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Failed to connect to the MQTT broker.\n");
        mosquitto_destroy(mosq);
        return 1;
    }

    if (mosquitto_subscribe(mosq, NULL, "#", 0) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Failed to subscribe to the topic.\n");
        mosquitto_destroy(mosq);
        return 1;
    }

    mosquitto_message_callback_set(mosq, on_message_callback);

    while (mosquitto_loop(mosq, -1, 1) == MOSQ_ERR_SUCCESS) {
        // Loop infinito para mantener la conexión y recibir mensajes
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();


	//t_bc = get_time();

	//t_bw = get_time();

	
	
	//t_aw = get_time() - t_bw;

	//ret = xpn_close(fd1);

	//t_ac = get_time() - t_bc;

	//printf("%d = xpn_close(%d)\n", ret, fd1) ;

	//printf("Prueba: %d (128B), Tiempo Total: %fms; Tiempo Escritura: %f ms\n", atoi(argv[1]), t_ac * 1000, t_aw * 1000);

		// xpn-destroy
	printf("xpn_destroy()\n");
	ret = xpn_destroy();
	if (ret < 0) {
		printf("ERROR: xpn_destroy()\n");
		return -1;
	}

	return 0;
}
