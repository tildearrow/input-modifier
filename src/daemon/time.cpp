#include "imodd.h"

// extensions to timespec

struct timespec mkts(time_t sec, long nsec) {
  struct timespec ret;
  ret.tv_sec=sec;
  ret.tv_nsec=nsec;
  return ret;
}

bool operator >(const struct timespec& l, const struct timespec& r) {
  if (l.tv_sec==r.tv_sec) {
    return l.tv_nsec>r.tv_nsec;
  }
  return l.tv_sec>r.tv_sec;
}

bool operator <(const struct timespec& l, const struct timespec& r) {
  if (l.tv_sec==r.tv_sec) {
    return l.tv_nsec<r.tv_nsec;
  }
  return l.tv_sec<r.tv_sec;
}

bool operator ==(const struct timespec& l, const struct timespec& r) {
  return (l.tv_sec==r.tv_sec && l.tv_nsec==r.tv_nsec);
}

struct timespec operator -(const struct timespec& l, const long& r) {
  struct timespec ret;
  ret=l;
  if ((ret.tv_nsec-=r)<0) {
    ret.tv_sec-=1-ret.tv_nsec/1000000000;
    ret.tv_nsec+=(1+(ret.tv_nsec/1000000000))*1000000000;
  }
  return ret;
}
