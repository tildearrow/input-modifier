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
#include <dlfcn.h>
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

#define logLevel 2

#define IMOD_VERSION "1.3.1"

#ifdef _WIN32
#define IMOD_EXPORT __declspec(dllexport)
#else
#define IMOD_EXPORT __attribute__((visibility("default")))
#endif

int imLogD(const char* format, ...);
int imLogI(const char* format, ...);
int imLogW(const char* format, ...);
int imLogE(const char* format, ...);

bool operator ==(const struct timespec& l, const struct timespec& r);
bool operator !=(const struct timespec& l, const struct timespec& r);
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

// K: key
// R: relative
// S: switch
// M: macro
enum ActionType {
  // NAME           KRSM
  actKey=0,      // YYYY
  actTurbo,      // Y.Y.
  actRel,        // YY.Y
  actRelConst,   // Y.Y.
  actAbs,        // YY..
  actExecute,    // YYY.
  actSwitchMap,  // YYY.
  actShiftMap,   // Y.Y.
  actDisable,    // YYY.
  actMacro,      // YY..
  actWait,       // ...Y
  actMouseMove   // ...Y
};
// notes about actions:
// - actTurbo, actRelConst and actShiftMap trigger until the switch has been released (set to 0).
// - actMouseMove is a vector of relative X, relative Y and delay.
//   the sole purpose of this event and not actRel is to ease displaying it in the GUI.

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
  Action(ActionType t, struct timespec delay): type(t), timeOn(delay) {}
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

struct activeRelConst {
  struct timespec delay;
  struct timespec next;
  int code, value;
  activeRelConst(struct timespec d, int c, int v): delay(d), next(curTime(CLOCK_MONOTONIC)+d), code(c), value(v) {}
};

struct activeDelayed {
  struct timespec next;
  struct input_event ev;
};

struct Macro {
  string name;
  std::vector<Action> actions;
  Macro(string n): name(n) {}
};

struct activeMacro {
  Macro* which;
  struct timespec next;
  size_t actionIndex;
  activeMacro(Macro* w): which(w), next(curTime(CLOCK_MONOTONIC)), actionIndex(0) {}
};

// macros are global
extern std::vector<Macro*> macros;

bool loadMacros(string path);
bool saveMacros(string path);

class Device {
  int fd[8];
  int fds;
  bool isBulkDevice;
  string name, phys, path[8];
  struct input_id info;
  struct input_absinfo absinfo[ABS_CNT];
  int uinputfd;
  // we won't be switching to the next version of the
  // uinput interface until Ubuntu 14.04 stops being
  // supported...
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
  std::vector<activeRelConst> runRelConst;
  std::vector<activeMacro> runMacro;
  std::vector<activeDelayed> runDelayedEvent;
  std::bitset<KEY_CNT> pressedKeys;
  std::bitset<KEY_CNT> ignoreRecord;

  string curProfile;

  Macro* recording;
  struct timespec prevActionTime, lastActionTime;
  int recordStopKey;

  struct mapStackElement {
    bindSet* map;
    int code;
    mapStackElement(bindSet* m, int c): map(m), code(c) {}
  };

  // not a true stack
  std::vector<mapStackElement> mapStack;

  struct timespec timeout;
  public:
    bool inited, active, enabled;
    int lastKey;
    std::vector<bindSet*> mappings;
    bindSet* curmap;
    
    string getName();
    string getSaneName();
    string getCurProfile();
    void setCurProfile(string name);
    void run();
    bool activate();
    bool deactivate();
    bool init();
    void newMap(string name);
    void delMap(string name);
    void copyMap(string src, string dest);
    int findMap(string name);
    bool addPath(string p);
    int delPath(string p);
    bool recordMacro(Macro* where, int stopKey, int delay, int maxTime);
    bool stopRecord();
    bool switchProfile(string name);
    bool loadState(string path);
    bool saveState(string path);
    bool loadProfile(string path);
    bool saveProfile(string path, string dirpath);
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
      Connection(): dataThread(-1), fd(-1), size(0) {
        addr.family=AF_UNIX;
        memset(addr.path,0,PATH_MAX);
      }
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
  int fd, wd;
  std::queue<PlugEvent> events;
  pthread_t thread;
  public:
    bool init();
    bool deactivate();
    void run();
};

int scanDev(std::vector<Device*>& populate);
int processDev(std::vector<Device*>& populate, string name);

// ~~~ { [ > -> PLUGIN FUNCTIONS <- < ] } ~~~ //

enum PluginEventType {
  plugInputEvent=0,
  plugDevicePlugEvent
};

struct PluginInfo {
  // PLUGIN-WRITABLE VARIABLES //
  string name;
  string author;
  std::vector<AvailCommands> commands;
  bool listenToEvent[2];

  // INPUT-MODIFIER-WRITABLE VARIABLES //
  std::vector<Device*>* devices;
  std::vector<Macro*>* macros;
};

class LoadedPlugin {
  PluginInfo* info;
  void* handle;
  public:
    PluginInfo* (*getInfo)(void);
    bool (*init)(void);
    bool (*quit)(void);
    bool loadFile(string path);
    LoadedPlugin(): info(NULL), handle(NULL), getInfo(NULL), init(NULL), quit(NULL) {}
    ~LoadedPlugin() {
      if (handle!=NULL) dlclose(handle);
    }
};
