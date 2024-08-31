#include <QDir>
#include <QFile>
#include <QIntValidator>
#include <QProcessEnvironment>

#include "options.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

const QString DEFAULT_PORT        = u"6800"_s;
const QString DEFAULT_RPC_SECRET  = "";
const bool DEFAULT_EXPOSE         = false;
const bool DEFAULT_USE_IPV6       = false;
const bool DEFAULT_SECURE         = false;
const QString DEFAULT_CERT_PATH   = "";
const QString DEFAULT_SAVE_FOLDER = "";
const bool DEFAULT_CERT_CHECK     = false;
const bool DEFAULT_RUN_ON_STARTUP = false;
const bool DEFAULT_ADVANCED_USER  = false;
const QStringList DEFAULT_CMD_ARGS =
    QStringList{"--max-connection-per-server", "16", "-s", "16", "--min-split-size", "1M"};

Options::Options(QWidget *parent) : QWidget(parent)
{
    rootLayout        = new QVBoxLayout(this);
    scrollArea        = new QScrollArea(this);
    auto scrollWidget = new QWidget(scrollArea);
    auto scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->addWidget(rpcLayout());
    scrollLayout->addWidget(miscLayout());
    scrollLayout->addWidget(cmdArgsLayout());
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    rootLayout->addWidget(scrollArea);
    rootLayout->addLayout(actionButtonsLayout());

    proc = Process::aria2Instance();
    connect(proc, &QProcess::stateChanged, this, &Options::onStateChange);

    loadConfig();
}

Options::~Options() {}

QGroupBox *Options::rpcLayout()
{
    rpcGroupBox  = new QGroupBox(tr("RPC Connection"), this);
    auto vLayout = new QVBoxLayout;

    auto portLayout = new QHBoxLayout;
    portLabel       = new QLabel(tr("Listening port:"), this);
    portEdit        = new QLineEdit(u"6800"_s, this);
    portEdit->setValidator(new QIntValidator(1024, 65535));
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portEdit);

    auto RPCSecretLayout = new QHBoxLayout;
    RPCSecretLabel       = new QLabel(tr("RPC Secret:"), this);
    RPCSecretEdit        = new QLineEdit(DEFAULT_RPC_SECRET, this);
    RPCSecretLabel->setToolTip(tr("Also used for integration secret"));
    RPCSecretEdit->setEchoMode(QLineEdit::Password);
    RPCSecretEdit->setValidator(new QRPCSecretValidator(this));
    RPCSecretLayout->addWidget(RPCSecretLabel);
    RPCSecretLayout->addWidget(RPCSecretEdit);

    expose  = new QCheckBox(tr("Expose to network"), this);
    useIpv6 = new QCheckBox(tr("Enable IPv6"), this);
    secure  = new QCheckBox(tr("Encrypt RPC traffic"), this);
    connect(secure, &QCheckBox::checkStateChanged, this, &Options::toggleCertPath);

    certPathLayoutRoot = new QWidget(this);
    certPathLayoutRoot->hide();
    auto certPathLayout = new QHBoxLayout(certPathLayoutRoot);
    certPathLabel       = new QLabel(tr("Certificate:"), this);
    certPathEdit        = new QLineEdit("", this);
    certPathButton      = new QPushButton(tr("Browse..."), this);
    certPathLayout->addWidget(certPathLabel);
    certPathLayout->addWidget(certPathEdit);
    certPathLayout->addWidget(certPathButton);
    connect(certPathButton, &QPushButton::clicked, this, &Options::onCertPathClick);

    vLayout->setAlignment(Qt::AlignTop);
    vLayout->addLayout(portLayout);
    vLayout->addLayout(RPCSecretLayout);
    vLayout->addWidget(expose);
    vLayout->addWidget(useIpv6);
    vLayout->addWidget(secure);
    vLayout->addWidget(certPathLayoutRoot);
    rpcGroupBox->setLayout(vLayout);

    connect(portEdit, &QLineEdit::textChanged, this, &Options::saveConfig);
    connect(RPCSecretEdit, &QLineEdit::textChanged, this, &Options::saveConfig);
    connect(expose, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(useIpv6, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(secure, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(certPathEdit, &QLineEdit::editingFinished, this, &Options::saveConfig);

    return rpcGroupBox;
}

QGroupBox *Options::miscLayout()
{
    miscGroupBox = new QGroupBox(tr("Miscellaneous"), this);
    auto vLayout = new QVBoxLayout;

    saveFolderLayoutRoot  = new QWidget(this);
    auto saveFolderLayout = new QHBoxLayout(saveFolderLayoutRoot);
    saveFolderLabel       = new QLabel(tr("Default save folder"), this);
    saveFolderEdit        = new QLineEdit(this);
    saveFolderButton      = new QPushButton(tr("Browse..."));
    saveFolderLayout->addWidget(saveFolderLabel);
    saveFolderLayout->addWidget(saveFolderEdit);
    saveFolderLayout->addWidget(saveFolderButton);
    connect(saveFolderButton, &QPushButton::clicked, this, &Options::onSaveFolderClick);

    certCheck    = new QCheckBox(tr("Enable ssl certificate check"), this);
    runOnStartup = new QCheckBox(tr("Start service on startup"), this);
    advancedUser = new QCheckBox(tr("I am an advanced user"), this);

    vLayout->setAlignment(Qt::AlignTop);
    vLayout->addWidget(saveFolderLayoutRoot);
    vLayout->addWidget(certCheck);
    vLayout->addWidget(runOnStartup);
    vLayout->addWidget(advancedUser);
    miscGroupBox->setLayout(vLayout);

    connect(saveFolderEdit, &QLineEdit::editingFinished, this, &Options::saveConfig);
    connect(certCheck, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(runOnStartup, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(advancedUser, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(runOnStartup, &QCheckBox::checkStateChanged, this, &Options::onStartupChange);
    connect(advancedUser, &QCheckBox::checkStateChanged, this, &Options::onAdvanceUserChange);

    return miscGroupBox;
}

QGroupBox *Options::cmdArgsLayout()
{
    cmdArgsGroupBox = new QGroupBox(tr("Additional Options"), this);
    auto vLayout    = new QVBoxLayout;

    cmdArgsBuilderWidget = new CmdArgsBuilder(this);
    cmdArgsAddButton     = new QPushButton(tr("+"), this);

    vLayout->addWidget(cmdArgsBuilderWidget);
    vLayout->addWidget(cmdArgsAddButton);
    cmdArgsGroupBox->setLayout(vLayout);

    connect(cmdArgsBuilderWidget, &CmdArgsBuilder::editingFinished, this, &Options::saveConfig);
    connect(cmdArgsAddButton, &QPushButton::clicked, this, &Options::addNewArgs);

    cmdArgsGroupBox->hide();

    return cmdArgsGroupBox;
}

QGridLayout *Options::actionButtonsLayout()
{
    actionGridLayout = new QGridLayout;
    resetButton      = new QPushButton(tr("Reset to default"), this);
    forceStopButton  = new QPushButton(tr("Force stop"), this);
    startButton      = new QPushButton(tr("START"), this);

    forceStopButton->setToolTip(tr("Will also kill all aria2c process"));

    connect(resetButton, &QPushButton::clicked, this, &Options::resetDefault);
    connect(forceStopButton, &QPushButton::clicked, this, &Options::kill);
    connect(forceStopButton, &QPushButton::clicked, this, &Options::killOthersAria2);
    connect(startButton, &QPushButton::clicked, this, &Options::start);

    actionGridLayout->addWidget(resetButton, 0, 0);
    actionGridLayout->addWidget(forceStopButton, 0, 1);
    actionGridLayout->addWidget(startButton, 1, 0, 1, 2);

    return actionGridLayout;
}

void Options::resetDefault()
{
    portEdit->setText(DEFAULT_PORT);
    expose->setChecked(DEFAULT_EXPOSE);
    useIpv6->setChecked(DEFAULT_USE_IPV6);
    secure->setChecked(DEFAULT_SECURE);
    certPathEdit->setText(DEFAULT_CERT_PATH);
    certCheck->setChecked(DEFAULT_CERT_CHECK);
    runOnStartup->setChecked(DEFAULT_RUN_ON_STARTUP);
    cmdArgsBuilderWidget->loadArgs(DEFAULT_CMD_ARGS);
}

void Options::start()
{
    if (proc->state() == Process::Running)
        return;
    QString programPath = Process::ariaExecutablePath();
    if (programPath.isEmpty()) {
        qDebug() << "aria2c not found.";
        logsWidget->log("Can't find aria2c.", Process::Critical);
        return;
    }
    proc->setProgram(programPath);
    QStringList args = buildArgs();
    proc->setArguments(args);
    qDebug() << "Starting aria2 with arguments:" << args;
    proc->start();
    qDebug() << "PID: " << proc->processId();
}

void Options::stop()
{
    if (proc->state() == Process::NotRunning)
        return;
    startButton->setText(tr("Stopping..."));
    startButton->setDisabled(true);
    qDebug() << "Stopping aria2...";
#ifdef Q_OS_WIN32
    proc->kill();
#else
    proc->terminate();
#endif // Q_OS_WIN32
}

void Options::stopWait(int msecs)
{
    proc->waitForFinished(msecs);
}

void Options::kill()
{
    if (proc->state() == Process::NotRunning)
        return;
    startButton->setText(tr("Force stopping..."));
    startButton->setDisabled(true);
    qDebug() << "Force stopping aria2...";
    proc->kill();
}

void Options::killOthersAria2()
{
#ifdef Q_OS_WIN32
    QProcess::execute("taskkill", QStringList() << "/F"
                                                << "/IM"
                                                << "aria2c.exe");
#else
    QProcess::execute("pkill", QStringList() << "aria2c");
#endif // Q_OS_WIN32
}

void Options::onStateChange(QProcess::ProcessState state)
{
    disconnect(startButton, &QPushButton::clicked, this, &Options::stop);
    disconnect(startButton, &QPushButton::clicked, this, &Options::start);
    switch (state) {
    case QProcess::NotRunning:
        startButton->setEnabled(true);
        startButton->setText(tr("START"));
        connect(startButton, &QPushButton::clicked, this, &Options::start);
        break;
    case QProcess::Starting:
        startButton->setDisabled(true);
        startButton->setText(tr("Starting..."));
        break;
    case QProcess::Running:
        startButton->setEnabled(true);
        startButton->setText(tr("STOP"));
        connect(startButton, &QPushButton::clicked, this, &Options::stop);
        break;
    }
}

void Options::onStartupChange(Qt::CheckState state)
{
    switch (state) {
    case Qt::Checked:
        addStartup();
        break;
    case Qt::Unchecked:
    default:
        removeStartup();
        break;
    }
}

void Options::onAdvanceUserChange(Qt::CheckState state)
{
    switch (state) {
    case Qt::Checked:
        cmdArgsGroupBox->show();
        break;
    case Qt::Unchecked:
        cmdArgsGroupBox->hide();
    default:
        break;
    }
}

#ifdef Q_OS_WIN32
const QString REG_KEY = u"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"_s;

void Options::addStartup()
{
    QSettings settings(REG_KEY, QSettings::NativeFormat);
    settings.setValue(QCoreApplication::applicationName(),
                      '"' + QCoreApplication::applicationFilePath().replace('/', '\\')
                          + "\" --hide-window");
    start();
}

void Options::removeStartup()
{
    QSettings settings(REG_KEY, QSettings::NativeFormat);
    settings.remove(QCoreApplication::applicationName());
}

#elif defined(Q_OS_LINUX)

void Options::addStartup()
{
    QString home             = QDir::homePath();
    QString userAutostartDir = QProcessEnvironment::systemEnvironment().value(
        u"XDG_CONFIG_HOME"_s, home + u"/.config/autostart"_s);

    auto entry           = QFile(u"/usr/share/applications/aria2tray.desktop"_s);
    auto autostart_entry = userAutostartDir + u"/aria2tray.desktop"_s;

    if (!entry.exists())
        goto end_start;

    if (QFile(autostart_entry).exists())
        goto end_start;

    if (entry.link(autostart_entry)) {
        qDebug() << "link created at" << userAutostartDir;
    } else {
        qCritical() << "failed to create link at" << userAutostartDir;
    }

end_start:
    start();
}

void Options::removeStartup()
{
    QString home      = QDir::homePath();
    QString entryPath = QProcessEnvironment::systemEnvironment().value(
                            u"XDG_CONFIG_HOME"_s, home + u"/.config/autostart"_s)
                        + u"/aria2tray.desktop"_s;
    auto entry = QFile(entryPath);
    if (!entry.exists())
        return;

    if (entry.remove()) {
        qDebug() << "Removed desktop entry symlink at" << entryPath;
    } else {
        qCritical() << "Failed to remove symlink at" << entryPath;
    }
}

#else
void Options::addStartup() {}
void Options::removeStartup() {}
#endif // Q_OS_WIN32

void Options::onCertPathClick()
{
    auto path =
        QFileDialog::getOpenFileName(this, tr("Pick pkcs12 certificate"), certPathEdit->text(),
                                     tr("PKCS12 certificate (*.p12 *.pfx)"));
    certPathEdit->setText(path);
    saveConfig();
}

void Options::onSaveFolderClick()
{
    auto path =
        QFileDialog::getExistingDirectory(this, tr("Pick directory"), saveFolderEdit->text());

    saveFolderEdit->setText(path);
    saveConfig();
}

void Options::toggleCertPath(Qt::CheckState state)
{
    switch (state) {
    case Qt::Checked:
        certPathLayoutRoot->show();
        break;
    case Qt::Unchecked:
        certPathLayoutRoot->hide();
    default:
        break;
    }
}

void Options::addNewArgs()
{
    cmdArgsBuilderWidget->appendKVEdit();
}

void Options::loadConfig()
{
    QSettings settings;
    auto savedPort         = settings.value(u"port"_s, DEFAULT_PORT).toString();
    auto savedRPCSecret    = settings.value(u"RPCSecret"_s, DEFAULT_RPC_SECRET).toString();
    auto savedExpose       = settings.value(u"expose"_s, DEFAULT_EXPOSE).toBool();
    auto savedUseIpv6      = settings.value(u"ipv6"_s, DEFAULT_USE_IPV6).toBool();
    auto savedSecure       = settings.value(u"secure"_s, DEFAULT_SECURE).toBool();
    auto savedSaveFolder   = settings.value(u"saveFolder"_s, DEFAULT_SAVE_FOLDER).toString();
    auto savedCertPath     = settings.value(u"pkcs12Path"_s, DEFAULT_CERT_PATH).toString();
    auto savedCertCheck    = settings.value(u"certCheck"_s, DEFAULT_CERT_CHECK).toBool();
    auto savedRunOnStartup = settings.value(u"runOnStartup"_s, DEFAULT_RUN_ON_STARTUP).toBool();
    auto savedAdvancedUser = settings.value(u"advancedUser"_s, DEFAULT_ADVANCED_USER).toBool();
    auto savedCmdArgs      = settings.value(u"aria2Arguments"_s, DEFAULT_CMD_ARGS).toStringList();

    qDebug() << "Loading configuration from" << settings.fileName();

    portEdit->setText(savedPort);
    RPCSecretEdit->setText(savedRPCSecret);
    expose->setChecked(savedExpose);
    useIpv6->setChecked(savedUseIpv6);
    secure->setChecked(savedSecure);
    saveFolderEdit->setText(savedSaveFolder);
    certPathEdit->setText(savedCertPath);
    certCheck->setChecked(savedCertCheck);
    runOnStartup->setChecked(savedRunOnStartup);
    advancedUser->setChecked(savedAdvancedUser);
    cmdArgsBuilderWidget->loadArgs(savedCmdArgs);

    // save again in case some preprocessing produce different output
    saveConfig();
}

void Options::saveConfig()
{

    QSettings settings;
    settings.setValue(u"port"_s, portEdit->text());
    settings.setValue(u"RPCSecret"_s, RPCSecretEdit->text());
    settings.setValue(u"expose"_s, expose->isChecked());
    settings.setValue(u"ipv6"_s, useIpv6->isChecked());
    settings.setValue(u"secure"_s, secure->isChecked());
    settings.setValue(u"saveFolder"_s, saveFolderEdit->text());
    settings.setValue(u"pkcs12Path"_s, certPathEdit->text());
    settings.setValue(u"certCheck"_s, certCheck->isChecked());
    settings.setValue(u"runOnStartup"_s, runOnStartup->isChecked());
    settings.setValue(u"advancedUser"_s, advancedUser->isChecked());
    settings.setValue(u"aria2Arguments"_s, cmdArgsBuilderWidget->buildArgs());
    qDebug() << "Saving configuration to" << settings.fileName();
    emit optionsChanged();
}

QStringList Options::buildArgs()
{
    QStringList args = {"--enable-rpc"};

    if (expose->isChecked())
        args += u"--rpc-listen-all"_s;
    if (!useIpv6->isChecked())
        args += u"--disable-ipv6"_s;
    if (secure->isChecked()) {
        if (!QFileInfo::exists(certPathEdit->text())) {
            logsWidget->log(tr("Certificate is not found, encryption will be disabled."),
                            Process::Warning);
        } else {
            args += u"--rpc-secure"_s;
            args += u"--rpc-certificate"_s;
            args += certPathEdit->text().replace(u"'"_s, u"\\'"_s);
        }
    }
    if (!saveFolderEdit->text().isEmpty()) {
        args += u"--dir"_s;
        args += saveFolderEdit->text();
    }
    if (!certCheck->isChecked())
        args += u"--check-certificate=false"_s;
    if (!RPCSecretEdit->text().isEmpty()) {
        args += u"--rpc-secret"_s;
        args += RPCSecretEdit->text();
    }
    if (!portEdit->text().isEmpty())
        args += u"--rpc-listen-port="_s + portEdit->text();

    auto customArgs = cmdArgsBuilderWidget->buildArgs();
    for (int i = 0; i < customArgs.length(); i += 2) {
        QString key   = customArgs.at(i);
        QString value = customArgs.at(i + 1);
        if (key.isEmpty() || value.isEmpty()) // skip if any empty
            continue;
        args += key;
        args += value;
    }

    return args;
}

QRPCSecretValidator::QRPCSecretValidator(QObject *parent) : QValidator(parent) {}

QValidator::State QRPCSecretValidator::validate(QString &input, int &pos) const
{
    if (input.isEmpty())
        return QValidator::Acceptable;
    else if (input.startsWith(" ") || input.endsWith(" "))
        return QValidator::Invalid;
    else if (input.contains(" "))
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}

CmdArgsBuilder::CmdArgsBuilder(QWidget *parent) : QWidget(parent)
{
    rootLayout = new QVBoxLayout;
    rootLayout->setSpacing(-20);
    setLayout(rootLayout);
}

CmdArgsBuilder::kvContainer_t *CmdArgsBuilder::appendKVEdit()
{
    auto base      = new QWidget;
    auto layout    = new QGridLayout(base);
    auto key       = new QLineEdit(base);
    auto value     = new QLineEdit(base);
    auto delButton = new QPushButton("—", base);
    delButton->setFixedWidth(50);
    connect(key, &QLineEdit::textChanged, this, &CmdArgsBuilder::onItemTextChanged);
    connect(value, &QLineEdit::textChanged, this, &CmdArgsBuilder::onItemTextChanged);
    connect(key, &QLineEdit::editingFinished, this, &CmdArgsBuilder::onEditingFinished);
    connect(value, &QLineEdit::editingFinished, this, &CmdArgsBuilder::onEditingFinished);

    layout->addWidget(key, 0, 0, 1, 6);
    layout->addWidget(value, 0, 7, 1, 7);
    layout->addWidget(delButton, 0, 15, 1, 1);

    auto container = new kvContainer_t{
        base,
        key,
        value,
    };

    connect(delButton, &QPushButton::clicked, [this, container] { remove(container); });

    kvContainerList.append(container);
    rootLayout->addWidget(base);
    return container;
}

CmdArgsBuilder::kvContainer_t *CmdArgsBuilder::appendKVEdit(QString key, QString val)
{
    CmdArgsBuilder::kvContainer_t *container = appendKVEdit();
    container->key->setText(key);
    container->value->setText(val);
    return container;
}

void CmdArgsBuilder::removeAt(qsizetype i)
{
    CmdArgsBuilder::kvContainer_t *item = kvContainerList.at(i);
    item->container->deleteLater();
    delete item;
    kvContainerList.removeAt(i);
}

void CmdArgsBuilder::remove(CmdArgsBuilder::kvContainer_t *container)
{
    container->container->deleteLater();
    for (int i = 0; i < kvContainerList.length(); i++) {
        if (kvContainerList.at(i) == container) {
            kvContainerList.removeAt(i);
            break;
        }
    }
    delete container;
}

void CmdArgsBuilder::clear()
{
    for (int i = kvContainerList.length() - 1; i >= 0; i--) {
        removeAt(i);
    }
}

void CmdArgsBuilder::onItemTextChanged(const QString &textChanged)
{
    emit contentChanged(textChanged);
}

void CmdArgsBuilder::onEditingFinished()
{
    emit editingFinished();
}

QStringList CmdArgsBuilder::buildArgs()
{
    QStringList args;
    for (auto i : kvContainerList) {
        args += i->key->text();
        args += i->value->text();
    }

    return args;
}

void CmdArgsBuilder::loadArgs(QStringList args)
{
    clear();
    for (int i = 0; i < args.length(); i += 2) {
        QString key = args.at(i);
        QString val = args.at(i + 1);
        if (key.isEmpty() && val.isEmpty())
            continue;
        appendKVEdit(key, val);
    }
}

} // namespace Aria2Tray
