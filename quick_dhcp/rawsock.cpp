#include "quick_dhcp.h"

int udp_test(){
    SOCKET raw_s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(raw_s == INVALID_SOCKET)
    {
        printf("Error creating raw socker : %d\n", WSAGetLastError());
        return -1;
    }
    printf("UDP Socket Created\n");
    // bind socket to the network interface
    sockaddr_in my_nic;
    my_nic.sin_family = AF_INET;
    my_nic.sin_addr.s_addr = inet_addr("192.168.1.150");
    my_nic.sin_port = 68;
    int err = bind(raw_s, (SOCKADDR*)&my_nic, sizeof(my_nic));
    if(err != 0){
        printf("Error binding raw socker %d\n", err);
        return -1;
    }
    printf("Raw socket bound\n");

    bool optval = TRUE;
    err = setsockopt(raw_s, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL));
    if(err != 0)
    {
        printf("error setting sockopt : %d\n", err);
    }
    printf("Sockopt set\n");

    sockaddr_in receiver;
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(68);
    receiver.sin_addr.s_addr = inet_addr("255.255.255.255");

    char test_sendbuff[] = "This is a udp test";
    err = sendto(raw_s, test_sendbuff, (int)strlen(test_sendbuff), 0, (SOCKADDR *)&receiver, sizeof(receiver));
    if(err != 0)
    {
        printf("UDP Bytes Sent : %d\n", err);
    }
    return 0;
}