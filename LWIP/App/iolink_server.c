#include "lwip.h"
#include "lwip/sockets.h"
#include "iolink_server.h"
#include "cmsis_os2.h"
#include <string.h>

#define ETH_DBG 0
#define LSR_DBG 1
#define ALL_DBG 0

osThreadId_t iolink_server_taskhandle;
const osThreadAttr_t iolink_server_attr = {
    .name = "IO-Link Server",
    .stack_size = 4096,
    .priority = osPriorityNormal
};

static void process_cmd(char cmd[static 1], char reply[static 1]);

#if ETH_DBG
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
#elif LSR_DBG
enum Dbg {
    NOTHING,
    TASK_STARTED,
    MAX_INITIALIZED,
    PORT_ACTIVATED,
    CONNECTION_ESTABLISHED,
    LASER_STARTED,
    LASER_READ,
    LASER_READ_FAILED,
    ID_VERIFY_FAIL,
    ID_VERIFIED
};
#endif

extern struct netif gnetif;
volatile enum Dbg check = 0;
IOLink_Com_t com_speed;
O1D100_Data_t laser_data;

volatile char my_debug_ip[16] = {0};

#if ETH_DBG
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

#elif LSR_DBG
void iolink_server_task(void* args) {
    MAX14819_Init();
    check = MAX_INITIALIZED;

    volatile uint8_t clock_valid = MAX14819_CheckClock();

    MAX14819_ActivatePort(MAX14819_PORT_B, 1000, MAX14819_CLIM_300mA);
    check = PORT_ACTIVATED;
    com_speed = MAX14819_EstablishConnection(MAX14819_PORT_B, 1000);
    if(com_speed == IOLINK_DETECTION_FAIL) {
        osThreadExit();
    } 
    check = CONNECTION_ESTABLISHED;

    osDelay(100);

    bool id_verified = O1D100_VerifyIdentity(MAX14819_PORT_B);
    if (!id_verified) {
        check = ID_VERIFY_FAIL;
        //osThreadExit();
    } else {
        check = ID_VERIFIED;
    }

    O1D100_Start(MAX14819_PORT_B);
    check = LASER_STARTED;
    while (1) {
        if(O1D100_ReadData(MAX14819_PORT_B, &laser_data)) {
            check = LASER_READ;
        } else {
            check = LASER_READ_FAILED;
        }
        osDelay(10);
    }
}

#elif ALL_DBG
void iolink_server_task(void* args) {
    MX_LWIP_Init();

    // Attempt to connect to laser
    MAX14819_ActivatePort(MAX14819_PORT_B, 1000, MAX14819_CLIM_200mA);
    check++;
    com_speed = MAX14819_EstablishConnection(MAX14819_PORT_B, 1000);
    check++;

    if (com_speed == IOLINK_DETECTION_FAIL) {
        // No connection
        check--;
    }

    check++;
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) osThreadExit(); // Failsafe

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5000);

    // If bind fails, the thread exits silently. 
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_sock);
        osThreadExit(); 
    }

    listen(server_sock, 1);

    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr); 

        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len); 

        if(client_sock >= 0) {
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

                    if(strcmp(rx_buffer, "quit") == 0) {
                        sprintf(tx_buffer, "Disconnecting...\n");
                        send(client_sock, tx_buffer, strlen(tx_buffer), 0);
                        break;
                    }

                    process_cmd(rx_buffer, tx_buffer);
                    
                    if(strlen(tx_buffer) == 0) {
                        strcpy(tx_buffer, "Unknown command\n> ");
                    }

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

#endif

void start_iolink_server(void) {
    if(iolink_server_taskhandle == NULL) {
        iolink_server_taskhandle = osThreadNew(iolink_server_task, NULL, &iolink_server_attr);
    }
}


static void process_cmd(char cmd[static 1], char reply[static 1]) {
    static O1D100_Data_t laser_data;
    if (strcmp(cmd, "on") == 0) {
        O1D100_Start(MAX14819_PORT_B);
        sprintf(reply, "Laser waking...\n> ");
    }
    else if (strcmp(cmd, "off") == 0) {
        O1D100_Stop(MAX14819_PORT_B);
        sprintf(reply, "Laser sleeping...\n> ");
    }
    else if (strcmp(cmd, "dist") == 0) {
        O1D100_ReadData(MAX14819_PORT_B, &laser_data);
        uint16_t dist = laser_data.distance_mm;
        sprintf(reply, "Distance: %u mm\n> ", dist);
    }
}