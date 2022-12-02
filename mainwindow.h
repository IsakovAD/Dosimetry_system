#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QPushButton>
#include <QLineEdit>
#include <QIODevice>
#include <QLabel>
#include <QMessageBox>
#include <QObject>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <fstream>
//#include <settingdialog.h>

float Les_sqrt (float*,float*,float,float,float,int);
const int MOTORX_ID=1,MOTORY_ID=2, STEP_CONST=100, AXIS_RANGEX=400,AXIS_RANGEY=430, WAITING_CONST_MOVE=20, CONST_BACKWARDS=10;

const int TIMER_VALUE=2000;

namespace Ui {
class MainWindow;
}

class Port : public QObject
{
     Q_OBJECT
public:
    explicit Port(QObject *parent =0,QString Newname="COM7",qint32 NewBaud= 2400 , QSerialPort::DataBits NewDataBits = QSerialPort::Data8, QSerialPort::Parity NewParity = QSerialPort::NoParity,  QSerialPort::StopBits NewStopBits = QSerialPort::TwoStop );

     // explicit Port(QObject *parent = 0, QString Newname="COM7");
       ~Port();

       QSerialPort *thisPort;
    QString name;
    QTime myTimer;
    QByteArray Port_data;
    QByteArray Current;
    int CurrentIterator=0;
    int StopFlag=0;
    int CheckCoordinate(int);


    struct PortSettings {
        QString name;
        qint32 baudRate;
        //QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        //QString stringDataBits;
        QSerialPort::Parity parity;
        //QString stringParity;
        QSerialPort::StopBits stopBits;
        //QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        //QString stringFlowControl;
        //bool localEchoEnabled;
    };

    PortSettings Current_settings;


signals:
    void ErrorInOpenPort(QString);
    void ConnectionError(void);
    void LimitSwitchError(void);
    void STOP_MCL(void);
    void CalibrationDone(bool);
    void Nanotec_state(bool,bool,bool,bool,bool,bool);

    void Time_pass(QByteArray &data);
    void outPort(QByteArray &data);
    bool StartComandTransferForMCL(QByteArray, QString);
    void Current_coordinate_changed(int,int);
    void Current_changed(QByteArray&,int);
    void NewDebugMSG(QString);

    void Port_state_changed(QSerialPort *Port, bool state);
    void finished_Port(void);

    void Set_check_box_abs1(bool);
    void Set_check_box_abs2(bool);
    void Set_check_box_2(bool);
    void Set_check_box_1(bool);
    void Set_check_box_05(bool);
    void MotorStat(bool);

public slots:

    bool ComandTransferForMCL(QByteArray, QString);
    void StopButton(void);
    void SetNanotech(void);
    void ClearPositionError(void);
    void Check_Nanotec_state(void);
   // void Calibrate(void);
    void Check_Degrader_State(void);
    void STOP_MOVING(void);
    void UpdateCoordinates(void);
    void OpenClose_SerialPort(bool,QString);
   // void Close_Port(void);
    void process_Port(void);
    void MOVE_MCL (int,int);
    void Calibration(void);
    void ReadUNIDOS(void);
    //void ReadMCL(void);
    void MoveDegrader_05(bool);
    void MoveDegrader_1(bool);
    void MoveDegrader_2(bool);
    void MoveDegrader_abs1(bool);
    void MoveDegrader_abs2(bool);

   //void Calc_time(void);
private:

};


class Something : public QObject
{
    Q_OBJECT
public:
    explicit Something(QObject *parent = 0);
    ~Something();
    bool Abort_flag=false;
    bool CoordinateUpdState=false;
   int CurrentUpdState=0;
private slots:
        void  BeamScan(bool,int,int,int,int);
        void CoordinateUpdated(void);
        void CurrentUpdated(int);
        void AbortScan(void);
signals:
    void killed(void);
    void MoveItTo(int,int);
    void RunItUNIDOS();
    void Draw_Scan();
   void BeamScanDone();
    void NewDebugMSG(QString);
private:

  };


class SettingDialog;






class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Port* COM_MCL;
    Port* COM_DEGRADER;
    Port* COM_UNIDOS;
    Something *It;
    uint timestamp_begin;
    FILE *log;


    virtual void closeEvent(QCloseEvent *event);

    float Linear_interplotation(double);

    QString DirName;
    float pEnergyAtSample,CalibrationConstant, LET, flux,ScaleCoeficient,fluence,total_fluence,DoseInSilicone, AbsorbedDose;
    float GausAmp=0,GausMean=0,GausSigma=0,GausScale=0;
    float expected_amp=0,expected_sigma=0,expected_mean=0;
    float MonitoringArea_Max=1e-15, ScanArea_Max=1e-15;
    int X_current,Y_current,limit_range_random=6, ConnectionState=0, ScanState=0;
    int PositionErrorState;
    QString MCL_port_name="COM1",Degrader_port_name="COM2",UNIDOS_port_name="COM3", DebugStr;

    QByteArray Current;
    QTimer *timer;
    QElapsedTimer  *timeToIntegrateDose;
    bool CurrentUpdated=false, CoordinateUpdated=false, RefreshState=true;
 //void fillPortsInfo();



  SettingDialog *settings;


private slots:

 void OPENCLOSE_ports(int,QString,QString,QString);

// void OpenClose_MCL(bool);
// void OpenClose_UNIDOS(bool);
// void OpenClose_Degrader(bool);
    void UpdateLET(double);
    void ChangeTimerPeriod(int);
    void SetupCalibration(void);
    void PortConnectionError(void);
    void LimitSwitchError(void);
    void SetupScanRange(void);
    void WriteLogFile(void);
    void ShowDebug(bool);
    // void WriteMaxCoordinate(int,int);
     void WriteCurrentCoordinate(int,int);
     void WriteCurrent(QByteArray&,int);
     void Start_timer(bool);
     void Start_refresh(bool);
     void Start_integration(bool);
     void Start_ReachDose(bool);
     void Show_Nanotec_state(bool,bool,bool,bool,bool,bool);
     void CalibrationProceed(bool);

     void ReadNEWCoordinate_Here (void);
     void ReadNEWCoordinate_Move (void);
     void ReadNEWCoordinate_Move2 (void);
      void Draw_Scan_area(void);
   //  void MoveDegrader(void);
     void BeamScanSetup(void);
     void SaveFit(void);
     void FitGaus(void);
     void ResetGaus(void);
     void CalcBeamParams(void);
     void Label_device_state(QSerialPort *thisPort, bool state);
     void WriteDebugMSG(QString);
     void BeamScanState(void);
     void MotorMovingError(bool);
   //  void  BeamScan(void);

signals:

     void StopMovement(void);
    void MoveTo(int,int);
    void StartCalibration(void);

    void RunUNIDOS(void);
   // void RunMCL(void);
   void Main_CoordinateUpdated(void);
    void Current_CoordinateUpdated(int);
    void BeamScanRun(bool,int,int,int,int);
  // void RunPort(bool, QString);
    void RunMCL(bool, QString);
    void RunUNIDOS(bool, QString);
    void RunDegrader(bool, QString);
    void NanotechStartSettings(void);

private:
    Ui::MainWindow *ui;
    void Draw_MCL_area(int,int);
    void Draw_Monitoring_area(void);

  };









#endif // MAINWINDOW_H
