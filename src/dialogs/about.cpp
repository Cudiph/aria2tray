#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>

#include "dialogs/about.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    resize(450, 600);

    setupHeader();
    m_head_layout.addSpacing(10);
    m_head_layout.addWidget(createTabWidget());
    m_head_layout.addWidget(createActionWidget());

    m_head_layout.setAlignment(Qt::AlignCenter);

    setLayout(&m_head_layout);
}

void AboutDialog::setupHeader()
{
    auto logo     = new QLabel(this);
    auto the_name = new QLabel(u"Aria2Tray"_s, this);
    auto version  = new QLabel(tr("Version: ") + A2T_VERSION, this);

    logo->setPixmap(QIcon(":/images/assets/icon.svg").pixmap(QSize(75, 75)));
    logo->setAlignment(Qt::AlignCenter);

    the_name->setAlignment(Qt::AlignCenter);
    QFont name_font = the_name->font();
    name_font.setPointSize(20);
    the_name->setFont(name_font);

    version->setAlignment(Qt::AlignCenter);

    m_head_layout.addWidget(logo);
    m_head_layout.addWidget(the_name);
    m_head_layout.addWidget(version);
}

QTabWidget *AboutDialog::createTabWidget()
{
    auto about_widget   = createAboutWidget();
    auto license_widget = createLicenseWidget();

    m_tab.addTab(about_widget, tr("About"));
    m_tab.addTab(license_widget, tr("License"));

    return &m_tab;
}

QWidget *AboutDialog::createLicenseWidget()
{
    auto wdgt      = new QWidget(this);
    auto layout    = new QVBoxLayout(wdgt);
    auto text_edit = new QTextEdit(wdgt);

    QFile license(":/documents/LICENSE");
    license.open(QIODevice::ReadOnly);
    text_edit->setText(license.readAll());
    text_edit->setReadOnly(true);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    text_edit->setFont(font);

    layout->addWidget(text_edit);

    return wdgt;
}

QWidget *AboutDialog::createAboutWidget()
{
    const QString repo_url  = "https://github.com/Cudiph/Aria2Tray";
    const QString ia2dm_url = "https://github.com/Cudiph/IA2DM";

    auto wdgt        = new QWidget(this);
    auto layout      = new QVBoxLayout(wdgt);
    auto description = new QLabel(tr("Uncomplicated aria2 launcher"), wdgt);
    auto repo =
        new QLabel(tr("Source code") + QString(": <a href=\"%1\">%1</a>").arg(repo_url), wdgt);
    auto ia2dm = new QLabel(
        tr("Works great with") + QString(" <a href=\"%1\">IA2DM</a>").arg(ia2dm_url), wdgt);

    description->setAlignment(Qt::AlignCenter);
    repo->setOpenExternalLinks(true);
    ia2dm->setOpenExternalLinks(true);

    layout->addWidget(description);
    layout->addSpacing(20);
    layout->addWidget(repo);
    layout->addWidget(ia2dm);
    layout->addStretch();

    return wdgt;
}

QWidget *AboutDialog::createActionWidget()
{
    auto wdgt        = new QWidget(this);
    auto layout      = new QHBoxLayout(wdgt);
    auto aboutqt_btn = new QPushButton(tr("About Qt"));
    auto close_btn   = new QPushButton(tr("Close"));

    layout->addWidget(aboutqt_btn);
    layout->addStretch();
    layout->addWidget(close_btn);
    layout->setAlignment(Qt::AlignJustify);

    connect(aboutqt_btn, &QPushButton::clicked, &QApplication::aboutQt);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::close);

    return wdgt;
}

} // namespace Aria2Tray
