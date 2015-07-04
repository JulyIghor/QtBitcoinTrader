#include "sound.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <QSound>
#endif

namespace Platform {


void playSound(const QString& path)
{
#ifdef Q_OS_WIN
    PlaySound((LPCWSTR) path.utf16(), NULL, SND_ASYNC);
#else
    static QSound soundInstance("",0);
    soundInstance.stop();
    soundInstance.play(path);
#endif
}


}
