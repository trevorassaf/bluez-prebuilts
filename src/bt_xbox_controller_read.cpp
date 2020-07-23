#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>

#include <bluetooth/bluetooth.h>
#include <hci.h>

#include <algorithm>
#include <iostream>

bool OpenBtSocket(int *out_fd, int channel)
{
  *out_fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
  if (*out_fd < 0)
  {
    std::cerr << "Failed to open socket(): " << strerror(errno) << std::endl;
    close(*out_fd);
    return false;
  }

  struct sockaddr_hci address;
  memset(&address, 0, sizeof(address));
  address.hci_family = AF_BLUETOOTH;
  address.hci_dev = HCI_DEV_NONE;
  address.hci_channel = channel;

  int opt = 1;
  if (setsockopt(*out_fd, SOL_SOCKET, SO_TIMESTAMP, &opt, sizeof(opt)) < 0)
  {
    std::cerr << "Failed to setsockopt(SO_TIMESTAMP): " << strerror(errno) << std::endl;
    close(*out_fd);
    return false;
  }

  if (setsockopt(*out_fd, SOL_SOCKET, SO_PASSCRED, &opt, sizeof(opt)) < 0)
  {
    std::cerr << "Failed to setsockopt(SO_PASSCRED): " << strerror(errno) << std::endl;
    close(*out_fd);
    return false;
  }

  /*
  int flags = fcntl(*out_fd, F_GETFL, 0);
  if (fcntl(*out_fd, F_SETFL, flags | O_NONBLOCK) < 0)
  {
    std::cout << "Failed to set non-blocking mode for fd=" << *out_fd  << std::endl;
    return false;
  }
  */

  return true;
}

int HandleRead(int fd)
{
  std::cout << "Attempting to read from fd=" << fd << std::endl;

  uint8_t read_buffer[8];
  int result = read(fd, read_buffer, sizeof(read_buffer));

  if (result < 0)
  {
    std::cerr << "Failed to perform xbox controller read. fd="
              << fd << ". Error: " << strerror(errno) << std::endl;
    return false;
  }

  for (size_t i = 0; i < result; ++i)
  {
    std::cout << "read_buffer[" << i << "]=0x" << std::hex
              << read_buffer[i] << std::dec << std::endl;
  }

  return true;
}

int ReadXboxControllerInput(int monitor_channel_fd, int control_channel_fd)
{
  while (true)
  {
    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(monitor_channel_fd, &master_set);
    FD_SET(control_channel_fd, &master_set);
    int max_fd = std::max(monitor_channel_fd, control_channel_fd);
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int result = select(
        max_fd + 1,
        &master_set,
        nullptr,
        nullptr,
        &timeout);

    if (result < 0)
    {
      std::cerr << "Failed during select(): " << strerror(errno) << std::endl;
      return false;
    }
    else if (result == 0)
    {
      std::cerr << "Select timed out..." << std::endl;
    }

    for (int i = 0; i <= max_fd; ++i)
    {
      std::cout << "Testing fd=" << i << std::endl;

      if (FD_ISSET(i, &master_set))
      {
        if (!HandleRead(i))
        {
          std::cerr << "Failed to handle socket read. Fd=" << i << std::endl;
        }
      }
    }
  }

  return true;
}

int main(int argc, char** argv)
{
  int monitor_fd;
  int control_fd;

  std::cout << "Opening monitor socket..." << std::endl;

  if (!OpenBtSocket(&monitor_fd, HCI_CHANNEL_MONITOR))
  {
    std::cerr << "Failed to open monitor socket" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Successfully opened channel monitor BT socket. Fd=" << monitor_fd << std::endl;
  std::cout << "Opening channel socket..." << std::endl;

  if (!OpenBtSocket(&control_fd, HCI_CHANNEL_CONTROL))
  {
    std::cerr << "Failed to open control socket" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Successfully opened channel control BT socket. Fd=" << control_fd << std::endl;

  if (!ReadXboxControllerInput(monitor_fd, control_fd))
  {
    std::cerr << "Failed to read xbox controll data stream" << std::endl;
    return false;
  }

  return EXIT_SUCCESS;
}
