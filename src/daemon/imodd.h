#include <stdio.h>
#include <string>

typedef std::string string;

class Device {
  int fd;
  string name, path;
  int uinputfd;
  struct uinput_user_dev uiconfig;
  pthread_t* inThread;
  public:
    void run();
};
