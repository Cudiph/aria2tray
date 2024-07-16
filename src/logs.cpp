#include <QFont>

#include "logs.h"
#include "process.h"

namespace Aria2Tray {

int defaultSize = 10;
#ifdef Q_OS_WIN32
QString defaultMonospace = "Courier New";
#elif defined(Q_OS_MAC)
QString defaultMonospace = "Menlo";
#else
QString defaultMonospace = "Monospace";
#endif // Q_OS_WIN32

Logs::Logs(QWidget *parent) : QWidget(parent)
{
    rootLayout = new QVBoxLayout(this);

    auto logsFont = QFont(defaultMonospace);
    logsFont.setFixedPitch(true);
    logsFont.setStyleHint(QFont::Monospace);
    logsFont.setPointSize(defaultSize);

    textbox = new QPlainTextEdit(this);
    textbox->setReadOnly(true);
    textbox->setFont(logsFont);
    textbox->setPlaceholderText(tr("Logs is empty."));

    buttonGridLayout = new QGridLayout;
    clearButton      = new QPushButton(tr("Clear"), this);
    buttonGridLayout->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, &Logs::clear);

    rootLayout->addWidget(textbox);
    rootLayout->addLayout(buttonGridLayout);

    proc = Process::instance();

    connect(proc, &Process::logReady, this, &Logs::log);
}

Logs::~Logs() {}

void Logs::log(const QString &msg, Process::LogLevel level)
{
    // TODO: set color and address ansi escape code
    switch (level) {
    case Process::StdOut:
        textbox->appendPlainText(msg);
        break;
    case Process::StdErr:
        textbox->appendPlainText(msg);
        break;
    case Process::Unknown:
        textbox->appendPlainText(msg);
        break;
    case Process::Fatal:
        textbox->appendPlainText(msg);
        break;
    default:
        textbox->appendPlainText(msg);
        break;
    }
}

void Logs::clear()
{
    qInfo() << "Clearing logs.";
    textbox->setPlainText("");
}

} // namespace Aria2Tray
