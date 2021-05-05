#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QtSerialPort/QSerialPortInfo>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);


    fillPortsInfo();

 connect(ui->pushButton,SIGNAL(clicked(bool)),this, SLOT(apply()));
}


void SettingDialog::apply(void)
{
    int state;
    state=(int)ui->checkBox_COM_MCL->isChecked()+(int)ui->checkBox_COM_Degrader->isChecked()*2+(int)ui->checkBox_COM_UNIDOS->isChecked()*4;
    emit Communication_settings_changed(state,ui->comboBox_MCL->currentText().split(':')[0],ui->comboBox_Degrader->currentText().split(':')[0],ui->comboBox_UNIDOS->currentText().split(':')[0]);
     hide();
}


void SettingDialog:: ErrorInOpening(QString error_name)
{
    if (ui->comboBox_MCL->currentText().contains(error_name))
    {
        ui->checkBox_COM_MCL->setChecked(!ui->checkBox_COM_MCL->isChecked());
        return;
    }


    if (ui->comboBox_Degrader->currentText().contains(error_name))
    {
        ui->checkBox_COM_Degrader->setChecked(!ui->checkBox_COM_Degrader->isChecked());
        return;
    }
       if (ui->comboBox_MCL->currentText().contains(error_name))
    {
        ui->checkBox_COM_UNIDOS->setChecked(!ui->checkBox_COM_UNIDOS->isChecked());
        return;
    }



}
void SettingDialog::fillPortsInfo()
{
  ui->comboBox_MCL->clear();
   ui->comboBox_UNIDOS->clear();
 ui->comboBox_Degrader->clear();

   // QString description;
   // QString manufacturer;
   // QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
  //      QStringList list;
      //  description = info.description();
      //  manufacturer = info.manufacturer();
      //  serialNumber = info.serialNumber();
      /*  list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
*/
       ui->comboBox_MCL->addItem(info.portName()+": "+info.description());
     ui->comboBox_UNIDOS->addItem(info.portName()+": "+info.description());
    ui->comboBox_Degrader->addItem(info.portName()+": "+info.description());
    }

    //ui->serialPortInfoListBox->addItem(tr("Custom"));
}


SettingDialog::~SettingDialog()
{
    delete ui;
}
