#ifndef PROCDESTRUCTOR_H
#define PROCDESTRUCTOR_H

#include <QObject>
#include <QProcess>
#include <QSet>
#include <memory>

class ProcDestructor : public QObject
{
    Q_OBJECT
public:
    explicit ProcDestructor(QObject* parent = nullptr);
    ~ProcDestructor();

    void addProc(QProcess* proc);
private slots:
    void finished(int, QProcess::ExitStatus);

private:
    QSet<QProcess*> m_procSet;
};

#endif // PROCDESTRUCTOR_H
