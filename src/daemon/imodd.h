#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <bitset>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <signal.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define DEVICE_DIR "/dev/input"

typedef std::string string;

#define S(x) string(x)

#define LOGLEVEL_ERROR 0
#define LOGLEVEL_WARN 1
#define LOGLEVEL_INFO 2
#define LOGLEVEL_DEBUG 3

#define logLevel 3

int imLogD(const char* format, ...);
int imLogI(const char* format, ...);
int imLogW(const char* format, ...);
int imLogE(const char* format, ...);

// 0: no weird devices
// 1: allow weird devices
// 2: allow super weird devices
#define allowWeird 0

enum ActionType {
  actKey=0,
  actTurbo,
  actRel,
  actRelConst,
  actAbs,
  actExecute,
  actSwitchMap,
  actShiftMap
};

struct Action {
  ActionType type;
  int code;
  int value;
  struct timespec timeOn, timeOff;
  string command;
  std::vector<string> args;
  std::vector<string> env;
};

struct eventBind {
  bool doModify;
  std::vector<Action> actions;
};

class Device {
  int fd;
  string name, phys, path;
  struct input_id info;
  int uinputfd;
  struct uinput_user_dev uiconfig;
  bool threadRunning;
  pthread_t inThread;
  bool active;
  std::bitset<EV_CNT> evcaps;
  std::bitset<SYN_CNT> syncaps;
  std::bitset<KEY_CNT> keycaps;
  std::bitset<REL_CNT> relcaps;
  std::bitset<ABS_CNT> abscaps;
  std::bitset<MSC_CNT> msccaps;
  std::bitset<SW_CNT> swcaps;
  std::bitset<LED_CNT> ledcaps;
  std::bitset<REP_CNT> repcaps;
  std::bitset<SND_CNT> sndcaps;

  eventBind keybinds[KEY_CNT];
  public:
    void run();
    bool activate();
    bool init();
    Device(string p);
    Device();
};

int scanDev(std::vector<Device*>& populate);
