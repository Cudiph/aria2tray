#ifndef ARIA2TRAY_PROCESS_H_
#define ARIA2TRAY_PROCESS_H_

#include <QProcess>

namespace Aria2Tray {

class Process : public QProcess {
    Q_OBJECT

public:
    static Process *instance();

    /**
     * return empty string if not found
     */
    static QString ariaExecutablePath();
    enum LogLevel {
        Unknown,
        StdOut,
        StdErr,
        Error,
        Debug,
        Info,
        Warning,
        Critical,
        Fatal,
    };

signals:
    void logReady(const QString &idk, Process::LogLevel level);

public Q_SLOTS:
    void onStdOut();
    void onStdErr();
    void onErrorOccurred(QProcess::ProcessError error);

private:
    Process(QObject *parent = nullptr);
    static inline Process *instance_ = nullptr;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_PROCESS_H_
