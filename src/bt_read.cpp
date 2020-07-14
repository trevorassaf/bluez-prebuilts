#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/rfcomm.h>

#include <cstdint>
#include <iostream>
#include <sstream>

int main(int argc, char **argv)
{
    struct sockaddr_l2 addr = { 0 };
    int s, status;
    char dest[18] = "C8:3F:26:11:D7:D3";

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    //addr.l2_psm = htobs(0x1001);
    addr.l2_psm = htobs(17);
    //addr.l2_psm = htobs();
    str2ba( dest, &addr.l2_bdaddr );

    // connect to server
    std::cout << "Before connect()" << std::endl;
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    std::cout << "After connect()" << std::endl;
    if (status < 0)
    {
      perror("connect");
      return EXIT_FAILURE;
    }

    std::cout << "Before read loop" << std::endl;
    size_t bt_packets_received = 0;
    while (true)
    {
      uint8_t buffer[64];
    std::cout << "Before read iteration: " << bt_packets_received << std::endl;
      int result = recv(s, buffer, sizeof(buffer), 0);
    std::cout << "After read iteration: " << bt_packets_received << std::endl;
      if (result < 0)
      {
        perror("uh oh");
        return EXIT_FAILURE;
      }
      else
      {
        std::stringstream stream;
        for (size_t i = 0; i < result; ++i)
        {
          stream << std::dec << "buffer[" << i << "]=" << std::hex << "0x" << (int)buffer[result] << std::endl;
        }

        std::cout << "Bluetooth buffer read: " << bt_packets_received << std::endl;
        std::cout << stream.str() << std::endl;
      }

      ++bt_packets_received;
    }

    close(s);
    return EXIT_SUCCESS;
}
