#include <QApplication>
#include <QFileInfo>
#include <QStandardPaths>

#include "process.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

Process::Process(QObject *parent) : QProcess(parent)
{
    connect(this, &Process::readyReadStandardOutput, this, &Process::onStdOut);
    connect(this, &Process::readyReadStandardError, this, &Process::onStdErr);
    connect(this, &Process::errorOccurred, this, &Process::onErrorOccurred);
}

Process *Process::instance()
{
    if (instance_ == nullptr)
        instance_ = new Process();
    return instance_;
}

QString Process::ariaExecutablePath()
{
    QString file = QApplication::applicationDirPath() % "/aria2c";
    auto exist   = QFileInfo::exists(file);
    if (exist) {
        return file;
    }

    file += ".exe";
    if (QFileInfo::exists(file)) {
        return file;
    }

    return QStandardPaths::findExecutable(u"aria2c"_s);
}

void Process::onStdOut() { emit logReady(readAllStandardOutput(), StdOut); }

void Process::onStdErr() { emit logReady(readAllStandardError(), StdErr); }

void Process::onErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        emit logReady(errorString(), Fatal);
        break;
    case QProcess::UnknownError:
        emit logReady(errorString(), Unknown);
        break;
    default:
        emit logReady(errorString(), Fatal);
        break;
    }
}

} // namespace Aria2Tray
