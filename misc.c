// misc.c

#include "stdio.h"
#include "defs.h"

#include <windows.h>
#include "sys/time.h"
// #include "unistd.h"
#include "string.h"

int GetTimeMs() {
#ifdef WIN32
  return GetTickCount();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}

// int InputWaiting()
// {
//    static int init = 0, pipe;
//    static HANDLE inh;
//    DWORD dw;

//    if (!init) {
//      init = 1;
//      inh = GetStdHandle(STD_INPUT_HANDLE);
//      pipe = !GetConsoleMode(inh, &dw);
//      if (!pipe) {
//         SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
//         FlushConsoleInputBuffer(inh);
//       }
//     }
//     if (pipe) {
//       if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
//       return dw;
//     } else {
//       GetNumberOfConsoleInputEvents(inh, &dw);
//       return dw <= 1 ? 0 : dw;
// 	}
// }

// void ReadInput(S_SEARCHINFO *info) {
//   int             bytes;
//   char            input[256] = "", *endc;

//     if (InputWaiting()) {
// 		info->stopped = TRUE;
// 		do {
// 		  bytes=read(fileno(stdin),input,256);
// 		} while (bytes<0);
// 		endc = strchr(input,'\n');
// 		if (endc) *endc=0;

// 		if (strlen(input) > 0) {
// 			if (!strncmp(input, "quit", 4))    {
// 			  info->quit = TRUE;
// 			}
// 		}
// 		return;
//     }
// }


