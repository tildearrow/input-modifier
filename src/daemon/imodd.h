#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdexcept>
#include <string>
#include <bitset>
#include <vector>
#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <grp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/inotify.h>
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

#define IMOD_VERSION "1.0.1"

int imLogD(const char* format, ...);
int imLogI(const char* format, ...);
int imLogW(const char* format, ...);
int imLogE(const char* format, ...);

bool operator ==(const struct timespec& l, const struct timespec& r);
bool operator >(const struct timespec& l, const struct timespec& r);
bool operator <(const struct timespec& l, const struct timespec& r);
struct timespec operator +(const struct timespec& l, const struct timespec& r);
struct timespec operator -(const struct timespec& l, const struct timespec& r);
struct timespec operator +(const struct timespec& l, const long& r);
struct timespec operator -(const struct timespec& l, const long& r);
struct timespec mkts(time_t sec, long nsec);
struct timespec stots(string s);
string tstos(struct timespec ts);
struct timespec curTime(clockid_t clockSource);

extern "C" {
#include "names.h"
};

void childHandler(int data);

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
  actShiftMap,
  actDisable
};

#define Command(x) int x(int output, std::vector<string>* args)

struct AvailCommands {
  const char* n;
  int (*c)(int,std::vector<string>*);
};

extern const AvailCommands cmds[];

struct Action {
  ActionType type;
  int code;
  int value;
  struct timespec timeOn, timeOff;
  string command;
  std::vector<string> args;
  std::vector<string> env;
  Action(ActionType t, int c): type(t), code(c) {}
  Action(ActionType t, int c, struct timespec on, struct timespec off): type(t), code(c), timeOn(on), timeOff(off) {}
  Action(ActionType t, int c, int v): type(t), code(c), value(v) {}
  Action(ActionType t, int c, int v, struct timespec ti): type(t), code(c), value(v), timeOn(ti) {}
  Action(ActionType t, string prog, std::vector<string>& a, std::vector<string>& e): type(t), command(prog), args(a), env(e) {}
  Action(ActionType t, string name): type(t), command(name) {}
};

struct eventBind {
  bool doModify;
  bool advanced;
  // actions becomes pressActions if advanced
  std::vector<Action> actions;
  std::vector<Action> relactions;
  eventBind():
    doModify(false), advanced(false) {}
};

struct bindSet {
  string name;
  eventBind keybinds[KEY_CNT];
  eventBind relbinds[REL_CNT];
  eventBind swbinds[SW_CNT];
  bindSet(string n): name(n) {}
};

struct activeTurbo {
  bool phase;
  struct timespec timeOn;
  struct timespec timeOff;
  struct timespec next;
  int code;
  activeTurbo(struct timespec on, struct timespec off, struct timespec n, int c): phase(true), timeOn(on), timeOff(off), next(n), code(c) {}
};

class Device {
  int fd[8];
  int fds;
  bool isBulkDevice;
  string name, phys, path[8];
  struct input_id info;
  struct input_absinfo absinfo[ABS_CNT];
  int uinputfd;
  struct uinput_user_dev uiconfig;
  bool threadRunning;
  pthread_t inThread;
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

  std::vector<activeTurbo> runTurbo;
  std::bitset<KEY_CNT> pressedKeys;

  struct mapStackElement {
    bindSet* map;
    int code;
    mapStackElement(bindSet* m, int c): map(m), code(c) {}
  };

  // not a true stack
  std::vector<mapStackElement> mapStack;

  struct timespec timeout;
  public:
    bool active, enabled;
    std::vector<bindSet*> mappings;
    bindSet* curmap;
    
    string getName();
    string getSaneName();
    void run();
    bool activate();
    bool deactivate();
    bool init();
    void newMap(string name);
    void delMap(string name);
    void copyMap(string src, string dest);
    int findMap(string name);
    bool addPath(string p);
    bool loadState(string path);
    bool saveState(string path);
    Device(string n, string p);
    Device();
};

class SocketInterface {
  struct {
    sa_family_t family;
    char path[PATH_MAX];
  } addr;
  int fd;
  public:
    struct Connection {
      struct {
        sa_family_t family;
        char path[PATH_MAX];
      } addr;
      pthread_t dataThread;
      int fd;
      socklen_t size;
    };
  private:
  std::vector<Connection> conns;

  pthread_t listenThread;
  public:
    bool init();
    bool activate();
    void run();
};

enum PlugEventType {
  evPlug,
  evUnplug
};

struct PlugEvent {
  PlugEventType type;
  string path;
};

class DeviceListener {
  int fd;
  std::queue<PlugEvent> events;
  public:
    bool init();
};

int scanDev(std::vector<Device*>& populate);
