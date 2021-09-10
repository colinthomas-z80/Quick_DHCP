/*************************************
 *  Quick DHCP server for a single client.
 * 
 *  Assigns a single ip address
 *  format *.exe [NIC_ADDDRESS] [OFFER_ADDRESS]
 * 
 **************************************/

#include "quick_dhcp.h"

char *host_ip;
char *offer_ip;

int init_net(SOCKET *rx, SOCKET *tx);
DWORD WINAPI server_thread(LPVOID);

int main(int argc, char **argv){

    if(argc == 3)
    {
        printf("Host: %s\nOffer: %s\n", argv[1], argv[2]);

        host_ip = argv[1];
        offer_ip = argv[2];
    }else{
        printf("Incorrect Program Arguments\n\nFormat : \nquick_dhcp.exe [NIC_ADDRESS] [OFFER_ADDRESS]\n\n");
        return 1;
    }

    HANDLE my_thread;
    DWORD  my_thread_id;
    my_thread = CreateThread(
        NULL,
        0,
        server_thread,
        NULL,
        0,
        &my_thread_id
    );

    while(1);
    return 0;
}

DWORD WINAPI server_thread(LPVOID x)
{
    int iResult;
    SOCKET rx_socket, tx_socket;


    iResult = init_net(&rx_socket, &tx_socket);
    if(iResult != 0)
    {
        printf("Error initializing network\n");
        WSACleanup();
        return 1;
    }

    while(1)
    {
        // Listen for a DHCP Discover 
        iResult = discover_client(&rx_socket);
        if(iResult != 0)
        {
            printf("Error Listening for DHCP Discover : %d\n", iResult);
            WSACleanup();
            return 1;
        }
        
        // Create a RAW socket and send offer
        iResult = send_dhcp_offer(&tx_socket);
        if(iResult != 0)
        {
            printf("Error Sending UDP Response : %d\n", iResult);
            WSACleanup();
            return 1;
        }   

        // Wait for client dhcp request
        iResult = client_request(&rx_socket);
        if(iResult != 0)
        {
            printf("Error getting client request : %d\n", iResult);
            WSACleanup();
            return 1;
        }

        // Respond to client with ack
        iResult = client_ack(&tx_socket);
        if(iResult != 0)
        {
            printf("Error acknowledging client request : %d\n", iResult);
            WSACleanup();
            return 1;
        }
        printf("DHCP Success!\n\n");
    }
    return 0;
}

int init_net(SOCKET *rx, SOCKET *tx)
{
    WSADATA wsa;
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsa);
    if(iResult != 0)
    {
        printf("WSAStartup Failed : %d\n", iResult);
        return -1;
    }

    // Initialize sockets
    iResult = init_rx_socket(rx);
    if(iResult != 0)
    {
        printf("Error Creating RX Socket : %d\n", iResult);
        WSACleanup();
        return -1;
    }

    iResult = init_tx_socket(tx);
    if(iResult != 0)
    {
        printf("Error Creating TX Socket : %d\n", iResult);
        WSACleanup();
        return -1;
    }
    return 0;
}
