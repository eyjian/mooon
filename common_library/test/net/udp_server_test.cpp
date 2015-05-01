// Copyright 2015, Tencent Inc.
// Author: yijian
// Date: 2015/4/28
#include "mooon/net/udp_socket.h"
MOOON_NAMESPACE_USE

int main(int argc, char* argv[])
{
    try
    {
        uint16_t port = 2015;
        net::CUdpSocket udp_socket;

        udp_socket.listen(port);
        printf("udp listen on: %d\n", port);

        while (true)
        {
            char buffer[548+1]; // 1472
            struct sockaddr_in from_addr;

            int bytes = udp_socket.receive_from(buffer, sizeof(buffer)-1, &from_addr);
            if (bytes > 0)
            {
                buffer[bytes] = '\0';
                printf("[%s] %s\n", net::to_string(from_addr).c_str(), buffer);

                char response[548+1];
                strcpy(response, "ok");
                udp_socket.send_to(response, strlen(response)+1, from_addr);
            }
        }
    }
    catch (sys::CSyscallException& syscall_ex)
    {
        fprintf(stderr, "%s\n", syscall_ex.str().c_str());
    }

    return 0;
}
