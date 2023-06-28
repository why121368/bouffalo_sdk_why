
/**
 * @file
 * A simple program that subscribes to a topic.
 */
#include "FreeRTOS_POSIX.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <lwip/errno.h>
#include <netdb.h>

#include "utils_getopt.h"

#include "mqtt.h"
#include "shell.h"

shell_sig_func_ptr abort_exec;

// static TaskHandle_t client_daemon;

/**
 * @brief The function will be called whenever a PUBLISH message is received.
 */
void publish_callback1(void **unused, struct mqtt_response_publish *published);

/**
 * @brief The client's refresher. This function triggers back-end routines to
 *        handle ingress/egress traffic to the broker.
 *
 * @note All this function needs to do is call \ref __mqtt_recv and
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that
 *       client ingress/egress traffic will be handled every 100 ms.
 */
void *client_refresher(void *client);

/**
 * @brief Safelty closes the \p sockfd and cancels the \p client_daemon before \c exit.
 */
void exit_example(int status, int sockfd, pthread_t *client_daemon);


static void test_close(int sig)
{
    abort_exec(sig);
    // exit_example(EXIT_SUCCESS, sockfd, NULL);

}
int open_nb_socket(const char* addr, const char* port);

int open_nb_socket(const char* addr, const char* port) {
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;

    /* get address information */
    rv = getaddrinfo(addr, port, &hints, &servinfo);
    if(rv != 0) {
        // fprintf(stderr, "Failed to open socket (getaddrinfo): %s\n", gai_strerror(rv));
        printf("Failed to open socket (getaddrinfo)\n");
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        /* connect to server */
        rv = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if(rv == -1) {
          close(sockfd);
          sockfd = -1;
          continue;
        }
        break;
    }  

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* make non-blocking */

    if (sockfd != -1) {
        fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, O_NONBLOCK) | O_NONBLOCK);
    }

    /* return the new socket fd */
    return sockfd;
}

// #define MQTT_PORT 1883
// int open_nb_socket(const char *addr, const char *port)
// {
//     long unsigned int noblock = 1;

//     struct sockaddr_in dest;
//     memset(&dest, 0, sizeof(dest));
//     dest.sin_family = AF_INET;
//     dest.sin_port = htons(MQTT_PORT);
//     dest.sin_addr.s_addr = inet_addr(addr);

//     int fd = -1;


//     if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         printf("Error in socket\r\n");
//         return -1;
//     }

//     if (connect(fd, (struct sockaddr *)&dest, sizeof(dest)) != 0) {
//         printf("Error in connect\r\n");
//         close(fd);
//         //vPortFree(recv_buffer);
//         while (1)
//             ;
//     }
// #if 0
//     struct addrinfo hints = { 0 };
//     struct addrinfo *p, *servinfo;
//     int rv;

//     hints.ai_family = AF_UNSPEC;     /* IPv4 or IPv6 */
//     hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
    
//     /* get address information */
//     rv = getaddrinfo(addr, port, &hints, &servinfo);
//     if (rv != 0) {
//         fprintf(stderr, "Failed to open socket (getaddrinfo): %s\n", rv);
//         return -1;
//     }

//     /* open the first possible socket */
//     for (p = servinfo; p != NULL; p = p->ai_next) {
//         fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
//         if (fd == -1)
//             continue;

//         /* connect to server */
//         rv = connect(fd, p->ai_addr, p->ai_addrlen);
//         printf("rv is %d\r\n", rv);
//         if (rv == -1) {
//             close(fd);
//             fd = -1;
//             continue;
//         }
//         break;
//     }

//     /* free servinfo */
//     freeaddrinfo(servinfo);

//     /* make non-blocking */
// #endif
//     // if (fd != -1)
//     //     fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
//     if (fd >= 0) {
//         ioctl(fd, FIONBIO, &noblock);
//     }
//     /* return the new socket fd */
//     return fd;
// }
uint8_t sendbuf[2048] = {0}; /* sendbuf should be large enough to hold multiple whole mqtt messages */
uint8_t recvbuf[1024] = {0}; /* recvbuf should be large enough any whole mqtt message expected to be received */
int example_mqtt(int argc, const char *argv[])
{
    abort_exec = shell_signal(SHELL_SIGINT, test_close);
    const char* addr;
    const char* port;
    const char* topic;

    /* get address (argv[1] if present) */
    if (argc > 1) {
        addr = argv[1];
    } else {
        addr = "test.mosquitto.org";
    }

    /* get port number (argv[2] if present) */
    if (argc > 2) {
        port = argv[2];
    } else {
        port = "1883";
    }

    /* get the topic name to publish */
    if (argc > 3) {
        topic = argv[3];
    } else {
        topic = "datetime";
    }

    /* open the non-blocking TCP socket (connecting to the broker) */
    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    // uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    // uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback1);
    /* Create an anonymous session */
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        exit_example(EXIT_FAILURE, sockfd, NULL);

    }

    /* subscribe */
    mqtt_subscribe(&client, topic, 0);

    /* start publishing the time */
    printf("%s listening for '%s' messages.\n", argv[0], topic);
    printf("Press CTRL-D to exit.\n\n");

    /* block */
    while(1)
    {
        vTaskDelay(100);
    }
    // while(fgetc(stdin) != EOF);

    /* disconnect */
    printf("\n%s disconnecting from %s\n", argv[0], addr);
    sleep(1);

    /* exit */
    exit_example(EXIT_SUCCESS, sockfd, &client_daemon);
}

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1){
        close(sockfd);
    }
    
    if (client_daemon != NULL) {
        pthread_cancel(*client_daemon);
    }
    exit(status);
}

void publish_callback1(void** unused, struct mqtt_response_publish *published)
{
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char* topic_name = (char*) malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';

    printf("Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);

    free(topic_name);
}

void* client_refresher(void* client)
{
    while(1)
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}

#ifdef CONFIG_SHELL
#include <shell.h>

int cmd_mqtt_subscribe(int argc, const char **argv)
{
    example_mqtt(argc, argv);

    return 0;
}

SHELL_CMD_EXPORT_ALIAS(cmd_mqtt_subscribe, mqtt_sub, mqtt subscribe);
#endif
