#include <QFont>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCursor>

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

Qt::GlobalColor ansiSGRtoQtColor(enum AnsiSGR param)
{
    switch (param) {
    case Black:
    case BlackBG:
        return Qt::black;
    case Red:
    case RedBG:
        return Qt::red;
    case Green:
    case GreenBG:
        return Qt::green;
    case Yellow:
    case YellowBG:
        return Qt::yellow;
    case Blue:
    case BlueBG:
        return Qt::blue;
    case Magenta:
    case MagentaBG:
        return Qt::magenta;
    case Cyan:
    case CyanBG:
        return Qt::cyan;
    case White:
    case WhiteBG:
        return Qt::white;
    default:
        break;
    }

    return Qt::transparent;
}

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

    proc = Process::aria2Instance();
    connect(proc, &Process::logReady, this, &Logs::log);
}

Logs::~Logs() {}

void Logs::log(const QString &msg, Process::LogLevel level)
{
    Qt::GlobalColor fgColor = Qt::transparent;
    if (msg.trimmed().isEmpty())
        return;

    switch (level) {
    case Process::Critical:
    case Process::Error:
    case Process::Fatal:
        fgColor = Qt::red;
        break;
    case Process::Warning:
        fgColor = Qt::yellow;
        break;
    case Process::Unknown:
        fgColor = Qt::gray;
    case Process::StdOut:
    case Process::StdErr:
    default:
        break;
    }

    QTextCursor cur = QTextCursor(textbox->document());
    cur.movePosition(QTextCursor::End);
    QTextCharFormat stdTextFormat;
    if (fgColor != Qt::transparent)
        stdTextFormat.setForeground(fgColor);

    QRegularExpression ansiOpen("\033\\[[\\d;]+m");
    QRegularExpression ansiClose("\033\\[0m");
    QRegularExpressionMatch matchOpen;
    int idxPointer = 0;

    bool followOutput      = false;
    QScrollBar *vScrollBar = textbox->verticalScrollBar();
    if (vScrollBar->value() == vScrollBar->maximum())
        followOutput = true;

    while (idxPointer < msg.length()) {
        QTextCharFormat ansiTextFormat;
        int startIndex = msg.indexOf(ansiOpen, idxPointer, &matchOpen);
        int endIndex   = msg.indexOf(ansiClose, startIndex);

        if (startIndex >= 0) {
            int ansiBlockLen;
            QString sgrParam = matchOpen.captured();
            sgrParam.remove(0, 2);
            sgrParam.chop(1);
            QStringList sgrParamList = sgrParam.split(';');

            for (auto i : sgrParamList) {
                auto sgrCode = static_cast<AnsiSGR>(i.toInt());
                switch (sgrCode) {
                case Bold:
                    ansiTextFormat.setFontWeight(QFont::Bold);
                    break;
                case Faint:
                    ansiTextFormat.setFontWeight(QFont::Light);
                    break;
                case Italic:
                    ansiTextFormat.setFontItalic(true);
                    break;
                case Underline:
                    ansiTextFormat.setFontUnderline(true);
                    break;
                default:
                    break;
                }

                if (sgrCode >= 30 && sgrCode <= 37) {
                    ansiTextFormat.setForeground(ansiSGRtoQtColor(sgrCode));
                } else if (sgrCode >= 40 && sgrCode <= 47) {
                    ansiTextFormat.setBackground(ansiSGRtoQtColor(sgrCode));
                }
            }

            int blockStart = startIndex + matchOpen.captured().length();
            if (endIndex >= 0)
                ansiBlockLen = endIndex - blockStart;
            else {
                ansiBlockLen = msg.length();
            }

            cur.insertText(msg.sliced(idxPointer, startIndex - idxPointer), stdTextFormat);
            cur.insertText(msg.sliced(blockStart, ansiBlockLen), ansiTextFormat);
            idxPointer = blockStart + ansiBlockLen + ansiClose.pattern().length() - 1;
        } else {
            cur.insertText(msg.sliced(idxPointer), stdTextFormat);
            idxPointer = msg.length();
        }
    }

    if (followOutput)
        vScrollBar->setValue(vScrollBar->maximum());
}

void Logs::clear()
{
    qInfo() << "Clearing logs.";
    textbox->setPlainText("");
}

} // namespace Aria2Tray
