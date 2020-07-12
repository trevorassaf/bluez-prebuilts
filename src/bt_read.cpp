#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <sstream>

int main(int argc, char **argv)
{
    struct sockaddr_rc addr = { 0 };
    int s, status;
    char dest[18] = "C8:3F:26:11:D7:D3";

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if( status < 0 ) {
      perror("uh oh");
      return EXIT_FAILURE;
    }

    size_t bt_packets_received = 0;
    while (true)
    {
      char buffer[1024];
      int result = read(s, buffer, sizeof(buffer));
      if (result < 0)
      {
        perror("uh oh");
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
    return 0;
}
