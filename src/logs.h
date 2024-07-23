#ifndef ARIA2TRAY_LOGS_H_
#define ARIA2TRAY_LOGS_H_

#include <QGridLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "process.h"

namespace Aria2Tray {

enum AnsiSGR {
    Reset = 0,

    // Font styling
    Bold,
    Faint,
    Italic,
    Underline,

    // Foreground color
    Black = 30,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,

    // Background
    BlackBG = 40,
    RedBG,
    GreenBG,
    YellowBG,
    BlueBG,
    MagentaBG,
    CyanBG,
    WhiteBG,
};

Qt::GlobalColor ansiSGRtoQtColor(enum AnsiSGR param);

class Logs : public QWidget {
    Q_OBJECT

public:
    Logs(QWidget *parent = nullptr);
    virtual ~Logs();

public Q_SLOTS:
    void log(const QString &msg, Process::LogLevel level);
    void clear();

private:
    QVBoxLayout *rootLayout;
    QPlainTextEdit *textbox;
    QPushButton *clearButton;
    QGridLayout *buttonGridLayout;

    Process *proc;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_LOGS_H_
