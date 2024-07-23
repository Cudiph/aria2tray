#include <QIntValidator>

#include "options.h"

using namespace Qt::Literals::StringLiterals;

// TODO: set tooltip
namespace Aria2Tray {

const QString defaultPort       = u"6800"_s;
const QString defaultRPCSecret  = "";
const bool defaultExpose        = false;
const bool defaultUseIpv6       = false;
const bool defaultSecure        = false;
const QString defaultCertPath   = "";
const QString defaultSaveFolder = "";
const bool defaultCertCheck     = false;
const bool defaultRunOnStartup  = false;
const QStringList defaultCmdArgs =
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
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    rootLayout->addWidget(scrollArea);
    rootLayout->addLayout(actionButtonsLayout());

    proc = Process::instance();
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
    RPCSecretEdit        = new QLineEdit(defaultRPCSecret, this);
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

    vLayout->setAlignment(Qt::AlignTop);
    vLayout->addWidget(saveFolderLayoutRoot);
    vLayout->addWidget(certCheck);
    vLayout->addWidget(runOnStartup);
    miscGroupBox->setLayout(vLayout);

    connect(saveFolderEdit, &QLineEdit::editingFinished, this, &Options::saveConfig);
    connect(certCheck, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(runOnStartup, &QCheckBox::checkStateChanged, this, &Options::saveConfig);
    connect(runOnStartup, &QCheckBox::checkStateChanged, this, &Options::onStartupChange);

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

    return cmdArgsGroupBox;
}

QGridLayout *Options::actionButtonsLayout()
{
    actionGridLayout = new QGridLayout;
    resetButton      = new QPushButton(tr("Reset to default"), this);
    stopButton       = new QPushButton(tr("Force stop"), this);
    startButton      = new QPushButton(tr("START"), this);
    connect(resetButton, &QPushButton::clicked, this, &Options::resetDefault);
    connect(stopButton, &QPushButton::clicked, this, &Options::kill);
    connect(startButton, &QPushButton::clicked, this, &Options::start);

    actionGridLayout->addWidget(resetButton, 0, 0);
    actionGridLayout->addWidget(stopButton, 0, 1);
    actionGridLayout->addWidget(startButton, 1, 0, 1, 2);

    return actionGridLayout;
}

void Options::resetDefault()
{
    portEdit->setText(defaultPort);
    expose->setChecked(defaultExpose);
    useIpv6->setChecked(defaultUseIpv6);
    secure->setChecked(defaultSecure);
    certPathEdit->setText(defaultCertPath);
    certCheck->setChecked(defaultCertCheck);
    runOnStartup->setChecked(defaultRunOnStartup);
    cmdArgsBuilderWidget->loadArgs(defaultCmdArgs);
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
    proc->terminate();
}

void Options::stopWait() { proc->waitForFinished(); }

void Options::kill()
{
    if (proc->state() == Process::NotRunning)
        return;
    startButton->setText(tr("Force stopping..."));
    startButton->setDisabled(true);
    qDebug() << "Force stopping aria2...";
    proc->kill();
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

// TODO: fix logo and publisher and name
#ifdef Q_OS_WIN32
const QString REG_KEY = u"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"_s;

void Options::addStartup()
{
    QSettings settings(REG_KEY, QSettings::NativeFormat);
    settings.setValue(QCoreApplication::applicationName(),
                      '"' + QCoreApplication::applicationFilePath().replace('/', '\\') + '"');
}

void Options::removeStartup()
{
    QSettings settings(REG_KEY, QSettings::NativeFormat);
    settings.remove(QCoreApplication::applicationName());
}

#elif defined(Q_OS_LINUX)

void Options::addStartup() {}
void Options::removeStartup() {}

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

void Options::addNewArgs() { cmdArgsBuilderWidget->appendKVEdit(); }

void Options::loadConfig()
{
    QSettings settings;
    auto savedPort         = settings.value(u"port"_s, defaultPort).toString();
    auto savedRPCSecret    = settings.value(u"RPCSecret"_s, defaultRPCSecret).toString();
    auto savedExpose       = settings.value(u"expose"_s, defaultExpose).toBool();
    auto savedUseIpv6      = settings.value(u"ipv6"_s, defaultUseIpv6).toBool();
    auto savedSecure       = settings.value(u"secure"_s, defaultSecure).toBool();
    auto savedSaveFolder   = settings.value(u"saveFolder"_s, defaultSaveFolder).toString();
    auto savedCertPath     = settings.value(u"pkcs12Path"_s, defaultCertPath).toString();
    auto savedCertCheck    = settings.value(u"certCheck"_s, defaultCertCheck).toBool();
    auto savedRunOnStartup = settings.value(u"runOnStartup"_s, defaultRunOnStartup).toBool();
    auto savedCmdArgs      = settings.value(u"aria2Arguments"_s, defaultCmdArgs).toStringList();

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
    settings.setValue(u"aria2Arguments"_s, cmdArgsBuilderWidget->buildArgs());
    qDebug() << "Saving configuration to" << settings.fileName();
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
    // TODO: add sorta table header
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
    delete item->container;
    delete item;
    kvContainerList.removeAt(i);
}

void CmdArgsBuilder::remove(CmdArgsBuilder::kvContainer_t *container)
{
    delete container->container;
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

void CmdArgsBuilder::onEditingFinished() { emit editingFinished(); }

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
