#include "lwip.h"
#include "lwip/sockets.h"
#include "iolink_server.h"
#include "cmsis_os2.h"
#include <string.h>

osThreadId_t iolink_server_taskhandle;
const osThreadAttr_t iolink_server_attr = {
    .name = "IO-Link Server",
    .stack_size = 4096,
    .priority = osPriorityNormal
};


#define ETH_DBG 1

enum Dbg {
    NOTHING, 
    TASK_STARTED,
    LWIP_INITIALIZED,
    SOCKET_INITIALIZED,
    SOCKET_BOUND,
    LISTENING,
    BIND_FAILED,
    ACCEPTING,
    ACCEPTED_CLIENT,
};

extern struct netif gnetif;
volatile enum Dbg check = 0;
volatile char my_debug_ip[16] = {0};

void iolink_server_task(void* args) {
    osDelay(100);
    MX_LWIP_Init();
    check = LWIP_INITIALIZED;    // LWIP initialized
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) osThreadExit(); // Failsafe

    check = SOCKET_INITIALIZED;    // Socket intialized

    __attribute__((aligned(4))) struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5000);

    // If bind fails, the thread exits silently. 
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        check = BIND_FAILED;
        close(server_sock);
        osThreadExit(); 
    }

    check = SOCKET_BOUND; // Socket bound

    listen(server_sock, 1);

    check = LISTENING;    // Listening

    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr); 

        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len); 

        check = ACCEPTING;

        if(client_sock >= 0) {
            check = ACCEPTED_CLIENT;
            char rx_buffer[64];
            char tx_buffer[64];
            
            char *prompt = "Type 'on', 'off', or 'dist'.\n> ";
            send(client_sock, prompt, strlen(prompt), 0);

            while(1) {
                memset(rx_buffer, 0, sizeof(rx_buffer));
                memset(tx_buffer, 0, sizeof(tx_buffer));

                int bytes_received = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if(bytes_received > 0) {
                    rx_buffer[strcspn(rx_buffer, "\r\n")] = 0; // Strip newlines

                    strcpy(tx_buffer, rx_buffer);

                    // Send reply
                    send(client_sock, tx_buffer, strlen(tx_buffer), 0);
                } else {
                    break;
                }
            }
            close(client_sock); 
        }
    }
}

void start_iolink_server(void) {
    if(iolink_server_taskhandle == NULL) {
        iolink_server_taskhandle = osThreadNew(iolink_server_task, NULL, &iolink_server_attr);
    }
}


// static void process_cmd(char cmd[static 1], char reply[static 1]) {
//     static O1D100_Data_t laser_data;
//     if (strcmp(cmd, "on") == 0) {
//         O1D100_Start(MAX14819_PORT_B);
//         sprintf(reply, "Laser waking...\n> ");
//     }
//     else if (strcmp(cmd, "off") == 0) {
//         O1D100_Stop(MAX14819_PORT_B);
//         sprintf(reply, "Laser sleeping...\n> ");
//     }
//     else if (strcmp(cmd, "dist") == 0) {
//         O1D100_ReadData(MAX14819_PORT_B, &laser_data);
//         uint16_t dist = laser_data.distance_mm;
//         sprintf(reply, "Distance: %u mm\n> ", dist);
//     }
// }