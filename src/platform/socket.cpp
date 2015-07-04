#include "socket.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif


namespace Platform {

void setSocketTimeout(qintptr socket, int timeout)
{
#ifdef Q_OS_WIN
        setsockopt((SOCKET) socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(int));
#else
        struct timeval vtime;
        vtime.tv_sec = timeout / 1000;
        vtime.tv_usec = timeout * 1000 - vtime.tv_sec * 1000000;
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &vtime, sizeof(struct timeval));
#endif
}

}
