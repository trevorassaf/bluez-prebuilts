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

#include <signal.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include <bluetooth/bluetooth.h>
#include <hci.h>

#include <algorithm>
#include <iostream>

struct mgmt_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
} __packed;

struct XBoxControllerData
{
  int fd;
  const char *name;
};

bool OpenBtSocket(int *out_fd, int channel)
{
  std::cerr << "bozkurtus -- OpenBtSocket() -- call" << std::endl;
  std::cerr << "bozkurtus -- OpenBtSocket() -- before socket() call" << std::endl;
  std::cerr << "bozkurtus -- OpenBtSocket() -- AF_BLUETOOTH=" << (int)AF_BLUETOOTH << std::endl;
  std::cerr << "bozkurtus -- OpenBtSocket() -- SOCK_RAW=" << (int)SOCK_RAW << std::endl;
  std::cerr << "bozkurtus -- OpenBtSocket() -- SOCK_CLOEXEC=" << (int)SOCK_CLOEXEC << std::endl;
  std::cerr << "bozkurtus -- OpenBtSocket() -- BTPROTO_HCI=" << (int)BTPROTO_HCI << std::endl;

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

  if (bind(*out_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    std::cerr << "Failed to bind raw bluetooth socket. Fd=" << *out_fd << ". Error: " << strerror(errno) << std::endl;
    return false;
  }

  std::cerr << "bozkurtus -- OpenBtSocket() -- SO_TIMESTAMP=" << (int)SO_TIMESTAMP << std::endl;
  int opt = 1;
  if (setsockopt(*out_fd, SOL_SOCKET, SO_TIMESTAMP, &opt, sizeof(opt)) < 0)
  {
    std::cerr << "Failed to setsockopt(SO_TIMESTAMP): " << strerror(errno) << std::endl;
    close(*out_fd);
    return false;
  }

  std::cerr << "bozkurtus -- OpenBtSocket() -- SO_PASSCRED=" << (int)SO_PASSCRED << std::endl;
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

  std::cerr << "bozkurtus -- OpenBtSocket() -- end" << std::endl;
  return true;
}

bool HandleRead(int fd)
{
  std::cout << "Attempting to read from fd=" << fd << std::endl;

  struct mgmt_hdr header;
  struct msghdr msg;
  struct iovec iov[2];
  uint8_t data[1490];
  uint8_t control[64];

  iov[0].iov_base = &header;
  iov[0].iov_len = sizeof(header);
  iov[1].iov_base = data;
  iov[1].iov_len = sizeof(data);

  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = iov;
  msg.msg_iovlen = 2;
  msg.msg_control = control;
  msg.msg_controllen = sizeof(control);

  int result = recvmsg(fd, &msg, MSG_DONTWAIT);
  if (result < 0)
  {
    std::cerr << "Failed to read msg: " << strerror(errno) << std::endl;
    return false;
  }

  std::cout << "Packet dump: header.opcode=0x" << std::hex << (int)header.opcode
            << std::dec << std::endl;
  std::cout << "Packet dump: header.index=0x" << (int)header.index << std::endl;
  std::cout << "Packet dump: header.len=0x" << (int)header.len << std::endl;
 
  int packet_len = result - sizeof(header); 
  for (size_t i = 0; i < packet_len; ++i)
  {
    std::cout << "Packet dump: buffer[" << std::dec << i << std::hex << "]=0x"
              << (int)data[i] << std::endl;
  }

  return true;
}

bool AddEpollEvent(int epoll_fd, int fd, const char *name)
{
  XBoxControllerData *data = new XBoxControllerData();
  data->fd = fd;
  data->name = name;

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.ptr = data;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
  {
    std::cerr << "Failed to add epoll event" << std::endl;
    return false;
  }

  return true;
}

bool ReadXboxControllerInput(int monitor_channel_fd, int control_channel_fd)
{
  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);

  if (!AddEpollEvent(
          epoll_fd,
          monitor_channel_fd,
          "Monitor Channel"))
  {
    std::cerr << "Failed to add monitor channel event" << std::endl;
    return false;
  }

  if (!AddEpollEvent(
          epoll_fd,
          control_channel_fd,
          "Control Channel"))
  {
    std::cerr << "Failed to add control channel event" << std::endl;
    return false;
  }

  while (true)
  {
    std::cout << "Before epoll_wait()..." << std::endl;

    struct epoll_event events[2];
    int active_fds = epoll_wait(epoll_fd, events, 2, -1);

    std::cout << "After epoll_wait()..." << std::endl;

    if (active_fds < 0)
    {
      std::cerr << "EPoll active fd count less than zero. Error: " << strerror(errno)
                << std::endl;
      return false;
    }

    for (size_t i = 0; i < active_fds; ++i)
    {
      XBoxControllerData *data = (XBoxControllerData *)events[i].data.ptr;
      if (!HandleRead(data->fd))
      {
        std::cerr << "Failed to handle read for fd=" << data->fd << std::endl;
        return false;
      }
    }
  }

  close(epoll_fd);
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
