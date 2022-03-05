#include "procdestructor.h"

ProcDestructor::ProcDestructor(QObject* parent) : QObject(parent)
{
}

ProcDestructor::~ProcDestructor()
{
    for (QProcess* proc : m_procSet)
    {
        proc->blockSignals(true);
        if (proc->state() == QProcess::Running)
        {
            proc->terminate();
            if (!proc->waitForFinished(3000))
                proc->kill();
        }
        delete proc;
    }
}

void ProcDestructor::addProc(QProcess* proc)
{
    if (proc)
    {
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcDestructor::finished, Qt::QueuedConnection);
        m_procSet.insert(proc);
    }
}

void ProcDestructor::finished(int, QProcess::ExitStatus)
{
    if (QProcess* senderProc = qobject_cast<QProcess*>(sender()))
        if (m_procSet.remove(senderProc))
            delete senderProc;
}
