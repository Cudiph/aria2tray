#ifndef ARIA2TRAY_OPTIONS_H_
#define ARIA2TRAY_OPTIONS_H_

#include <QCheckBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>
#include <QValidator>
#include <QWidget>

#include "logs.h"
#include "process.h"
#include "qpushbutton.h"

namespace Aria2Tray {

class CmdArgsBuilder : public QWidget {
    Q_OBJECT

public:
    typedef struct {
        QWidget *container;
        QLineEdit *key;
        QLineEdit *value;
    } kvContainer_t;
    CmdArgsBuilder(QWidget *parent = nullptr);
    kvContainer_t *appendKVEdit();
    kvContainer_t *appendKVEdit(QString key, QString val);
    void removeAt(qsizetype i);
    void remove(kvContainer_t *container);
    void clear();
    QStringList buildArgs();
    void loadArgs(QStringList args);

signals:
    void contentChanged(const QString &textChanged);
    void editingFinished();
    void entryRemoved();

private Q_SLOTS:
    void onItemTextChanged(const QString &textChanged);
    void onEditingFinished();

private:
    void deleteEmpty();

    QVBoxLayout *rootLayout;
    QList<kvContainer_t *> kvContainerList;
};

class Options : public QWidget {
    Q_OBJECT

public:
    Options(QWidget *parent = nullptr);
    virtual ~Options();

    Logs *logsWidget;

    QStringList buildArgs();
    void start();
    void stop();
    void stopWait(int msecs = 30000);
    void kill();
    void killOthersAria2();

signals:
    void optionsChanged();

private Q_SLOTS:
    void onStateChange(QProcess::ProcessState state);
    void onStartupChange(Qt::CheckState state);
    void onAdvanceUserChange(Qt::CheckState state);
    void onCertPathClick();
    void onSaveFolderClick();

    void toggleCertPath(Qt::CheckState state);
    void addNewArgs();

private:
    QGroupBox *rpcLayout();
    QGroupBox *miscLayout();
    QGroupBox *cmdArgsLayout();
    QGridLayout *actionButtonsLayout();

    void resetDefault();
    void loadConfig();
    void saveConfig();

    void addStartup();
    void removeStartup();

    QVBoxLayout *rootLayout;
    QScrollArea *scrollArea;

    QGroupBox *rpcGroupBox;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QLabel *RPCSecretLabel;
    QLineEdit *RPCSecretEdit;
    QCheckBox *expose;
    QCheckBox *useIpv6;
    QCheckBox *secure;
    QWidget *certPathLayoutRoot;
    QLabel *certPathLabel;
    QLineEdit *certPathEdit;
    QPushButton *certPathButton;

    QGroupBox *miscGroupBox;
    QWidget *saveFolderLayoutRoot;
    QLabel *saveFolderLabel;
    QLineEdit *saveFolderEdit;
    QPushButton *saveFolderButton;
    QCheckBox *certCheck;
    QCheckBox *runOnStartup;
    QCheckBox *advancedUser;

    QGroupBox *cmdArgsGroupBox;
    CmdArgsBuilder *cmdArgsBuilderWidget;
    QPushButton *cmdArgsAddButton;

    QGridLayout *actionGridLayout;
    QPushButton *resetButton;
    QPushButton *startButton;
    QPushButton *forceStopButton;

    Process *proc;
};

class QRPCSecretValidator : public QValidator {
    Q_OBJECT

public:
    QRPCSecretValidator(QObject *parent = nullptr);
    virtual QValidator::State validate(QString &input, int &pos) const override;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_OPTIONS_H_
