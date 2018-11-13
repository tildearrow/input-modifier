#include <linux/input-event-codes.h>

extern const char* evnames[EV_CNT];
extern const char* keynames[KEY_CNT];
extern const char* relnames[REL_CNT];
extern const char* absnames[ABS_CNT];
extern const char* swnames[SW_CNT];

int findEv(const char* name);
int findKey(const char* name);
int findRel(const char* name);
int findAbs(const char* name);
int findSwitch(const char* name);
