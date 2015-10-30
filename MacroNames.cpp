
// MacroNames.cpp
// Copyright (C) 2007,2015 Matthew Rickard

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

const char *ErrnoToMacroName(int errorCode)
{
  if (errorCode == 1)
    return "EPERM";
  if (errorCode == 2)
    return "ENOFILE";
  if (errorCode == 2)
    return "ENOENT";
  if (errorCode == 3)
    return "ESRCH";
  if (errorCode == 4)
    return "EINTR";
  if (errorCode == 5)
    return "EIO";
  if (errorCode == 6)
    return "ENXIO";
  if (errorCode == 7)
    return "E2BIG";
  if (errorCode == 8)
    return "ENOEXEC";
  if (errorCode == 9)
    return "EBADF";
  if (errorCode == 10)
    return "ECHILD";
  if (errorCode == 11)
    return "EAGAIN";
  if (errorCode == 12)
    return "ENOMEM";
  if (errorCode == 13)
    return "EACCES";
  if (errorCode == 14)
    return "EFAULT";
  if (errorCode == 16)
    return "EBUSY";
  if (errorCode == 17)
    return "EEXIST";
  if (errorCode == 18)
    return "EXDEV";
  if (errorCode == 19)
    return "ENODEV";
  if (errorCode == 20)
    return "ENOTDIR";
  if (errorCode == 21)
    return "EISDIR";
  if (errorCode == 22)
    return "EINVAL";
  if (errorCode == 23)
    return "ENFILE";
  if (errorCode == 24)
    return "EMFILE";
  if (errorCode == 25)
    return "ENOTTY";
  if (errorCode == 27)
    return "EFBIG";
  if (errorCode == 28)
    return "ENOSPC";
  if (errorCode == 29)
    return "ESPIPE";
  if (errorCode == 30)
    return "EROFS";
  if (errorCode == 31)
    return "EMLINK";
  if (errorCode == 32)
    return "EPIPE";
  if (errorCode == 33)
    return "EDOM";
  if (errorCode == 34)
    return "ERANGE";
  if (errorCode == 36)
    return "EDEADLOCK";
  if (errorCode == 36)
    return "EDEADLK";
  if (errorCode == 38)
    return "ENAMETOOLONG";
  if (errorCode == 39)
    return "ENOLCK";
  if (errorCode == 40)
    return "ENOSYS";
  if (errorCode == 41)
    return "ENOTEMPTY";
  if (errorCode == 42)
    return "EILSEQ";
  return "(Unknown errno)";
}

#define nameSignal(a) { a, #a }

struct SigName {
  int number;
  const char *name;
} sigNames[] = {
  #ifdef     SIGHUP
  nameSignal(SIGHUP),
  #endif
  #ifdef     SIGINT
  nameSignal(SIGINT),
  #endif
  #ifdef     SIGQUIT
  nameSignal(SIGQUIT),
  #endif
  #ifdef     SIGILL
  nameSignal(SIGILL),
  #endif
  #ifdef     SIGABRT
  nameSignal(SIGABRT),
  #endif
  #ifdef     SIGFPE
  nameSignal(SIGFPE),
  #endif
  #ifdef     SIGKILL
  nameSignal(SIGKILL),
  #endif
  #ifdef     SIGSEGV
  nameSignal(SIGSEGV),
  #endif
  #ifdef     SIGPIPE
  nameSignal(SIGPIPE),
  #endif
  #ifdef     SIGALRM
  nameSignal(SIGALRM),
  #endif
  #ifdef     SIGTERM
  nameSignal(SIGTERM),
  #endif
  #ifdef     SIGUSR1
  nameSignal(SIGUSR1),
  #endif
  #ifdef     SIGUSR2
  nameSignal(SIGUSR2),
  #endif
  #ifdef     SIGCHLD
  nameSignal(SIGCHLD),
  #endif
  #ifdef     SIGCONT
  nameSignal(SIGCONT),
  #endif
  #ifdef     SIGSTOP
  nameSignal(SIGSTOP),
  #endif
  #ifdef     SIGTSTP
  nameSignal(SIGTSTP),
  #endif
  #ifdef     SIGTTIN
  nameSignal(SIGTTIN),
  #endif
  #ifdef     SIGTTOU
  nameSignal(SIGTTOU),
  #endif
  #ifdef     SIGBUS
  nameSignal(SIGBUS),
  #endif
  #ifdef     SIGPOLL
  nameSignal(SIGPOLL),
  #endif
  #ifdef     SIGPROF
  nameSignal(SIGPROF),
  #endif
  #ifdef     SIGSYS
  nameSignal(SIGSYS),
  #endif
  #ifdef     SIGTRAP
  nameSignal(SIGTRAP),
  #endif
  #ifdef     SIGURG
  nameSignal(SIGURG),
  #endif
  #ifdef     SIGVTALRM
  nameSignal(SIGVTALRM),
  #endif
  #ifdef     SIGXCPU
  nameSignal(SIGXCPU),
  #endif
  #ifdef     SIGXFSZ
  nameSignal(SIGXFSZ),
  #endif
  #ifdef     SIGIOT
  nameSignal(SIGIOT),
  #endif
  #ifdef     SIGEMT
  nameSignal(SIGEMT),
  #endif
  #ifdef     SIGSTLFLT
  nameSignal(SIGSTLFLT),
  #endif
  #ifdef     SIGIO
  nameSignal(SIGIO),
  #endif
  #ifdef     SIGCLD
  nameSignal(SIGCLD),
  #endif
  #ifdef     SIGPWR
  nameSignal(SIGPWR),
  #endif
  #ifdef     SIGINFO
  nameSignal(SIGINFO),
  #endif
  #ifdef     SIGLOST
  nameSignal(SIGLOST),
  #endif
  #ifdef     SIGWINCH
  nameSignal(SIGWINCH),
  #endif
  #ifdef     SIGUNUSED
  nameSignal(SIGUNUSED),
  #endif
};

const char *SignalToMacroName(unsigned int sig) {
  for (unsigned i = 0; i < sizeof(sigNames) / sizeof(SigName); i++)
    if (sigNames[i].number == sig)
      return sigNames[i].name;
  return "unknown";
}

