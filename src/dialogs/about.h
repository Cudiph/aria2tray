#ifndef ARIA2TRAY_DIALOG_ABOUT_H
#define ARIA2TRAY_DIALOG_ABOUT_H

#include "qboxlayout.h"
#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>

namespace Aria2Tray {

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = nullptr);

private:
    void setupHeader();
    QTabWidget *createTabWidget();

    QWidget *createLicenseWidget();
    QWidget *createAboutWidget();
    QWidget *createActionWidget();

    QVBoxLayout m_head_layout;
    QTabWidget m_tab;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_DIALOG_ABOUT_H
