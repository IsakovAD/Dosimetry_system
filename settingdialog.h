#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

    void fillPortsInfo();

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

private:
    Ui::SettingDialog *ui;

private slots:
    void apply(void);
    void ErrorInOpening(QString);

signals:
    void Communication_settings_changed(int,QString,QString,QString);
};

#endif // SETTINGDIALOG_H
