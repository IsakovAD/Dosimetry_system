#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "QTime"
#include "settingdialog.h"
#include "qmessagebox"
#include "qcloseevent"


static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings= new  SettingDialog ;
    connect(ui->actionSet_connection, &QAction::triggered, settings, &MainWindow::show);

  //  fillPortsInfo();                //Fill names of avaible ports to combo boxes (can be used to get more information about serial devices)
    X_current=0,Y_current=0, Current= QByteArray::number(0);




//--------------------WIDGETS Setting---------------------------
ui->Current_X->setEnabled(false);
ui->Current_Y->setEnabled(false);
ui->Current_X->setText(QString::number(X_current));
ui->Current_Y->setText(QString::number(Y_current));

ui->checkBox_ErrorX->setEnabled(false);
ui->checkBox_ErrorY->setEnabled(false);
ui->checkBox_ZeroX->setEnabled(false);
ui->checkBox_ZeroY->setEnabled(false);
ui->checkBox_ReadyX->setEnabled(false);
ui->checkBox_ReadyY->setEnabled(false);

//parameters for calculation dose
limit_range_random=20; //AID 06_05
CalibrationConstant=0;
pEnergyAtSample=0;
LET=0;
flux=0,
ScaleCoeficient=1,
fluence=0,
total_fluence=0,
DoseInSilicone=0;
AbsorbedDose=0;

CalcBeamParams(); //calculate parameters of the beam according to the known used absorber plates


ui->BeamMonitoring_button->setText("START Beam Monitoring");
ui->BeamMonitoring_button->setCheckable(true);
ui->BeamRefresh_button->setCheckable(true);
ui->BeamIntegrating_button->setCheckable(true);
ui->BeamReachDose_button->setCheckable(true);
ui->pushButton_Debug->setCheckable(true);

ui->ScaleSpinBox->setValue(1);
ui->ScaleSpinBox->setMaximum(1000);
ui->spinBox_Period->setValue(2);
ui->spinBox_Avr_range->setValue(2);
ui->spinBox_Step->setValue(10);

connect(ui->BeamMonitoring_button, SIGNAL(toggled(bool)),this,SLOT(Start_timer(bool)));
connect(ui->BeamRefresh_button, SIGNAL(toggled(bool)),this,SLOT(Start_refresh(bool)));
connect(ui->BeamReachDose_button, SIGNAL(toggled(bool)),this,SLOT(Start_ReachDose(bool)));
connect(ui->BeamIntegrating_button, SIGNAL(toggled(bool)),this,SLOT(Start_integration(bool)));
connect(ui->pushButton_Debug, SIGNAL(toggled(bool)),this,SLOT(ShowDebug(bool)));

connect(ui->AxisX_Button,SIGNAL(toggled(bool)),this,SLOT(SetupScanRange()));

//connect(ui->toolButton, SIGNAL(),this,SLOT(Start_timer(bool)));

ui->AxisX_Button->setChecked(true);
ui->spinBox_ScanMax->setValue(100);


timestamp_begin=QDateTime::currentDateTime().toTime_t();
 DirName=QString::number(QDateTime::currentDateTime().toTime_t());
  QDir().mkdir(DirName);


ui->spinBox_Step->setMinimum(1);
ui->spinBox_Period->setMinimum(1);

ui->spinBox_Avr_range->setMinimum(2);

ui->spinBox_AxisRange->setValue(5);


ui->spinBox_TargetDose->setValue(100);
ui->spinBox_TargetDose->setMaximum(500);




ui->spinBox_HereX->setMinimum(0);
ui->spinBox_HereX->setMaximum(AXIS_RANGEX);
ui->spinBox_HereY->setMinimum(0);
ui->spinBox_HereY->setMaximum(AXIS_RANGEY);
ui->spinBox_AxisRange->setMaximum(999);

ui->spinBox_MoveX->setMinimum(0);
ui->spinBox_MoveX->setMaximum(AXIS_RANGEX);
ui->spinBox_MoveY->setMinimum(0);
ui->spinBox_MoveY->setMaximum(AXIS_RANGEY);


ui->spinBox_MoveX_2->setMinimum(0);
ui->spinBox_MoveX_2->setMaximum(AXIS_RANGEX);
ui->spinBox_MoveY_2->setMinimum(0);
ui->spinBox_MoveY_2->setMaximum(AXIS_RANGEY);







//ui->pushButton_CheckPosition->setDisabled(true);
//ui->Move_button->setDisabled(true);
ui->Move_button2->setDisabled(true);
ui->Here_button->setDisabled(true);




//All state labels is OFF (Red)
ui->label_MCL_state->setStyleSheet("QLabel { background-color : red; }");
ui->label_MCL_state->setAlignment(Qt::AlignCenter);

ui->label_UNIDOS_state->setStyleSheet("QLabel { background-color : red;}");
ui->label_UNIDOS_state->setAlignment(Qt::AlignCenter);

ui->label_calibration_state->setStyleSheet("QLabel { background-color : Red; font-size: 11pt;font-weight: bold;}");
ui->label_calibration_state->setText("NEED CALIBRATION");



ui->label_monitoring_state->setStyleSheet("QLabel { background-color : red; }");
ui->label_monitoring_state->setAlignment(Qt::AlignCenter);

//Setting of labels for radiation status
ui->label_Avr_current->setAlignment(Qt::AlignCenter);
ui->label_Avr_current->setText("Average chamber current [pA] per " +QString::number(ui->spinBox_Period->value()*ui->spinBox_Avr_range->value()) + "s  " +  QString::number(0));

ui->label_Avr_flux->setAlignment(Qt::AlignCenter);
ui->label_Avr_flux->setText("Average proton flux  [cm^-2*s^-1] per " +QString::number(ui->spinBox_Period->value()*ui->spinBox_Avr_range->value()) + "s  "  + QString::number(0));

ui->label_Int_dose->setAlignment(Qt::AlignCenter);
ui->label_Int_dose->setText("Integrated dose in silicon [krad]  " +  QString::number(0));

ui->label_Fluence->setAlignment(Qt::AlignCenter);
ui->label_Fluence->setText("Fluence [cm^-2]  "+ QString::number(0));

ui->label_Sil_dose->setAlignment(Qt::AlignCenter);
ui->label_Sil_dose->setText("Dose in silicon  [krad]  "+ QString::number(0) );


ui->Info_label_1->setAlignment(Qt::AlignCenter);
ui->Info_label_1->setText("k [pA^-1 cm^-2 s^-1]=    " + QString::number(CalibrationConstant) +  "\n  E of p hitting sample [MeV]=   " + QString::number(pEnergyAtSample) +   " \n LET in Si [MeV/ (mg/cm2)]=    ");
ui->doubleSpinBox_LET->setValue(LET);
ui->Info_label_2->setAlignment(Qt::AlignCenter);
ui->Info_label_2->setText("Gaus Amplitude [nA] " + QString::number(expected_amp) +  "\n \n Gaus Mean [mm] " + QString::number(expected_mean)+ "\n \n Gaus Sigma [mm] " + QString::number(expected_sigma) +  "\n \n Scale (approx.) " + QString::number(GausScale));



//------------TIMER--------------------------------------

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(WriteLogFile()));  //Calculate all radiation parameters and write it to log file




//--------------------Draw MCL AREA

    Draw_MCL_area(X_current,Y_current);


 //--------------------Draw Monitoring AREA
    ui->Monitoring_area->addGraph();
    ui->Monitoring_area->axisRect()->setupFullAxesBox();
    Draw_Monitoring_area();

  //Draw Scan Area
    ui->Scan_area->addGraph();
     ui->Scan_area->addGraph();
    ui->Scan_area->axisRect()->setupFullAxesBox();
   // Draw_Scan_area();


//-----COM_MCL------------------------------------------------
//COM19
   COM_MCL=new Port(0,MCL_port_name,115200,QSerialPort::Data8,QSerialPort::NoParity,QSerialPort::OneStop);

    QThread *thread_New = new QThread;




    connect(this, SIGNAL(RunMCL(bool,QString)),COM_MCL, SLOT(OpenClose_SerialPort(bool,QString))); //read settings of com port and open/close it

     connect(thread_New, SIGNAL(started()), COM_MCL, SLOT(process_Port()));// probably unnecessary
     connect(COM_MCL, SIGNAL(finished_Port()), thread_New, SLOT(quit()));// ??? some QTmagic
     connect(COM_MCL, SIGNAL(finished_Port()), thread_New, SLOT(deleteLater()));// destructor of the thread




     connect(this,SIGNAL(NanotechStartSettings()),COM_MCL,SLOT(SetNanotech()));

     //connect(ui->Calibrate_button, SIGNAL(clicked(bool)),COM_MCL, SLOT(Calibrate())); //start procedure of calibration

     connect(ui->Here_button,SIGNAL(clicked(bool)),this,SLOT(ReadNEWCoordinate_Here()));
     connect(ui->Move_button,SIGNAL(clicked(bool)),this,SLOT(ReadNEWCoordinate_Move()));
     connect(ui->Move_button2,SIGNAL(clicked(bool)),this,SLOT(ReadNEWCoordinate_Move2()));
     connect(ui->pushButton_CheckPosition,SIGNAL(clicked(bool)),COM_MCL,SLOT(UpdateCoordinates()));
    connect(ui->pushButton_ClrError,SIGNAL(clicked(bool)), COM_MCL,SLOT(ClearPositionError()));




    connect (ui->pushButton_CheckState,SIGNAL(clicked(bool)),COM_MCL,SLOT(Check_Nanotec_state()));
    connect (COM_MCL,SIGNAL(Nanotec_state(bool,bool,bool,bool,bool,bool)),this,SLOT(Show_Nanotec_state(bool,bool,bool,bool,bool,bool)));





     connect(ui->pushButton_STOP,SIGNAL(clicked(bool)),COM_MCL,SLOT(StopButton()),Qt::DirectConnection);
    connect(COM_MCL,SIGNAL(CalibrationDone(bool)),this,SLOT(CalibrationProceed(bool)));
    // connect(COM_MCL,SIGNAL(STOP_MCL()),COM_MCL,SLOT(STOP_MOVING()),Qt::DirectConnection);
    // connect(COM_MCL,SIGNAL(StartComandTransferForMCL(QByteArray, QString)),COM_MCL,SLOT(ComandTransferForMCL(QByteArray,QString)),Qt::DirectConnection);
    // connect(this,SIGNAL(StopMovement()),COM_MCL,SLOT(STOP_MOVING()),Qt::DirectConnection);
    connect(COM_MCL,SIGNAL(ConnectionError()),this,SLOT(PortConnectionError()));
    connect(COM_MCL,SIGNAL(LimitSwitchError()),this,SLOT(LimitSwitchError()));

    connect(ui->pushButton_Calibration,SIGNAL(clicked()),this,SLOT(SetupCalibration()));
    connect(this,SIGNAL(StartCalibration()),COM_MCL,SLOT(Calibration()));
    connect(COM_MCL,SIGNAL(MotorStat(bool)),this,SLOT(MotorMovingError(bool)));
     connect(ui->FtGaus_button,SIGNAL(clicked(bool)),this,SLOT(FitGaus()));

     connect(this,SIGNAL(MoveTo(int,int)),COM_MCL,SLOT(MOVE_MCL(int,int)));


     connect(COM_MCL, SIGNAL(Current_coordinate_changed(int,int)),this, SLOT(WriteCurrentCoordinate(int,int)));

   //  connect(this,SIGNAL(RunMCL()),COM_MCL,SLOT(ReadMCL())); // why do I have TWO runMCL? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

     COM_MCL->moveToThread(thread_New);
     COM_MCL->thisPort->moveToThread(thread_New);
     thread_New->start();
//------------------DEGRADER-------------------------------------------------

//COM20
     COM_DEGRADER=new Port(0,Degrader_port_name,115200 ,QSerialPort::Data8,QSerialPort::NoParity,QSerialPort::OneStop);

   //  COM_DEGRADER=new Port(0,"COM2");

     QThread *thread_New2 = new QThread;
     COM_DEGRADER->moveToThread(thread_New2);
     COM_DEGRADER->thisPort->moveToThread(thread_New2);


      connect(thread_New2, SIGNAL(started()), COM_DEGRADER, SLOT(process_Port()));//Unnecessary probably
      connect(COM_DEGRADER, SIGNAL(finished_Port()), thread_New2, SLOT(quit()));//???
      connect(COM_DEGRADER, SIGNAL(finished_Port()), thread_New2, SLOT(deleteLater()));//destructor



      connect(COM_DEGRADER,SIGNAL(Set_check_box_abs1(bool)),ui->checkBox_Degrader_abs1,SLOT(setChecked(bool)));
      connect(COM_DEGRADER,SIGNAL(Set_check_box_abs2(bool)),ui->checkBox_Degrader_abs2,SLOT(setChecked(bool)));
      connect(COM_DEGRADER,SIGNAL(Set_check_box_1(bool)),ui->checkBox_Degrader_1,SLOT(setChecked(bool)));
      connect(COM_DEGRADER,SIGNAL(Set_check_box_2(bool)),ui->checkBox_Degrader_2,SLOT(setChecked(bool)));
      connect(COM_DEGRADER,SIGNAL(Set_check_box_05(bool)),ui->checkBox_Degrader_05,SLOT(setChecked(bool)));\

      connect(ui->checkBox_Degrader_1,SIGNAL(stateChanged(int)),this,SLOT(CalcBeamParams()));
      connect(ui->checkBox_Degrader_05,SIGNAL(stateChanged(int)),this,SLOT(CalcBeamParams()));
      connect(ui->checkBox_Degrader_2,SIGNAL(stateChanged(int)),this,SLOT(CalcBeamParams()));

      connect(ui->CheckState_button,SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(Check_Degrader_State()));

      connect(this, SIGNAL(RunDegrader(bool,QString)),COM_DEGRADER, SLOT(OpenClose_SerialPort(bool,QString)));

      thread_New2->start();



    connect(ui->checkBox_Degrader_05, SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(MoveDegrader_05(bool)));
    connect(ui->checkBox_Degrader_1, SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(MoveDegrader_1(bool)));
    connect(ui->checkBox_Degrader_2, SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(MoveDegrader_2(bool)));
    connect(ui->checkBox_Degrader_abs1, SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(MoveDegrader_abs1(bool)));
    connect(ui->checkBox_Degrader_abs2, SIGNAL(clicked(bool)),COM_DEGRADER,SLOT(MoveDegrader_abs2(bool)));

//----------------------UNIDOS------------------------------





  //   COM_UNIDOS=new Port(0,"COM5");
//COM18
    COM_UNIDOS=new Port(0,UNIDOS_port_name,9600 ,QSerialPort::Data8,QSerialPort::NoParity,QSerialPort::OneStop);


    if (COM_UNIDOS== NULL) qDebug("Error with opening UNIDOS");

     QThread *thread_New3 = new QThread;
     COM_UNIDOS->moveToThread(thread_New3);
     COM_UNIDOS->thisPort->moveToThread(thread_New3);



     connect(this, SIGNAL(RunUNIDOS(bool,QString)),COM_UNIDOS, SLOT(OpenClose_SerialPort(bool,QString)));

      connect(thread_New3, SIGNAL(started()), COM_UNIDOS, SLOT(process_Port()));//we don`t need it
      connect(COM_UNIDOS, SIGNAL(finished_Port()), thread_New3, SLOT(quit()));//???
      connect(COM_UNIDOS, SIGNAL(finished_Port()), thread_New3, SLOT(deleteLater()));//destructor

      thread_New3->start();

      connect(this,SIGNAL(RunUNIDOS()),COM_UNIDOS,SLOT(ReadUNIDOS())); // why do I have TWO RunUNIDOS? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
     connect(COM_UNIDOS, SIGNAL(Current_changed(QByteArray&,int)),this, SLOT(WriteCurrent(QByteArray&,int)),Qt::DirectConnection);
//-------------------SCAN----------------------------------



It= new Something();

    QThread *thread_New4 = new QThread;
   It->moveToThread(thread_New4);


   connect(It, SIGNAL(killed()), thread_New4, SLOT(quit()));//???
   connect(It, SIGNAL(killed()), thread_New4, SLOT(deleteLater()));//destructor
    connect(It,SIGNAL(BeamScanDone()),this,SLOT(BeamScanState()));
   connect(ui->StartScan_button,SIGNAL(clicked(bool)),this,SLOT(BeamScanSetup()));
   connect(this,SIGNAL(BeamScanRun(bool,int,int,int,int)),It,SLOT(BeamScan(bool,int,int,int,int)));

   connect(It,SIGNAL(MoveItTo(int,int)),COM_MCL,SLOT(MOVE_MCL(int,int)));
     connect(It,SIGNAL(RunItUNIDOS()),COM_UNIDOS,SLOT(ReadUNIDOS()));

     connect(this, SIGNAL(Main_CoordinateUpdated()),It, SLOT(CoordinateUpdated()),Qt::DirectConnection);
     connect(this, SIGNAL(Current_CoordinateUpdated(int)),It, SLOT(CurrentUpdated(int)),Qt::DirectConnection);

     connect(It, SIGNAL(Draw_Scan()),this, SLOT(Draw_Scan_area()));
     connect(ui->SaveFit_Button,SIGNAL(clicked(bool)),this,SLOT(SaveFit()));
    connect(ui->AbortScan_button,SIGNAL(clicked(bool)),It,SLOT(AbortScan()),Qt::DirectConnection);






thread_New4->start();

//-------------------Change Label states
     connect(COM_MCL,SIGNAL(Port_state_changed(QSerialPort*,bool)),this,SLOT(Label_device_state(QSerialPort*,bool)));
     connect(COM_UNIDOS,SIGNAL(Port_state_changed(QSerialPort*,bool)),this,SLOT(Label_device_state(QSerialPort*,bool)));

     connect(settings,SIGNAL(Communication_settings_changed(int,QString,QString,QString)),this,SLOT(OPENCLOSE_ports(int,QString,QString,QString)));

     /*
     connect(ui->checkBox_COM_MCL, SIGNAL(clicked(bool)),this, SLOT(OpenClose_MCL(bool)));
     connect(ui->checkBox_COM_Degrader, SIGNAL(clicked(bool)),this, SLOT(OpenClose_Degrader(bool)));
     connect(ui->checkBox_COM_UNIDOS, SIGNAL(clicked(bool)),this, SLOT(OpenClose_UNIDOS(bool)));
     */

 //---------------------------------experiments with scroll area

//     ui->scrollArea->setWidget(ui->textBrowser);

     //ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//     ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


     connect(COM_MCL,SIGNAL(NewDebugMSG(QString)),this,SLOT(WriteDebugMSG(QString)));

     connect(COM_UNIDOS,SIGNAL(NewDebugMSG(QString)),this,SLOT(WriteDebugMSG(QString)));

     connect(COM_DEGRADER,SIGNAL(NewDebugMSG(QString)),this,SLOT(WriteDebugMSG(QString)));
     connect(It,SIGNAL(NewDebugMSG(QString)),this,SLOT(WriteDebugMSG(QString)));


     connect(COM_MCL,SIGNAL(ErrorInOpenPort(QString)),settings,SLOT(ErrorInOpening(QString)));
     connect(COM_UNIDOS,SIGNAL(ErrorInOpenPort(QString)),settings,SLOT(ErrorInOpening(QString)));
     connect(COM_DEGRADER,SIGNAL(ErrorInOpenPort(QString)),settings,SLOT(ErrorInOpening(QString)));


//     initActionsConnections();


}



void MainWindow::ShowDebug(bool state)
{

    if (state)
    {
        ui->pushButton_Debug->setText("Close Debug");
        ui->textBrowser->setGeometry(402,450,384,191);

    }
    else
    {
        ui->pushButton_Debug->setText("Expand Debug");
        ui->textBrowser->setGeometry(402,600,384,41);



    }

}

void MainWindow:: WriteDebugMSG(QString msg)
{

    DebugStr+=msg;
    ui->textBrowser->setText(DebugStr);
    QScrollBar *sb= ui->textBrowser->verticalScrollBar();

    sb->setValue(sb->maximum());

}


void MainWindow:: FitGaus(){

    float max=0,temp=0;
    int max_coordinate;
    const QCPDataMap *dataMap= ui->Scan_area->graph(0)->data();


    if (dataMap->empty()){
        WriteDebugMSG("No Scan found");
        return;
    }

    QMap<double,QCPData>::const_iterator i=dataMap->constBegin();
    int size =dataMap->size(),j=0;

    float *Graph_Coordinate= new float[size];
    float *Graph_Current= new float[size];
    for (i=dataMap->constBegin();i!=dataMap->constEnd();i++){
          Graph_Current[j] = i.value().value ;
          Graph_Coordinate[j] = i.value().key ;
          j++;
    }


    qsrand(QDateTime::currentMSecsSinceEpoch());
    float fit_ampl[8]={18,19,20,20,0,0,0,0};
    float fit_mean[8]={48,50,52,49,0,0,0,0};
    float fit_sigma[8]={40,50,45,55,0,0,0,0};


    if (limit_range_random==20){

     // First predictions of parameters, "limit_range_random==20" means that we first time here
        for (int i=0;i<size;i++){
            if (Graph_Current[i]>max){
                max= Graph_Current[i];
                max_coordinate=i;
            }
        }
        ScanArea_Max=max; //AID 06_05 DELETE THIS LATER

        expected_amp=Graph_Current[max_coordinate];
        expected_mean=Graph_Coordinate[max_coordinate];

        float first=Graph_Coordinate[0];
        float second=Graph_Coordinate[size-1];
        int flag=0;
        for(int i=0;i<size;i++){
            if(Graph_Current[i]>=max/2. && flag==0){
                first=Graph_Coordinate[i];
                flag=1;
                continue;
            }
            if( Graph_Current[i]<=max/2. && flag==1){
                second=Graph_Coordinate[i];
                break;
            }
       }

       expected_sigma=(second-first)/2.355;
       //WriteDebugMSG("Expected Sigma "+ QString::number(expected_sigma)+ " amp "+ QString::number(expected_amp)+" mean "+ QString::number(expected_mean) );

    }

     //we need to have 4 random values of parameters. amplitude is always fixed
     for (int i=0;i<4;i++){
         int sign_mean;
         int sign_sigma;

         if (i%2==1) sign_mean=1;
         else sign_mean=-1;

         if (i<2) sign_sigma=1;
         else sign_sigma=-1;


         fit_ampl[i]=expected_amp;

         fit_sigma[i]=expected_sigma  + sign_mean* fabs(expected_sigma*((qrand() % limit_range_random -(int) limit_range_random/2))/100);
         fit_mean[i]=expected_mean  +  sign_sigma* fabs(expected_mean*((qrand() % limit_range_random -(int) limit_range_random/2))/100);

         //WriteDebugMSG("RandomUpdates: fit_sigma[i]: "+ QString::number(fit_sigma[i]-expected_sigma) + " fit_mean "+QString::number(fit_mean[i]-expected_mean) +"\n"  );

     }

    if (limit_range_random>10) limit_range_random--;  //aid 6Feb_2020
    float k=0, e=0.01, alpha=1, beta=0.5, gamma=2.9,sum=0;

    while (k<50){
        for (int i=0;i<4;i++){
            for (int j=0;j<3;j++){
                if (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[j],fit_mean[j],fit_sigma[j],size)<Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[j+1],fit_mean[j+1],fit_sigma[j+1],size)){
                    temp=fit_ampl[j];
                    fit_ampl[j]=fit_ampl[j+1];
                    fit_ampl[j+1]=temp;

                    temp=fit_sigma[j];
                    fit_sigma[j]=fit_sigma[j+1];
                    fit_sigma[j+1]=temp;

                    temp=fit_mean[j];
                    fit_mean[j]=fit_mean[j+1];
                    fit_mean[j+1]=temp;
                 }
              }
         }
     //     4 - best, 1 - worst
     //    find center of mass

    fit_ampl[4]=0;
    fit_sigma[4]=0;
    fit_mean[4]=0;

    for (int i=1;i<4;i++){ //all without wrost one
        fit_ampl[4]+=fit_ampl[i];
        fit_sigma[4]+=fit_sigma[i];
        fit_mean[4]+=fit_mean[i];
    }

    fit_ampl[4]/=3;
    fit_sigma[4]/=3;
    fit_mean[4]/=3;

    sum=0;
    for (int i=0;i<4;i++){
        sum+= pow((Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[i],fit_mean[i],fit_sigma[i],size) - Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[4],fit_mean[4],fit_sigma[4],size)),2);
    }
    sum=sqrt(sum)/4;

    if (sum<=e){
        break;
    }
    // reflection

    fit_ampl[5]=fit_ampl[4] + alpha*(fit_ampl[4]-fit_ampl[0]);
    fit_sigma[5]=fit_sigma[4] + alpha*(fit_sigma[4]-fit_sigma[0]);
    fit_mean[5]=fit_mean[4] + alpha*(fit_mean[4]-fit_mean[0]);


    if (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)<=Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[3],fit_mean[3],fit_sigma[3],size)){

        fit_ampl[6]=fit_ampl[4] + gamma*(fit_ampl[5]-fit_ampl[4]);
        fit_sigma[6]=fit_sigma[4] + gamma*(fit_sigma[5]-fit_sigma[4]);
        fit_mean[6]=fit_mean[4] + gamma*(fit_mean[5]-fit_mean[4]);
           if (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[6],fit_mean[6],fit_sigma[6],size)<Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[3],fit_mean[3],fit_sigma[3],size)){
               fit_ampl[0]=fit_ampl[6];
               fit_sigma[0]=fit_sigma[6];
               fit_mean[0]=fit_mean[6];
           }else{
               fit_ampl[0]=fit_ampl[5];
               fit_sigma[0]=fit_sigma[5];
               fit_mean[0]=fit_mean[5];
           }
         k++;
         continue;
     }


      if ((Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[2],fit_mean[2],fit_sigma[2],size)<Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)) && (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)<=Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[0],fit_mean[0],fit_sigma[0],size))){
          fit_ampl[7]=fit_ampl[4] + beta*(fit_ampl[0]-fit_ampl[4]) ;
          fit_sigma[7]=fit_sigma[4] + beta*(fit_sigma[0]-fit_sigma[4]);
          fit_mean[7]=fit_mean[4] + beta*(fit_mean[0]-fit_mean[4]);

          fit_ampl[0]=fit_ampl[7];
          fit_sigma[0]=fit_sigma[7];
          fit_mean[0]=fit_mean[7];
          k++;
          continue;
      }


      if ((Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[3],fit_mean[3],fit_sigma[3],size)<Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)) && (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)<=Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[2],fit_mean[2],fit_sigma[2],size))){
          fit_ampl[0]=fit_ampl[5];
          fit_sigma[0]=fit_sigma[5];
          fit_mean[0]=fit_mean[5];
          k++;
          continue;
      }

      if (Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[5],fit_mean[5],fit_sigma[5],size)>Les_sqrt(Graph_Coordinate,Graph_Current,fit_ampl[0],fit_mean[0],fit_sigma[0],size)){
          for (int i=0;i<4;i++){
              fit_ampl[i]=fit_ampl[3]+0.5*(fit_ampl[i]-fit_ampl[3]);
              fit_sigma[i]=fit_sigma[3]+0.5*(fit_sigma[i]-fit_sigma[3]);
              fit_mean[i]=fit_mean[3]+0.5*(fit_mean[i]-fit_mean[3]);
           }
           k++;
           continue;
      }
      k++;


   }

    ui->Scan_area->axisRect()->setupFullAxesBox();
    expected_amp=fit_ampl[4];
    expected_sigma=fit_sigma[4];
    expected_mean=fit_mean[4];

    //WriteDebugMSG("Results Expected Sigma "+ QString::number(expected_sigma)+ " amp "+ QString::number(expected_amp)+" mean "+ QString::number(expected_mean) );



    if (ui->AxisX_Button->isChecked())
        GausScale=fit_ampl[4]*exp(-0.5*pow((ui->spinBox_HereX->value()-fit_mean[4]),2)/pow(fit_sigma[4],2)) / (fit_ampl[4]*exp(-0.5*pow((ui->spinBox_MoveX->value()-fit_mean[4]),2)/pow(fit_sigma[4],2)));
    else
        GausScale=fit_ampl[4]*exp(-0.5*pow((ui->spinBox_HereY->value()-fit_mean[4]),2)/pow(fit_sigma[4],2)) / (fit_ampl[4]*exp(-0.5*pow((ui->spinBox_MoveY->value()-fit_mean[4]),2)/pow(fit_sigma[4],2)));

    ui->Info_label_2->setText("Gaus Amplitude [nA] " + QString::number(expected_amp) +  "\n \n Gaus Mean [mm] " + QString::number(expected_mean)+ "\n \n Gaus Sigma [mm] " + QString::number(expected_sigma) +  "\n \n Scale (approx.) " + QString::number(GausScale));
    float step = (ui->spinBox_ScanMax->value() -  ui->spinBox_ScanMin->value())/100.;

    qDebug()<<"Step"<< step;
    qDebug()<<" MAX: "<<ui->spinBox_ScanMax->value() <<" MIN: "<<ui->spinBox_ScanMin->value();

    float coordinate=ui->spinBox_ScanMin->value();
    ui->Scan_area->graph(1)->setPen(QPen(QColor(Qt::red)));
    ui->Scan_area->graph(1)->clearData();
    qDebug("Im plotting!");
    for (int i=1;i<100;i++){
        ui->Scan_area->graph(1)->addData(coordinate,fit_ampl[4]*exp(-0.5*pow((coordinate-fit_mean[4]),2)/pow(fit_sigma[4],2)));
        coordinate+=step;
    }
    ui->Scan_area->graph(1)->setAntialiased(false);         // Dummy solution to avoid additional points for the scan plot

    Draw_Scan_area();

}

float  Les_sqrt (float *x, float *y, float ampl, float mean, float sigma, int size){

    float ans=0;

             for (int i=0;i<size;i++)
                {
                    ans+=pow(y[i]-ampl*exp(-0.5*pow((x[i]-mean),2)/pow(sigma,2)),2);                      \
                }

    return ans;
}


void MainWindow:: OPENCLOSE_ports(int state,QString MCL_name,QString Degrader_name,QString UNIDOS_name)
{

    qDebug()<<"OpenPorts with state"<<state<< " "<<MCL_name<<" "<<Degrader_name<<" "<<UNIDOS_name;

    if (state!=ConnectionState)
    {
        qDebug()<<"Inside"<<state<< ConnectionState;

        if ( (state & 1) != (ConnectionState & 1))
        {
              qDebug()<<"MCL"<<(state & 1);
            emit RunMCL(state & 1,MCL_name);
            MCL_port_name=MCL_name;
        }
        if ( ((state & 2) >> 1) != ((ConnectionState & 2) >> 1) )
        {
            emit RunDegrader((state & 2) >> 1,Degrader_name);
            Degrader_port_name=Degrader_name;
        }
        if (((state & 4) >> 2)  != ((ConnectionState & 4) >> 2))
        {
            emit RunUNIDOS((state & 4) >> 2 ,UNIDOS_name);
            UNIDOS_port_name=UNIDOS_name;
        }
        ConnectionState=state;

    }

}
/*
void MainWindow:: OpenClose_MCL(bool state){


    emit RunMCL(state, ui->comboBox_MCL->currentText().split(':')[0]);


}

void MainWindow:: OpenClose_UNIDOS(bool state){

    emit RunUNIDOS(state, ui->comboBox_UNIDOS->currentText().split(':')[0]);


}

void MainWindow:: OpenClose_Degrader(bool state){

    emit RunDegrader(state, ui->comboBox_Degrader->currentText().split(':')[0]);


}
*/

void MainWindow:: SetupScanRange()
{
    if (ui->AxisX_Button->isChecked())
    {
            ui->spinBox_ScanMax->setMaximum(AXIS_RANGEX);
            ui->spinBox_ScanMin->setMaximum(AXIS_RANGEX-ui->spinBox_Step->value()-1);
            ui->spinBox_FixPosition->setMaximum(AXIS_RANGEY);
    }
    else
    {
            ui->spinBox_ScanMax->setMaximum(AXIS_RANGEY);
            ui->spinBox_ScanMin->setMaximum(AXIS_RANGEY-ui->spinBox_Step->value()-1);
             ui->spinBox_FixPosition->setMaximum(AXIS_RANGEX);
    }
   // qDebug("ScanRangesWasChanged");

}

void MainWindow:: SetupCalibration()
 {
    ui->label_calibration_state->setStyleSheet("QLabel { background-color : Green; font-size: 11pt;font-weight: bold;}");
    ui->label_calibration_state->setText("CALIBRATION ON");
    qDebug()<<"Let`s get started calibratoiJK!";
    emit StartCalibration();
 }

void MainWindow:: BeamScanState(void)
{


    ScanState=0;
    ui->label_ScaningState->setStyleSheet("QLabel { font-size: 11pt;font-weight: bold;}");
     ui->label_ScaningState->setAlignment(Qt::AlignCenter);
        ui->label_ScaningState->setText("Scanning stopped");

}
void MainWindow:: BeamScanSetup(void){

    if (!PositionErrorState)

    {
            ui->Scan_area->graph(0)->clearData();
            ui->Scan_area->graph(1)->clearData();
            ui->Scan_area->graph(1)->setAntialiased(true);
            limit_range_random=20; //aid 6Feb_2020
            ScanArea_Max=1e-15; //AID!!!!!!!!
           // ScanArea_Max=0; //AID!!!!!!!!

            ui->label_ScaningState->setStyleSheet("QLabel { font-size: 11pt;font-weight: bold;}");
            ui->label_ScaningState->setAlignment(Qt::AlignCenter);
            ui->label_ScaningState->setText("On Going");
            ScanState=1;
            emit BeamScanRun(ui->AxisX_Button->isChecked(),ui->spinBox_ScanMin->value(),ui->spinBox_ScanMax->value(),ui->spinBox_Step->value(),ui->spinBox_FixPosition->value());
            //  ui->AxisX_Button->isChecked(),ui->spinBox_ScanMin->value(),ui->spinBox_ScanMax->value(),ui->spinBox_Step->value());
        }

}
void MainWindow:: SaveFit(){



    //QString FileName="/"+QString::number(timestamp_begin)+"/FIT_"+QString::number(QDateTime::currentDateTime().toTime_t());
    QString FileName=DirName+"\\FIT_"+QString::number(QDateTime::currentDateTime().toTime_t());

     ui->Scan_area->savePng(FileName+".png");

     FileName+=".txt";



     QFile file( FileName );
     if ( file.open(QIODevice::ReadWrite) )
     {
         QTextStream stream( &file );
         stream <<"Amplitude; Mean; Sigma; Scale \r" << endl;
         stream << expected_amp<<"; "<<expected_mean<<"; "<<expected_sigma<<"; "<<GausScale << endl;
     }


     //FILE *FitScan;
   // FitScan=fopen(FileName.toLatin1(),"w");

    //if (FitScan==NULL)
      // WriteDebugMSG("error in opening file");

    //fprintf(FitScan,"%f %f %f %f",expected_amp,expected_mean,expected_sigma,GausScale);

  //  fclose(FitScan);
  //  qDebug("I`ve saved Fit");
}

void MainWindow:: PortConnectionError(void)
{

    ui->label_error_state->setStyleSheet("QLabel { background-color : yellow;}");
    ui->label_error_state->setAlignment(Qt::AlignCenter);
    ui->label_error_state->setText("Error in Connection");

}

void MainWindow:: LimitSwitchError(void)
{   
    ui->label_error_state_2->setStyleSheet("QLabel { background-color : yellow;}");
    ui->label_error_state_2->setAlignment(Qt::AlignCenter);
    ui->label_error_state_2->setText("Limit Switch Error");
}

void MainWindow:: MotorMovingError(bool state)
{
    if (state)
    {
        ui->label_error_state_3->setStyleSheet("QLabel { background-color : yellow;}");
        ui->label_error_state_3->setAlignment(Qt::AlignCenter);
        ui->label_error_state_3->setText("Motor MOVING");

    }
    else
    {
        ui->label_error_state_3->setStyleSheet("QLabel { background-color : none;}");
        ui->label_error_state_3->setAlignment(Qt::AlignCenter);
        ui->label_error_state_3->setText("Motor STOPPED");

    }



  }




void MainWindow::closeEvent(QCloseEvent* event)
{
        if  ( timer->isActive() )
        {
          QMessageBox::warning(0,"Warning", "Close all operations!");
          event->ignore();
        }
        else
             event->accept();
}

void MainWindow:: Label_device_state (QSerialPort* Port,bool state){
    if (state)


    {


        if (MCL_port_name==Port->portName())
                {
                    ui->label_MCL_state->setStyleSheet("QLabel { background-color : yellow; font-size: 11pt;font-weight: bold;}");
                    ui->label_MCL_state->setText("MCL is READY");
                    emit NanotechStartSettings();

                 }
        else if ( UNIDOS_port_name==Port->portName())
        {
            ui->label_UNIDOS_state->setStyleSheet("QLabel { background-color : yellow; font-size: 11pt;font-weight: bold;}");
            ui->label_UNIDOS_state->setText("UNIDOS is READY");

         }

        else  WriteDebugMSG("Error in Label_device_state");

    }

    else {

        if (MCL_port_name==Port->portName())
                {
                    ui->label_MCL_state->setStyleSheet("QLabel { background-color : red;font-size: 12pt;font-weight: bold;}");
                    ui->label_MCL_state->setText("MCL is NOT READY");
                 }

        else if (UNIDOS_port_name==Port->portName())
        {
            ui->label_UNIDOS_state->setStyleSheet("QLabel { background-color : red; font-size: 11pt;font-weight: bold;}");
            ui->label_UNIDOS_state->setText("UNIDOS is NOT READY");

         }
        else  WriteDebugMSG("Error in Label_device_state");
    }

}


void MainWindow:: CalcBeamParams (void){


    int konfigurace =  (int) (ui->checkBox_Degrader_05->isChecked())*2 +  (int) (ui->checkBox_Degrader_1->isChecked()) *4 + (int) (ui->checkBox_Degrader_2->isChecked()) *8;

  //qDebug("konfigurace is %d",konfigurace);

      if(konfigurace == 0 ){  //no plate
           CalibrationConstant = 15873; //cm-2 s-1 pA-1
           pEnergyAtSample = 32.3f;
           LET=0.013932

;

         }else if(konfigurace == 2 ){  //0.5 mm
           CalibrationConstant = 16011.; //cm-2 s-1 pA-1
           pEnergyAtSample = 30.2f;
           LET=0.014688;

         }else if(konfigurace == 4 ){  //1 mm
           CalibrationConstant = 15189.; //cm-2 s-1 pA-1
           pEnergyAtSample = 28.1f;
           LET=0.0155504;

         }else if(konfigurace == 6 ){  //0.5+1 mm
           CalibrationConstant = 13964.; //cm-2 s-1 pA-1
           pEnergyAtSample = 25.8f;
           LET=0.0166364;

         }else if(konfigurace == 8 ){  //2 mm
           CalibrationConstant = 12971.; //cm-2 s-1 pA-1
           pEnergyAtSample = 24.0f;
           LET=0.017618;

         }else if(konfigurace == 10 ){  //0.5+2 mm
           CalibrationConstant = 11463.; //cm-2 s-1 pA-1
           pEnergyAtSample = 21.4f;
           LET=0.0192876;

         }else if(konfigurace == 12 ){  //1+2 mm
           CalibrationConstant = 10324.; //cm-2 s-1 pA-1
           pEnergyAtSample = 18.5f;
           LET=0.0216025;

         }else if(konfigurace == 14 ){  //0.5+1+2 mm
           CalibrationConstant = 8336.; //cm-2 s-1 pA-1
           pEnergyAtSample = 15.1f;
           LET=0.025295;



  }
      //LET= Linear_interplotation(pEnergyAtSample);
      //LET = 0.018;
      ui->Info_label_1->setText("k [pA^-1 cm^-2 s^-1]=    " + QString::number(CalibrationConstant) +  "\n E of p hitting sample [MeV]=   " + QString::number(pEnergyAtSample) +   " \n LET in Si [MeV/ (mg/cm2)]=    ");
      ui->doubleSpinBox_LET->setValue(LET);

}


float MainWindow:: Linear_interplotation(double E)
{
    const int size =25;
     double x[size] = {5,5.5,6,6.5,7,8,9,10,11,12,13,14,15,16,17,18,20,22.5,25,27.5,30,32.5,35,37.5,40};
     double y[size] = {5.866E-02,5.466E-02,5.124E-02,4.826E-02,4.564E-02,4.125E-02,3.771E-02,3.479E-02,3.233E-02,3.023E-02,2.841E-02,2.682E-02,2.542E-02,2.417E-02,2.305E-02,2.204E-02,2.029E-02,1.850E-02,1.703E-02,1.580E-02,1.476E-02,1.386E-02,1.307E-02,1.238E-02,1.177E-02};

            double l,L=0;



        for (int i=0;i<size;i++)
        {
            if (E>x[i])
                    L=y[i]+(y[i]-y[i+1])/(x[i]-x[i+1])*(E-x[i]);


        }

return L;
}

 void MainWindow:: Show_Nanotec_state(bool ReadyX,bool ZeroX,bool PErrorX,bool ReadyY,bool ZeroY,bool PErrorY)
 {


     ui->checkBox_ReadyX->setChecked(ReadyX);
     ui->checkBox_ReadyY->setChecked(ReadyY);
     ui->checkBox_ZeroX->setChecked(ZeroX);
     ui->checkBox_ZeroY->setChecked(ZeroY);
     ui->checkBox_ErrorX->setChecked(PErrorX);
     ui->checkBox_ErrorY->setChecked(PErrorY);

    PositionErrorState=PErrorX |  PErrorY;
    if (PositionErrorState==0)
    {
        ui->label_error_state_2->setStyleSheet("QLabel { background-color : none;}");
        ui->label_error_state_2->setText("Limit Switch OK");

    }

    qDebug()<<"Test:" <<PositionErrorState;

 }



void  MainWindow:: Start_timer(bool state){

  //  qDebug("Button is toggled");
    if (state){
        ui->BeamMonitoring_button->setText("STOP Beam Monitoring");
        flux=0;
        fluence = 0;
        total_fluence = 0;
        DoseInSilicone =0;
        ui->Monitoring_area->graph(0)->clearData();

        QString FileName=DirName+ "\\LOG_"+QString::number(QDateTime::currentDateTime().toTime_t())+".txt";
        // std:: string FileName= "LOG_"+QDateTime::currentDateTime().toTime_t()+".txt";
        qDebug()<<" I will open file: "<<FileName;
         log=fopen(FileName.toLatin1(),"w");
         if (log == NULL)  WriteDebugMSG("Error in opening LOG file");


        this->timestamp_begin=QDateTime::currentDateTime().toTime_t();


           timer->start(ui->spinBox_Period->value()*1000);

         ui->label_monitoring_state->setStyleSheet("QLabel { background-color : yellow; font-size: 11pt;font-weight: bold;}");
         ui->label_monitoring_state->setText("MONITORING ON");
         qDebug("Button is toggled true");


    }
    else
    {
        qDebug("Button is toggled False");
         fclose(log);
     ui->BeamMonitoring_button->setText("Start Beam Monitoring");
       timer->stop();
        MonitoringArea_Max=1e-15;

       ui->BeamReachDose_button->setChecked(false);
       ui->BeamIntegrating_button->setChecked(false);


       ui->label_monitoring_state->setStyleSheet("QLabel { background-color : red; font-size: 11pt;font-weight: bold;}");
       ui->label_monitoring_state->setText("MONITORING OFF");
       emit Start_refresh(false);

    }
}

void  MainWindow:: Start_refresh(bool state){

  //qDebug("Button refresh is toggled");
    if (state){
        ui->BeamRefresh_button->setText("START Refreshing");
        RefreshState=false;

    }
    else
    {
        ui->BeamRefresh_button->setText("PAUSE Refreshing");
        RefreshState=true;
    }
}


void  MainWindow:: Start_integration(bool state){

  //qDebug("Button refresh is toggled");
    if (!state){
        ui->BeamIntegrating_button->setText("START Integration");
        RefreshState=false;

    }
    else
    {
        ui->BeamIntegrating_button->setText("STOP Integration");
        RefreshState=true;
    }
}
void  MainWindow:: Start_ReachDose(bool state){

  //qDebug("Button refresh is toggled");
    if (!state){
        ui->BeamReachDose_button->setText("Reach target dose ON");

    }
    else
    {
        ui->BeamReachDose_button->setText("Reach target dose OFF");
        AbsorbedDose=0;

    }
}

void MainWindow:: WriteLogFile(){


    if (!ScanState)

{
        emit RunUNIDOS();

        QTimer * WaitTimer = new QTimer(this);
        WaitTimer->setSingleShot(true);
        WaitTimer->start(1000);

        while (!CurrentUpdated)
        {
              if (WaitTimer->remainingTime() ==0)
                {
                  WriteDebugMSG("Too long wait UNIDOS \n");
                  return;
                }

        }
        delete(WaitTimer);

}








CurrentUpdated=0;
//qDebug("Draw Monitoring area \n");
Draw_Monitoring_area();

    timer->start(ui->spinBox_Period->value()*1000);

    // Draw_MCL_area(X_max,Y_max, X_current,Y_current);



    //emit RunMCL();
    float avr_current=0, avr_flux=0;

    //qDebug("Number of graphs in monitoring area %d",ui->Monitoring_area->selectedGraphs().count());
    //if(ui->Monitoring_area->selectedGraphs().count() != 0 )
     const QCPDataMap *dataMap = ui->Monitoring_area->graph(0)->data();
    Draw_Monitoring_area();
    // qDebug("Size of graph in monitoring aread %d",dataMap->size());
    ScaleCoeficient=ui->ScaleSpinBox->value();



 //Calculation of the average values of current and flux
 flux= ScaleCoeficient* CalibrationConstant*(1e12)* Current.toFloat()* (int)(!ui->checkBox_Degrader_abs2->isChecked());
if (flux<0) flux=0;

     if (dataMap->size() > ui->spinBox_Avr_range->value())
     {
         avr_current=0;
         avr_flux=0;
             QMap<double, QCPData>::const_iterator i = dataMap->constEnd();
        i--;
          //   qDebug("------------------");
             for (int j=0;j<ui->spinBox_Avr_range->value();j++,i--)
             {

               avr_current+=i.value().value*(1e12);
               avr_flux+=ScaleCoeficient* CalibrationConstant*(1e12)* i.value().value*(int)(!ui->checkBox_Degrader_abs2->isChecked());


              }
    }

   avr_current/=ui->spinBox_Avr_range->value();
   avr_flux/=ui->spinBox_Avr_range->value();
   fluence = flux*ui->spinBox_Period->value();





if (ui->BeamIntegrating_button->isChecked())
{
    total_fluence += fluence;
    DoseInSilicone += 1.602e-8 * ui->doubleSpinBox_LET->value() * fluence;
 }
else
{
    total_fluence = 0;
    DoseInSilicone = 0;
}


if (ui->BeamReachDose_button->isChecked())
{
    AbsorbedDose+=1.602e-8 * ui->doubleSpinBox_LET->value() * fluence;

}

if (AbsorbedDose>ui->spinBox_TargetDose->value())
{
    ui->BeamMonitoring_button->setChecked(false);
    ui->checkBox_Degrader_abs1->setChecked(true);
}






    ui->label_Avr_current->setText("Average chamber current  [pA] per " +QString::number(ui->spinBox_Period->value()*ui->spinBox_Avr_range->value()) + "s  "+ QString::number(avr_current) );
    ui->label_Avr_flux->setText("Average proton flux [cm^-2*s^-1] per " +QString::number(ui->spinBox_Period->value()*ui->spinBox_Avr_range->value()) + "s  "+ QString::number(avr_flux));
    ui->label_Int_dose->setText("Integrated dose in silicon  [krad]  " + QString::number(DoseInSilicone));
    ui->label_Fluence->setText("Fluence  [cm^-2]  "+ QString::number(total_fluence));
    ui->label_Sil_dose->setText("Dose in silicon  [krad]  "+ QString::number(AbsorbedDose ));




uint timestamp=QDateTime::currentDateTime().toTime_t();
    fprintf(log,"%d %d %d %e %e %e %e %e %e %e %d %d %d %d %d\n",
    timestamp,X_current, Y_current,
    Current.toFloat(), flux, total_fluence, DoseInSilicone, ScaleCoeficient, CalibrationConstant, ui->doubleSpinBox_LET->value(),ui->checkBox_Degrader_abs1->isChecked(),
    ui->checkBox_Degrader_05->isChecked(),ui->checkBox_Degrader_1->isChecked(),ui->checkBox_Degrader_2->isChecked(),ui->checkBox_Degrader_abs2->isChecked() );
}





void MainWindow:: showDataMessage(QByteArray &data){

  //  ui->line_COM2_2->setText(QString::number( myTimer.elapsed()));
    //ui->line_COM2->setText(data);

}
void MainWindow:: CalibrationProceed(bool state)
{

qDebug()<<"Im in calibration Proceed";

    if (state)
    {
        ui->label_calibration_state->setStyleSheet("QLabel { background-color : yellow; font-size: 11pt;font-weight: bold;}");
        ui->label_calibration_state->setText("CALIBRATION DONE");

        ui->pushButton_CheckPosition->setDisabled(false);
        ui->Move_button->setDisabled(false);
        ui->Move_button2->setDisabled(false);
        ui->Here_button->setDisabled(false);
    }
    else
    {
        ui->label_calibration_state->setStyleSheet("QLabel { background-color : red; font-size: 11pt;font-weight: bold;}");
        ui->label_calibration_state->setText("CALIBRATION Error");

    }

}

void MainWindow:: WriteCurrentCoordinate(int x,int y){

  //  qDebug("Coordinate is updated");

   ui->label_error_state->setStyleSheet("QLabel { background-color : none;}");
   ui->label_error_state->setText("Connection OK");

  // ui->label_error_state_2->setStyleSheet("QLabel { background-color : none;}");
 //  ui->label_error_state_2->setText("Limit Switch OK");

    //ui->label_motor_state->setStyleSheet("QLabel { background-color : none;}");
    //ui->label_motor_state->setText("Motor is stoped");
    X_current=x;
    Y_current=y;
    ui->Current_X->setText(QString::number(X_current));
    ui->Current_Y->setText(QString::number(Y_current));
    Draw_MCL_area(X_current,Y_current);
    CoordinateUpdated=true; //needs for SCANNING

  //  MotorMovingError(false); //SET label "Motor is STOPPED"

     emit Main_CoordinateUpdated();
}


void MainWindow:: WriteCurrent(QByteArray& data, int stat){

 //   qDebug("Current is updated");
  //  Current = "5e-8";
    if (stat==1)
            {
                CurrentUpdated=true;
                 Current = data;
             }

     emit Current_CoordinateUpdated(stat);

}

//--------------------------Read Coordinates from spinBOX
// y-axis is negative because motor has another direction
void MainWindow:: ReadNEWCoordinate_Here(void){


 if (!PositionErrorState)
 {

   // MotorMovingError(true);
    ui->Here_button->setText("Here");
    ui->Move_button->setText("Move");
    ui->Move_button2->setText("Move");

    emit MoveTo(ui->spinBox_HereX->value()*STEP_CONST,-ui->spinBox_HereY->value()*STEP_CONST);

}

 else
 {
        QMessageBox::warning(0,"Position Error", "Position Error was detected! \n Please Clear Error");
 }


    // qDebug("SPIN BOX Here x= %d y=%d \n", (ui->spinBox_HereX->value()),(ui->spinBox_HereY->value()));

}
void MainWindow:: ReadNEWCoordinate_Move(void){


    if (!PositionErrorState)
    {

        //MotorMovingError(true);
        emit MoveTo(ui->spinBox_MoveX->value()*STEP_CONST,-ui->spinBox_MoveY->value()*STEP_CONST);
        ui->Here_button->setText("Move");
        ui->Move_button->setText("Here");
        ui->Move_button2->setText("Move");
   }

    else
    {
           QMessageBox::warning(0,"Position Error", "Position Error was detected! \n Please Clear Error");
    }

}
void MainWindow:: ReadNEWCoordinate_Move2(void){

if (!PositionErrorState)
    {
      //  MotorMovingError(true);
        ui->Here_button->setText("Move");
        ui->Move_button->setText("Move");
        ui->Move_button2->setText("Here");
         emit MoveTo(ui->spinBox_MoveX_2->value()*STEP_CONST,-ui->spinBox_MoveY_2->value()*STEP_CONST);

   }

    else
    {
           QMessageBox::warning(0,"Position Error", "Position Error was detected! \n Please Clear Error");
    }

}

//--------------------------------------------------------------------
void MainWindow:: Draw_MCL_area(int X_current, int Y_current){


    double h = 1;

   int N_x= AXIS_RANGEX*1.1 /h + 1; //Вычисляем количество точек, которые будем отрисовывать
   int N_y=AXIS_RANGEX*1.1 /h + 1;
   QVector<double> x_curr(1),y_curr(1);
   x_curr[0]=X_current;
   y_curr[0]=Y_current;

 //  qDebug("Current X is %d",X_current);



   int i=0;





   ui->MCL_area->addGraph();
   ui->MCL_area->graph(0)->setData(x_curr, y_curr);

   ui->MCL_area->graph(0)->setPen(QColor(50, 50, 50, 255));//задаем цвет точки

   ui->MCL_area->graph(0)->setLineStyle(QCPGraph::lsNone);//убираем линии
   ui->MCL_area->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus , 20));


   //Подписываем оси Ox и Oy
   ui->MCL_area->xAxis->setLabel("Coordinate X [mm]");
   ui->MCL_area->yAxis->setLabel("Coordinate Y [mm]");



   //Установим область, которая будет показываться на графике
 //  if (X_edge-5<abs(X_current)) X_edge=abs(X_current)+5;
 //  if (Y_edge-5<abs(Y_current)) Y_edge=abs(Y_current)+5;

   ui->MCL_area->xAxis->setRange(-AXIS_RANGEX*0.05 , AXIS_RANGEX*1.05);//Для оси Ox
   ui->MCL_area->xAxis->setAutoTickCount(10);


   ui->MCL_area->yAxis->setRange(-AXIS_RANGEY*0.05 , AXIS_RANGEY*1.05);//Для оси Oy
   ui->MCL_area->yAxis->setAutoTickCount(10);

   ui->MCL_area->xAxis2->setVisible(true);
   ui->MCL_area->xAxis2->setTickLabels(false);
   ui->MCL_area->xAxis2->setRange(-AXIS_RANGEX*0.05 , AXIS_RANGEX*1.05);//Для оси Ox
   ui->MCL_area->xAxis2->setAutoTickCount(10);

   ui->MCL_area->yAxis2->setVisible(true);
   ui->MCL_area->yAxis2->setTickLabels(false);
   ui->MCL_area->yAxis2->setRange(-AXIS_RANGEY*0.05 , AXIS_RANGEY*1.05);//Для оси Oy
   ui->MCL_area->yAxis2->setAutoTickCount(10);


   ui->MCL_area->replot();



}

void MainWindow:: Draw_Scan_area()
{

  //  qDebug("Start Drawing Scan area");
    ui->Scan_area->setInteraction(QCP::iRangeZoom,true);
    ui->Scan_area->axisRect()->setRangeZoom(Qt::Vertical);

    ui->Scan_area->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->Scan_area->yAxis->setTickLabelFont(QFont(QFont().family(), 8));


    ui->Scan_area->xAxis2->setVisible(true);
    ui->Scan_area->yAxis2->setVisible(true);
    ui->Scan_area->xAxis2->setTicks(true);
    ui->Scan_area->yAxis2->setTicks(true);
    ui->Scan_area->xAxis2->setTickLabels(false);
    ui->Scan_area->yAxis2->setTickLabels(false);

    ui->Scan_area->yAxis->setTickLabelColor(QColor(Qt::red)); // Красный цвет подписей тиков по Оси Y




   ui->Scan_area->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc , 8));
   ui->Scan_area->graph(0)->setPen(QPen(QColor(Qt::red))); // Устанавливаем цвет графика
   ui->Scan_area->graph(0)->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
   ui->Scan_area->graph(0)->setLineStyle(QCPGraph::lsNone);


   if (ScanArea_Max<Current.toDouble())    ScanArea_Max=Current.toDouble();

   ui->Scan_area->xAxis->setRange(ui->spinBox_ScanMin->value(), ui->spinBox_ScanMax->value());

/*
 if (ui->AxisX_Button->isChecked())
      // ui->Scan_area->graph(0)->addData(X_current, Current.toFloat());
     ui->Scan_area->graph(0)->addData(X_current, Current.toDouble());
 else
     ui->Scan_area->graph(0)->addData(Y_current,  Current.toDouble());

*/


  // ui->Scan_area->graph(0)->clearData();
  // float Graph_Coordinate[18]= {160,		165	,		175		,	180	,	185,		190,		195	,	200,		205	,	210,		215	,	220,		225		,230,		235	,	240,		245,		250};
  // float Graph_Current[18]= { 4.410000e-013,		7.010000e-013,			1.010000e-012	,		1.981000e-012,		2.629000e-012	,	3.265000e-012	,	3.886000e-012	,	4.433000e-012	,	4.660000e-012	,	4.733000e-012	,	4.501000e-012	,	3.973000e-012	,	3.507000e-012	,	2.832000e-012	,	2.097000e-012	,	1.576000e-012	,	1.127000e-012,		7.970000e-013};

  // float Graph_Current[21]={1.090000e-009,	1.260000e-009,	1.370000e-009	,1.490000e-009,	1.570000e-009,	1.730000e-009,	1.800000e-009,	1.930000e-009,	2.050000e-009,	2.120000e-009,	2.180000e-009,	2.170000e-009	,2.300000e-009,	2.160000e-009,	2.210000e-009,	2.180000e-009,	2.090000e-009,	2.010000e-009,	1.830000e-009,	1.740000e-009	,1.620000e-009};
  //float Graph_Current[21]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//float Graph_Coordinate[9]={150,	155, 160,165,170,175,180,185,190};
//float Graph_Current[9]={6.468000e-010	,		8.341000e-010	,		9.094000e-010,			8.345000e-010		,	6.340000e-010	,		3.874000e-010	,		2.003000e-010};
 // float Graph_Coordinate[21]={0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100};
   //float Graph_Coordinate[11]={0,3,6,9,12,15,18,21,24,27,30};
/*
  for (int i=0;i<9;i++)
  {
       //  ui->Scan_area->graph(0)->addData(Graph_Coordinate[i], Graph_Current[i]*pow(10,9));
       // if (ScanArea_Max<Graph_Current[i]*pow(10,9))    ScanArea_Max=Graph_Current[i]*pow(10,9);
       ui->Scan_area->graph(0)->addData(Graph_Coordinate[i], Graph_Current[i]);
      if (ScanArea_Max<Graph_Current[i])    ScanArea_Max=Graph_Current[i];
  }
*/




   // X_current++;
//------------------------------------------------------------COrrect


   //antialiased doesn't have sense, main idea to detect, that graph is filled (to avoid additional last point)
if (ui->Scan_area->graph(1)->antialiased())
{
   if (ui->AxisX_Button->isChecked())
       ui->Scan_area->graph(0)->addData(X_current, Current.toDouble());
   else
       ui->Scan_area->graph(0)->addData(Y_current,  Current.toDouble());
  }
//-----------------------------------------------------------
  //ui->Scan_area->graph(0)->addData(X_current, Current.toDouble());



//  qDebug("SCAN Float mean of current is %e",Current.toDouble());

    //ui->Scan_area->yAxis->rescale(true);
  ui->Scan_area->yAxis->setRange(-ScanArea_Max*0.5,ScanArea_Max*1.1);
    ui->Scan_area->yAxis->setLabel("Current [A]");
    ui->Scan_area->yAxis->setLabel("Current [A]");
    ui->Scan_area->xAxis->setLabel("Coordinate [mm]");


    ui->Scan_area->replot();           // Отрисовываем график

   // qDebug()<<"Scan area is reploted"<<ScanArea_Max;


}

void MainWindow:: Draw_Monitoring_area(){



         //ui->Monitoring_area->setInteraction(QCP::iRangeZoom,true);   // Включаем взаимодействие удаления/приближения
         ui->Monitoring_area->setInteraction(QCP::iRangeDrag, true);  // Включаем взаимодействие перетаскивания графика
         ui->Monitoring_area->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
         //ui->Monitoring_area->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси
        ui->Monitoring_area->setInteraction(QCP::iRangeZoom,true);
         ui->Monitoring_area->axisRect()->setRangeZoom(Qt::Vertical);


        ui->Monitoring_area->xAxis->setTickLabelType(QCPAxis::ltDateTime);   // Подпись координат по Оси X в качестве Даты и Времени
         ui->Monitoring_area->xAxis->setDateTimeFormat("mm:ss");         // Устанавливаем формат даты и времени

        // Настраиваем шрифт по осям координат
         ui->Monitoring_area->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
         ui->Monitoring_area->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

        // Автоматическое масштабирование тиков по Оси X
       //  ui->Monitoring_area->xAxis->setAutoTickStep(true);

        /* Делаем видимыми оси X и Y по верхней и правой границам графика,
         * но отключаем на них тики и подписи координат
         * */
         ui->Monitoring_area->xAxis2->setVisible(true);
         ui->Monitoring_area->yAxis2->setVisible(true);
         ui->Monitoring_area->xAxis2->setTicks(false);
         ui->Monitoring_area->yAxis2->setTicks(false);
         ui->Monitoring_area->xAxis2->setTickLabels(false);
         ui->Monitoring_area->yAxis2->setTickLabels(false);
         ui->Monitoring_area->yAxis->setLabel("Current  [pA]");
         ui->Monitoring_area->xAxis->setLabel("Time [sec]");

        ui->Monitoring_area->yAxis->setTickLabelColor(QColor(Qt::red)); // Красный цвет подписей тиков по Оси Y
        ui->Monitoring_area->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc , 8));



        ui->Monitoring_area->graph(0)->setPen(QPen(QColor(Qt::red))); // Устанавливаем цвет графика
        ui->Monitoring_area->graph(0)->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено

        if (MonitoringArea_Max<Current.toDouble())    MonitoringArea_Max=Current.toDouble();


       uint key = QDateTime::currentDateTime().toTime_t()-this->timestamp_begin;


if (ui->checkBox_Constrain->isChecked())
       ui->Monitoring_area->xAxis->setRange(key-ui->spinBox_AxisRange->value()*60, key);
else
     ui->Monitoring_area->xAxis->setRange(0, key);


const QCPDataMap *dataMap= ui->Monitoring_area->graph(0)->data();

QMap<double,QCPData>::const_iterator lower = dataMap->lowerBound(ui->Monitoring_area->xAxis->range().lower);
QMap<double,QCPData>::const_iterator upper = dataMap->upperBound(ui->Monitoring_area->xAxis->range().upper);


//TODO: error checking

double dHigh=lower.value().value;
double dLow=lower.value().value;
qDebug()<<"AT THE BEGINING Low is  "<<dLow<<" high is "<<dHigh;

/*
while (lower != upper)
{
    if (lower.value().value > dHigh) dHigh = lower.value().value;
    if (lower.value().value < dLow) dLow = lower.value().value;
    lower++;
}
*/
while (lower != upper)
{
    if (lower.value().value > dHigh) dHigh = lower.value().value;
    if (lower.value().value < dLow) dLow = lower.value().value;
    lower++;
}

ui->Monitoring_area->yAxis->setRange(dLow*0.95, dHigh*1.05);
qDebug()<<"Low is  "<<dLow<<" high is "<<dHigh;



   //  ui->Monitoring_area->yAxis->setRange(0-MonitoringArea_Max*0.05, MonitoringArea_Max*1.1);
 // ui->Monitoring_area->yAxis->rescale(false);
//  ui->Monitoring_area->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));


       qDebug("Float mean of current is %e",Current.toDouble());



       ui->Monitoring_area->graph(0)->addData(key, Current.toDouble());
       //ui->Monitoring_area->yAxis->rescale(true);
      // ui->Monitoring_area->xAxis->rescale(true);





     //  ui->Monitoring_area->graph(0)->addData(key, key);

        // Заполняем график значениями
/*
income[0]=0;
time[0]=0;
        for (int i=1; i<400; i++)
          {
            time[i] = now + 3600*i;
            income[i] = qFabs(income[i-1]) + (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
          }

        ui->Monitoring_area->graph(0)->setData(time, income); // Устанавливаем данные
*/


//rescaleAxes();      // Масштабируем график по данным
if (RefreshState) ui->Monitoring_area->replot();           // Отрисовываем график

}



MainWindow::~MainWindow()
{

     qDebug("MainWindow is closed");

     delete ui;
     delete COM_MCL;
     delete COM_DEGRADER;
     delete COM_UNIDOS;
     delete It;

}

//----------------------------------------------------------------------------



bool Port::  ComandTransferForMCL( QByteArray comand, QString msg){


 if (thisPort->isOpen())
 {
                  QByteArray ReadData;
                  emit NewDebugMSG("Send command " + msg + ": "+comand);
                  qDebug()<<"Send command "<<msg <<": "<<comand;
                         thisPort->write(comand);




                if (thisPort->waitForReadyRead(500))
                 {
                     qDebug("Read command ");
                    ReadData=thisPort->readAll();
                     qDebug()<<ReadData;
                  }
                 else
                 {
                     qDebug()<<"Too long answer "<<msg;

                     emit NewDebugMSG("Too long answer "+msg+"\r");
                     emit ConnectionError();
                     return false;
                 }

                 if  (ReadData.contains(comand.remove(0,1)) || comand.contains("$")) //if we are trying to check state of controller
                            {
                                       qDebug("OK");
                            }
                  else
                            {
                                        qDebug("Error not  ");
                                        emit ConnectionError();
                                        return false;
                            }
}
 else
 {
     qDebug("Port is Closed");
     emit ConnectionError();
     return false;
 }

 return true;
}




void Port:: SetNanotech(void){

    QByteArray SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append(":crc0\t3F\r");

    ComandTransferForMCL(SendComand, "turn of CRC");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("p2\r");

    ComandTransferForMCL(SendComand, "Set positioning modeY");


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("p2\r");

    ComandTransferForMCL(SendComand, "Set positioning modeX");



    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("g2\r");

    ComandTransferForMCL(SendComand, "Set step modeY" );


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("g2\r");

    ComandTransferForMCL(SendComand, "Set step modeX");

    //Check Continuation record #NUM_MOTORZN\r

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("l9226\r"); //(10  0100 0000 1010) check p. 23 of manual

    ComandTransferForMCL(SendComand, "Set limit switch modeX");


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("l9226\r");

    ComandTransferForMCL(SendComand, "Set limit switch modeY");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("J1\r");

    ComandTransferForMCL(SendComand, "Set auto status X");


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("J1\r");

    ComandTransferForMCL(SendComand, "Set auto status Y");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append(":accel50000\r");

    ComandTransferForMCL(SendComand, "Set accel ramp Y");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append(":accel50000\r");

    ComandTransferForMCL(SendComand, "Set accel ramp X");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("u400\r");

    ComandTransferForMCL(SendComand, "Set min frequency Y");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("u400\r");

    ComandTransferForMCL(SendComand, "Set min frequency X");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("o1000\r");

    ComandTransferForMCL(SendComand, "Set max frequency Y");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("o1000\r");

    ComandTransferForMCL(SendComand, "Set max frequency X");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append(":port_in_d7\r");

    ComandTransferForMCL(SendComand, "Set 4 input as limit switch X");

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append(":port_in_d7\r");

    ComandTransferForMCL(SendComand, "Set 4 input as limit switch Y");


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("h17236023\r");

    ComandTransferForMCL(SendComand, "Set 4 input as reversed X");


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("h17236023\r");

    ComandTransferForMCL(SendComand, "Set 4 input as reversed Y");


    emit Check_Nanotec_state();

    //need also set digital outpus parameter


}


int Port:: CheckCoordinate(int MOTOR_ID)
{

    if (thisPort->isOpen())
    {
    QByteArray SendComand="#",ReadData;



    SendComand.append(QByteArray::number(MOTOR_ID));
    SendComand.append("C\r");
    qDebug()<<"Send command to check Position "<<MOTOR_ID;

    thisPort->write(SendComand);

  if (thisPort->waitForReadyRead(500))
        {
                    ReadData=thisPort->readAll();
                    qDebug()<<"Read command "<<ReadData;

       }
   else
   {
                    qDebug()<<"Too long answer";
                    emit ConnectionError();
                    return false;
   }
        int coordinate =ReadData.remove(ReadData.size()-1,2).remove(0,2).toInt()/STEP_CONST;
        qDebug()<<"Return coordinate"<<coordinate;
        return coordinate;
    }
    else
        emit NewDebugMSG("MCL is closed you can't check postion!");

}

Port::Port(QObject *parent,QString Newname,qint32 NewBaud, QSerialPort::DataBits NewDataBits, QSerialPort::Parity NewParity,  QSerialPort::StopBits NewStopBits) :  QObject(parent)
{
    thisPort= new QSerialPort();

    Current_settings.name = Newname;
   Current_settings.baudRate=NewBaud;
    Current_settings.dataBits=NewDataBits;
    Current_settings.parity=NewParity;
    Current_settings.stopBits=NewStopBits;




}

void Port::OpenClose_SerialPort(bool state, QString Portname)
{


   if (state)
    {


       thisPort->setPortName(Portname);
       thisPort->setBaudRate(Current_settings.baudRate);
       thisPort->setDataBits( Current_settings.dataBits);
       thisPort->setParity(Current_settings.parity);
       thisPort->setStopBits( Current_settings.stopBits );

                if (thisPort->open(QIODevice::ReadWrite))
                 {

                    qDebug()<<("Port ")<<thisPort->portName()<<(" is connected \n");

                    emit Port_state_changed(thisPort,TRUE);
                }
                    else

                {
                     qDebug()<<("Error connection in Port ")<<thisPort->portName();
                     emit ErrorInOpenPort(Portname);

                  }


      }
   else
   {

       if(thisPort->isOpen())  {
                                thisPort->close();
                                emit Port_state_changed(thisPort,FALSE);
                                 qDebug()<<("Port ")<<thisPort->portName()<<(" is Closed \n");
                                }

   }

}


void Port :: process_Port(){//Выполняется при старте класса
  qDebug("Hello world in %s Port \n", this->name.toLatin1().constData());

}

void  Port:: Check_Degrader_State(void){

    QByteArray ReadData;
    if (thisPort->isOpen()){

            qDebug("Asking for abs1");

            //emit WriteToPort("*B1IR1\r"); //request of the state of the degrader
             thisPort->write(QByteArray("*B1IR1\r", 8));

            if (thisPort->waitForReadyRead(1000))
            {
                qDebug("Read command ");
                ReadData=thisPort->readLine();
                qDebug()<<ReadData;
             }
            else
            {
                qDebug("Too long answer in Degrader");
            }



             if  (ReadData.contains("*B10H"))
                  emit Set_check_box_abs1(true);
             else
                   if (ReadData.contains("*B10L"))
                        emit Set_check_box_abs1(false);
                    else
                        qDebug("Error in reading");
 //-----------------------------------------------------------
             qDebug("Asking for 0.5");
             thisPort->write(QByteArray("*B1IR2\r", 8));


             if (thisPort->waitForReadyRead(1000))
             {
                 qDebug("Read command ");
                 ReadData=thisPort->readLine();
                 qDebug()<<ReadData;
              }
             else
             {
                 qDebug("Too long answer in Degrader");
             }


              if  (ReadData.contains("*B10H"))
                   emit Set_check_box_05(true);
              else
                    if (ReadData.contains("*B10L"))
                         emit Set_check_box_05(false);
                     else
                         qDebug("Error in reading");
//-----------------------------------------------------------
             qDebug("Asking for 1");
            thisPort->write(QByteArray("*B1IR3\r", 8)); //request of the state of the degrader

             if (thisPort->waitForReadyRead(1000))
             {
                 qDebug("Read command ");
                 ReadData=thisPort->readLine();
                 qDebug()<<ReadData;
              }
             else
             {
                 qDebug("Too long answer in Degrader");
             }
             if  (ReadData.contains("*B10H"))
                 emit Set_check_box_1(true);
             else
                 if (ReadData.contains("*B10L"))
                     emit Set_check_box_1(false);
                 else
                     qDebug("Error in reading");
//-----------------------------------------------------------
              qDebug("Asking for 2");
              thisPort->write(QByteArray("*B1IR4\r", 8)); //request of the state of the degrader

              if (thisPort->waitForReadyRead(1000))
              {
                  qDebug("Read command ");
                  ReadData=thisPort->readLine();
                  qDebug()<<ReadData;
               }
              else
              {
                  qDebug("Too long answer in Degrader");
              }

              if  (ReadData.contains("*B10H"))
                   emit Set_check_box_2(true);
              else
                   if (ReadData.contains("*B10L"))
                        emit Set_check_box_2(false);
                   else
                        qDebug("Error in reading");
//-----------------------------------------------------------
               qDebug("Asking for abs2");
               thisPort->write(QByteArray("*B1IR5\r", 8)); //request of the state of the degrader

               if (thisPort->waitForReadyRead(1000))
               {
                   qDebug("Read command ");
                   ReadData=thisPort->readLine();
                   qDebug()<<ReadData;
                }
               else
               {
                   qDebug("Too long answer in Degrader");
               }
                if  (ReadData.contains("*B10H"))
                                 emit Set_check_box_abs2(true);
                else
                                 if (ReadData.contains("*B10L"))
                                      emit Set_check_box_abs2(false);
                                 else
                                      qDebug("Error in reading");


    }
    else qDebug("Error in Check_Degrader_State, DEGRADER com is closed");

qDebug("Degrader State is Checked");


}


/*
void Port::handleError(QSerialPort::SerialPortError error)//проверка ошибок при работе
{
    if ( (thisPort.isOpen()) && (error == QSerialPort::ResourceError)) {
        error_(thisPort.errorString().toLocal8Bit());
        DisconnectPort();
    }
}
*/
//--------------------------Degrader moving-------------------
void Port:: MoveDegrader_05(bool state){
        qDebug("Enter in moving Degrader 05");
   QByteArray ReadData;
        if (thisPort->isOpen())
            {
                if (state)
                    {
                            thisPort->write(QByteArray("*B1OS2H\r", 9));
                    }
                 else
                    {
                            thisPort->write(QByteArray("*B1OS2L\r", 9));
                    }


                if (thisPort->waitForReadyRead(1000))
                {
                    qDebug("Read command ");
                    ReadData=thisPort->readLine();
                    qDebug()<<ReadData;
                 }
                else
                {
                    qDebug("Too long answer in Degrader Move");
                }


                if  (ReadData.contains("*B10"))
                     qDebug("Succses");
                else
                      if (ReadData.contains("*B1"))
                          qDebug("False");
                       else
                           qDebug("Error in reading");

            }
            else qDebug("Error in MoveDegrader_05, COM port is closed");

}
void Port:: MoveDegrader_1(bool state){

    qDebug("Enter in moving Degrader 1");
if (thisPort->isOpen())
        {
            if (state)
                {
                        thisPort->write(QByteArray("*B1OS3H\r", 9));
                }
             else
                {
                        thisPort->write(QByteArray("*B1OS3L\r", 9));
                }
            QByteArray ReadData;

            if (thisPort->waitForReadyRead(1000))
            {
                qDebug("Read command ");
                ReadData=thisPort->readLine();
                qDebug()<<ReadData;
             }
            else
            {
                qDebug("Too long answer in Degrader Move");
            }
            if  (ReadData.contains("*B10"))
                 qDebug("Succses");
            else
                  if (ReadData.contains("*B1"))
                      qDebug("False");
                   else
                       qDebug("Error in reading");

        }
        else qDebug("Error in MoveDegrader_1, COM port is closed");
}
void Port:: MoveDegrader_2(bool state){

    qDebug("Enter in moving Degrader 2");
if (thisPort->isOpen())
        {
            if (state)
                {
                        thisPort->write(QByteArray("*B1OS4H\r", 9));
                }
             else
                {
                        thisPort->write(QByteArray("*B1OS4L\r", 9));
                }

            qDebug(Port_data);

            if  (Port_data.contains("*B10"))
                 qDebug("Succses");
            else
                  if (Port_data.contains("*B1"))
                      qDebug("False");
                   else
                       qDebug("Error in reading");

        }
        else qDebug("Error in MoveDegrader_2, COM port is closed");
}
void Port:: MoveDegrader_abs1(bool state){
    qDebug("Enter in moving Degrader abs1");
if (thisPort->isOpen())
        {
            if (state)
                {
                        thisPort->write(QByteArray("*B1OS1H\r", 9));
                }
             else
                {
                        thisPort->write(QByteArray("*B1OS1L\r", 9));
                }
            QByteArray ReadData;

            if (thisPort->waitForReadyRead(1000))
            {
                qDebug("Read command ");
                ReadData=thisPort->readLine();
                qDebug()<<ReadData;
             }
            else
            {
                qDebug("Too long answer in Degrader Move");
            }


            if  (ReadData.contains("*B10"))
                 qDebug("Success");
            else
                  if (ReadData.contains("*B1"))
                      qDebug("False");
                   else
                       qDebug("Error in reading");

        }
        else qDebug("Error in MoveDegrader_abs1, COM port is closed");

}
void Port:: MoveDegrader_abs2(bool state){
    qDebug("Enter in moving Degrader abs2");
if (thisPort->isOpen())
        {
            if (state)
                {
                        thisPort->write(QByteArray("*B1OS5H\r", 9));
                }
             else
                {
                        thisPort->write(QByteArray("*B1OS5L\r", 9));
                }

            QByteArray ReadData;

            if (thisPort->waitForReadyRead(1000))
            {
                qDebug("Read command ");
                ReadData=thisPort->readLine();
                qDebug()<<ReadData;
             }
            else
            {
                qDebug("Too long answer in Degrader Move");
            }


            if  (ReadData.contains("*B10"))
                 qDebug("Succses");
            else
                  if (ReadData.contains("*B1"))
                      qDebug("False");
                   else
                       qDebug("Error in reading");

        }
        else qDebug("Error in MoveDegrader_abs2, COM port is closed");

}


//--------------READ current state of Devices

void Port:: ReadUNIDOS (void){
 QByteArray ReadData_UNIDOS,Error="errr";
qDebug("Read Current:");
//emit Current_changed(QByteArray::number(1e-12));

/*
 float CurrentTest[50]= {0.000000e+000, 0.000000e+000, 1.007000e-008, 1.016000e-008, 9.950000e-009, 1.000000e-008, 9.970000e-009, 4.880000e-008, 8.006000e-008, 7.016000e-008, 5.010000e-008, 1.017000e-008, 1.007000e-008, 5.016000e-09,4.015000e-09,3.007000e-009, 1.016000e-008, 9.950000e-009, 1.000000e-008, 9.970000e-009, 9.880000e-009, 1.006000e-008, 1.016000e-008, 1.010000e-008, 1.017000e-008, 1.007000e-008, 1.016000e-008, 1.015000e-008,1.007000e-008, 1.016000e-008, 9.950000e-009, 1.000000e-008, 9.970000e-009, 9.880000e-009, 1.006000e-008, 1.016000e-008, 1.010000e-008, 1.017000e-008, 1.007000e-008, 1.016000e-008, 1.015000e-008,1.007000e-008, 1.016000e-008, 9.950000e-009, 1.000000e-008, 9.970000e-009, 9.880000e-009, 1.006000e-008, 1.016000e-008};


    if (CurrentIterator<31)
    {
        CurrentIterator++;
        emit Current_changed(QByteArray::number(CurrentTest[CurrentIterator]));
    }*/

 if (thisPort->isOpen())
      {

        thisPort->write(QByteArray("PTW\r\n", 5)); //????????????
        qDebug()<<QByteArray("PTW\r\n", 5);


        if (thisPort->waitForReadyRead(5000))
        {
            qDebug("Read Unidos command 1");
            ReadData_UNIDOS=thisPort->readLine();
            qDebug()<<ReadData_UNIDOS;
         }
        else
        {
            qDebug("Too long answer in UNIDOS 1");
            return;
        }

      //  if  (ReadData.contains("*B10")) //????


     //  ofile = open("logfile.txt",'w')
        thisPort->write(QByteArray("M1\r\n", 4));
        qDebug()<<QByteArray("M1\r\n", 4);



        if (thisPort->waitForReadyRead(5000))
        {
            qDebug("Read command 2");
            ReadData_UNIDOS=thisPort->readLine();
            qDebug()<<ReadData_UNIDOS;
         }
        else
        {
            qDebug("Too long answer in UNIDOS 2");
            emit Current_changed(Error,2);
            return;
        }

        thisPort->write(QByteArray("STA\r\n", 5));
        qDebug()<<QByteArray("STA\r\n", 5);


        if (thisPort->waitForReadyRead(5000))
        {
            qDebug("Read command 3");
            ReadData_UNIDOS=thisPort->readLine();
            qDebug()<<ReadData_UNIDOS;
         }
        else
        {
            qDebug("Too long answer in UNIDOS 3");
            emit Current_changed(Error,2);
            return;
        }

       //time.sleep(int(sys.argv[2]))   #wait argv[2] sec before new measurement
       //timestamp=int(time.time()) #get unix time

         thisPort->write(QByteArray("D\r\n", 3));
        qDebug()<<QByteArray("D\r\n", 3);



        if (thisPort->waitForReadyRead(5000))
        {
            qDebug("Read command 4");
            ReadData_UNIDOS=thisPort->readLine();
            qDebug()<<ReadData_UNIDOS;
         }
        else
        {
            qDebug("Too long answer in UNIDOS 3");
             emit Current_changed(Error,2);
            return;
        }
if (!ReadData_UNIDOS.contains("D1")) ReadData_UNIDOS.clear();

   for (int i=0;i<20; i++)

   {
        if (thisPort->waitForReadyRead(5000))
        {
            qDebug("Read command 5");
           ReadData_UNIDOS.append(thisPort->readLine());
            qDebug()<<ReadData_UNIDOS;
         }
        else
        {
            qDebug("Too long answer in UNIDOS 4");
            emit Current_changed(Error,2);
            return;
        }

        if (ReadData_UNIDOS.contains("\r\n") && ReadData_UNIDOS.size()>6) break;
}

 //emit NewDebugMSG("answer:" + ReadData_UNIDOS);
        QByteArray line = ReadData_UNIDOS;
qDebug("Answer from UNIOS is");
      QList<QByteArray> word= line.split(';');
qDebug()<<line;
if (word.size()<5)
              qDebug("Error in Currnet, not enoupgh info");
      else
      {
        Current= word[5];

        while (Current.contains(" "))
            {
                qDebug("It contains!");
                Current.remove(0,1);
             }
        emit NewDebugMSG("Current RAW data: " + Current+ "\r");
        qDebug("UNIDOS CURRENT IS");
        qDebug()<<Current;
         emit Current_changed(Current,1);
      }
      //  qDebug ("Current is %s",Current);
      // ofile.write(word[5].decode('ascii')) #write fifth substring to output
    }
    else {
            qDebug ("Error in reading current, UNIDOS com is closed");

        }


}


void Port:: UpdateCoordinates(void)
{
    emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));

}

void Port:: MOVE_MCL (int gotoX,int gotoY)
{


     int temp_time=0, waiting_const, overwaiting_flag=0;

        qDebug("MCL MOVE x= %d y=%d \n", gotoX,gotoY);

      QByteArray SendComand="#", ReadData;
      StopFlag=0;

      qDebug()<<"this port is "<<thisPort->isOpen();
if (thisPort->isOpen())
{
    int StatusX,StatusY,CurrentX=CheckCoordinate(MOTORX_ID),CurrentY=CheckCoordinate(MOTORY_ID);

        if (gotoX/STEP_CONST!=CurrentX)
                            {
                             SendComand.append(QByteArray::number(MOTORX_ID));
                             SendComand.append("s");
                             SendComand.append(QByteArray::number((gotoX)));
                             SendComand.append("\r");



                            if  (!ComandTransferForMCL(SendComand, "MoveX"))
                            {
                                  emit ConnectionError();
                                 return;
                            }

                             SendComand="#";
                             SendComand.append(QByteArray::number(MOTORX_ID));
                             SendComand.append("A\r");



                             if  (!ComandTransferForMCL(SendComand, "StartX"))
                             {
                                   emit ConnectionError();
                                 return;
                             }



                            //gotoX-CurrentX*STEP_CONST - distance in step
                            //1000 Hz - max speed
                            //  + 1 sec for obtaining results
                            // 0.5 - we check state every 500 ms

                            waiting_const= (int) (( abs(gotoX-CurrentX*STEP_CONST)/1000  + 4 )/0.5 );
                            emit NewDebugMSG("Will wait status for "+QString::number(waiting_const*0.5)+" sec \r");
                            emit MotorStat(true);
                            qDebug()<<"Will wait status for "<<QString::number(waiting_const*0.5)<<" sec";


                            while (1)
                             {
                                 if (thisPort->waitForReadyRead(500))
                                  {
                                       ReadData=thisPort->readAll();
                                       qDebug()<<"Read command "<<ReadData;
                                       break;
                                   }
                                   if  (StopFlag)
                                     {
                                       qDebug()<<"Stoped";
                                       STOP_MOVING();
                                       return;
                                     }
                                    if (temp_time>=waiting_const)
                                            {
                                                qDebug()<<"Too long waiting";
                                                STOP_MOVING();
                                                overwaiting_flag=1;
                                                break;

                                            }
                                    temp_time++;


                             }
                             emit MotorStat(false);
                             //write some processing for status msg
                            qDebug()<<"Overwaiting flag check X";
                            if (overwaiting_flag==1)
                            {
                                            SendComand="#";
                                            SendComand.append(QByteArray::number(MOTORX_ID));
                                            SendComand.append("$\r");
                                            thisPort->write(SendComand);

                                            if (thisPort->waitForReadyRead(500))
                                                {
                                                    qDebug("Read command ");
                                                    ReadData=thisPort->readAll();
                                                     qDebug()<<ReadData;
                                                 }
                                           else
                                                 {
                                                    qDebug()<<"Too long answer ";
                                                    emit NewDebugMSG("Too long answer in Repeated checking of status X\r");

                                                }

                                           int Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();
                                           if (((Status & 4) >> 2)==1)
                                              {

                                                emit LimitSwitchError();
                                                 emit NewDebugMSG("Limit switch X touched");
                                                emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));
                                                emit Check_Nanotec_state();
                                                return;
                                              }
                                            else
                                                {
                                                     emit NewDebugMSG("Too long answer in Moving, Something wrong");
                                                     return;
                                                }
                            }
        }

        if (gotoY/STEP_CONST!=CurrentY)
                     {

                             qDebug()<<"Now Mobving Y";
                             emit NewDebugMSG("Now Mobving Y\r");

                             SendComand="#";
                             SendComand.append(QByteArray::number(MOTORY_ID));
                             SendComand.append("s");
                             SendComand.append(QByteArray::number(gotoY));
                             SendComand.append("\r");

                             if  (!ComandTransferForMCL(SendComand, "MoveY"))
                             {
                                   emit ConnectionError();
                                   return;
                             }


                             SendComand="#";
                             SendComand.append(QByteArray::number(MOTORY_ID));
                             SendComand.append("A\r");

                              if  (!ComandTransferForMCL(SendComand, "StartY"))
                              {
                                  emit ConnectionError();
                                 return;
                            }



                              waiting_const= (int) (( abs(gotoY-CurrentY*STEP_CONST)/1000  + 4 )/0.5 );
                              emit NewDebugMSG("Will wait status for "+QString::number(waiting_const*0.5)+" sec \r");
                              qDebug()<<"Will wait status for "<<QString::number(waiting_const*0.5)<<" sec";
                                 emit MotorStat(true);

                                temp_time=0;
                             while (1)
                             {
                                 if (thisPort->waitForReadyRead(500))
                                  {                    
                                      qDebug("Read command ");
                                      ReadData=thisPort->readAll();
                                      qDebug()<<ReadData;
                                       break;
                                   }
                                   if  (StopFlag)
                                     {
                                       qDebug()<<"Stoped";
                                       STOP_MOVING();
                                       return;
                                     }
                                    if (temp_time==waiting_const)
                                    {
                                           qDebug()<<"Too long waiting";
                                           STOP_MOVING();
                                           overwaiting_flag=1;
                                           break;
                                    }


                                    temp_time++;


                             }
                                 emit MotorStat(false);
                                qDebug()<<"Check Y overwaiting"<<overwaiting_flag;
                             if (overwaiting_flag==1)
                                                         {
                                                                         SendComand="#";
                                                                         SendComand.append(QByteArray::number(MOTORY_ID));
                                                                         SendComand.append("$\r");
                                                                         thisPort->write(SendComand);

                                                                         if (thisPort->waitForReadyRead(500))
                                                                             {
                                                                                 qDebug("Read command ");
                                                                                 ReadData=thisPort->readAll();
                                                                                  qDebug()<<ReadData;
                                                                              }
                                                                        else
                                                                              {
                                                                                 qDebug()<<"Too long answer ";
                                                                                 emit NewDebugMSG("Too long answer in Repeated checking of status Y\r");

                                                                             }

                                                                        int Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();
                                                                        if (((Status & 4) >> 2)==1)
                                                                           {

                                                                             emit LimitSwitchError();
                                                                             emit NewDebugMSG("Limit switch Y touched");
                                                                             emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));
                                                                             emit Check_Nanotec_state();
                                                                             return;
                                                                           }
                                                                         else
                                                                             {
                                                                                  emit NewDebugMSG("Too long answer in Moving Y, Something wrong");
                                                                                  return;
                                                                             }
                                                         }

                             }



        if (CheckCoordinate(MOTORX_ID)==gotoX/STEP_CONST && CheckCoordinate(MOTORY_ID)==gotoY/STEP_CONST)
        {
            emit NewDebugMSG("Moving completed successfully \r");
            qDebug()<<"Moving completed successfully ";
        }
        else
        {
            emit NewDebugMSG("Some Error in moving\r");
            qDebug()<<"Some Error in moving";

        }
        emit Check_Nanotec_state();
        emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));

}
else
        emit ConnectionError();


}

void Port:: StopButton(void)
{
    qDebug("I`m in stop button");
     StopFlag=1;
}

void Port:: Calibration(void)
{
    //set mode
 int overwaiting_flag;

    StopFlag=false;

    QByteArray SendComand="#",ReadData;
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("p4\r");

    qDebug()<<"Im in the calibration";

    if  (!ComandTransferForMCL(SendComand, "Set positioning reference run mode Y"))
    {
         emit ConnectionError();
         emit CalibrationDone(false);
            qDebug()<<"I've got error";
          return;

    }
    qDebug()<<"what i`m doing here?!1";

    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("d1\r");



    if  (!ComandTransferForMCL(SendComand, "Set direction for reference run Y"))
    {
          emit ConnectionError();
         emit CalibrationDone(false);
          return;
    }


    SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("A\r");



    if  (!ComandTransferForMCL(SendComand, "StartY"))
    {
          emit ConnectionError();
         emit CalibrationDone(false);
          return;
    }


    int waiting_const= (int) (( abs(AXIS_RANGEY*STEP_CONST)/1000  + 4 )/0.5 );
    emit NewDebugMSG("Will wait status for "+QString::number(waiting_const*0.5)+" sec \r");
    qDebug()<<"Will wait status for "<<QString::number(waiting_const*0.5)<<" sec";


     int  temp_time=0;
   while (1)
        {
       if (thisPort->waitForReadyRead(500))
        {
            qDebug("Read status ");
            ReadData=thisPort->readAll();
            qDebug()<<ReadData;
             break;
         }
         if  (StopFlag)
           {
             qDebug()<<"Stoped";
             STOP_MOVING();
              emit CalibrationDone(false);
             return;
           }
          if (temp_time==waiting_const)
          {
                 qDebug()<<"Too long waiting";
                 STOP_MOVING();
                  overwaiting_flag=1;
                 break;
          }


          temp_time++;


   }

   if (overwaiting_flag==1)
                               {
                                               SendComand="#";
                                               SendComand.append(QByteArray::number(MOTORY_ID));
                                               SendComand.append("$\r");
                                               thisPort->write(SendComand);

                                               if (thisPort->waitForReadyRead(500))
                                                   {
                                                       qDebug("Read command ");
                                                       ReadData=thisPort->readAll();
                                                        qDebug()<<ReadData;
                                                    }
                                              else
                                                    {
                                                       qDebug()<<"Too long answer ";
                                                       emit NewDebugMSG("Too long answer in Repeated checking of status Y\r");

                                                   }

                                              int Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();

                                               if (((Status & 4) >> 2)==1)
                                                 {


                                                   emit LimitSwitchError();
                                                 //   qDebug()<<"Limit Switch X is triggered now we at "<<-CheckCoordinate(MOTORY_ID)<<" and plans to move to "<<-CheckCoordinate(MOTORY_ID)-CONST_BACKWARDS*sign;
                                                    //  emit ClearPositionError();
                                                    //  emit MOVE_MCL(CheckCoordinate(MOTORX_ID)-CONST_BACKWARDS*sign,-CheckCoordinate(MOTORY_ID));
                                                   return;
                                               }
                               }




  //if (ReadData.remove(ReadData.size()-1,2).remove(0,2).toInt() & 1)
    {

            //Start X

            SendComand="#";
            SendComand.append(QByteArray::number(MOTORX_ID));
            SendComand.append("p4\r");

            if  (!ComandTransferForMCL(SendComand, "Set positioning reference run mode X"))
            {
                  emit ConnectionError();
                    emit CalibrationDone(false);
                  return;

            }


            SendComand="#";
            SendComand.append(QByteArray::number(MOTORX_ID));
            SendComand.append("d0\r");



            if  (!ComandTransferForMCL(SendComand, "Set direction for reference run X"))
            {
                  emit ConnectionError();
                 emit CalibrationDone(false);
                  return;
            }


            SendComand="#";
            SendComand.append(QByteArray::number(MOTORX_ID));
            SendComand.append("A\r");



            if  (!ComandTransferForMCL(SendComand, "StartX"))
            {
                  emit ConnectionError();
                 emit CalibrationDone(false);
                  return;
            }


             waiting_const= (int) (( abs(AXIS_RANGEX*STEP_CONST)/1000  + 4 )/0.5 );
            emit NewDebugMSG("Will wait status for "+QString::number(waiting_const*0.5)+" sec \r");
            qDebug()<<"Will wait status for "<<QString::number(waiting_const*0.5)<<" sec";


              temp_time=0;
           while (1)
                {
               if (thisPort->waitForReadyRead(500))
                {
                    qDebug("Read status X ");
                    ReadData=thisPort->readAll();
                    qDebug()<<ReadData;
                     break;
                 }
                 if  (StopFlag)
                   {
                     qDebug()<<"Stoped";
                     STOP_MOVING();
                      emit CalibrationDone(false);
                     return;
                   }
                  if (temp_time==waiting_const)
                  {
                         qDebug()<<"Too long waiting";
                         STOP_MOVING();
                         overwaiting_flag=1;
                         break;
                  }


                  temp_time++;


           }

           if (overwaiting_flag==1)
                                       {
                                                       SendComand="#";
                                                       SendComand.append(QByteArray::number(MOTORX_ID));
                                                       SendComand.append("$\r");
                                                       thisPort->write(SendComand);

                                                       if (thisPort->waitForReadyRead(500))
                                                           {
                                                               qDebug("Read command ");
                                                               ReadData=thisPort->readAll();
                                                                qDebug()<<ReadData;
                                                            }
                                                      else
                                                            {
                                                               qDebug()<<"Too long answer ";
                                                               emit NewDebugMSG("Too long answer in Repeated checking of status X\r");

                                                           }

                                                      int Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();

                                                       if (((Status & 4) >> 2)==1)
                                                         {

                                                           emit LimitSwitchError();
                                                         //  emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));
                                                         //  qDebug()<<"Limit Switch X is triggered now we at "<<-CheckCoordinate(MOTORY_ID)<<" and plans to move to "<<-CheckCoordinate(MOTORY_ID)-CONST_BACKWARDS*sign;
                                                            //  emit ClearPositionError();
                                                            //  emit MOVE_MCL(CheckCoordinate(MOTORX_ID)-CONST_BACKWARDS*sign,-CheckCoordinate(MOTORY_ID));
                                                           return;
                                                       }
                                       }






    }


            SendComand="#";
            SendComand.append(QByteArray::number(MOTORY_ID));
             SendComand.append("p2\r");
            if  (!ComandTransferForMCL(SendComand, "Set normal mode X"))
            {
                  emit ConnectionError();
                 emit CalibrationDone(false);
                  return;
            }



            SendComand="#";
            SendComand.append(QByteArray::number(MOTORX_ID));
            SendComand.append("p2\r");
            if  (!ComandTransferForMCL(SendComand, "Set normal mode Y"))
            {
                  emit ConnectionError();
                  emit CalibrationDone(false);
                  return;
            }



emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));
emit Check_Nanotec_state();
emit CalibrationDone(true);

}


void Port:: STOP_MOVING(void)
{
    qDebug("We are int the STOP_MOVING");
    QByteArray SendComand="#";
    SendComand.append(QByteArray::number(MOTORX_ID));
    SendComand.append("S\r");
   if (!ComandTransferForMCL(SendComand, "StopX"));
    {
         emit ConnectionError();
    }

     SendComand="#";
    SendComand.append(QByteArray::number(MOTORY_ID));
    SendComand.append("S\r");
    if (!ComandTransferForMCL(SendComand, "StopY"));
    {
        emit ConnectionError();
    }

    emit MotorStat(false);
    emit Current_coordinate_changed(CheckCoordinate(MOTORX_ID),-CheckCoordinate(MOTORY_ID));

}

void Port :: ClearPositionError(void)
{
if (thisPort->isOpen())
  {

    QByteArray SendComand="#";
   SendComand.append(QByteArray::number(MOTORX_ID));
   SendComand.append("D"+QByteArray::number(CheckCoordinate(MOTORX_ID)*STEP_CONST)+"\r");
   if (!ComandTransferForMCL(SendComand, "RestX"));
   {
       emit ConnectionError();
   }



   SendComand="#";
  SendComand.append(QByteArray::number(MOTORY_ID));
 SendComand.append("D"+QByteArray::number(CheckCoordinate(MOTORY_ID)*STEP_CONST)+"\r");
  if (!ComandTransferForMCL(SendComand, "RestY"));
  {
      emit ConnectionError();
  }
}
  emit Check_Nanotec_state();
}

void Port:: Check_Nanotec_state(void)
{

    int ReadyX,ZeroX,PErrorX,ReadyY,ZeroY,PErrorY;
if (thisPort->isOpen())
{
            QByteArray SendComand="#",ReadData;
            SendComand.append(QByteArray::number(MOTORX_ID));
            SendComand.append("$\r");

            emit NewDebugMSG("Send command : "+SendComand);
            qDebug()<<"Send command "<<SendComand;
            thisPort->write(SendComand);

          if (thisPort->waitForReadyRead(500))
           {
               qDebug("Read command ");
              ReadData=thisPort->readAll();
               qDebug()<<ReadData;
            }
           else
           {
               qDebug()<<"Too long answer ";
               emit NewDebugMSG("Too long answer \r");
             //  return;
            }

           int Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();
           qDebug()<<"StatusX"<<Status;

           ReadyX=Status & 1;
           ZeroX=(Status & 2) >> 1;
           PErrorX=(Status & 4) >> 2;


         //--------------NOW Y-----------------
           SendComand="#";
           SendComand.append(QByteArray::number(MOTORY_ID));
           SendComand.append("$\r");

           emit NewDebugMSG("Send command: "+SendComand);
           qDebug()<<"Send command : "<<SendComand;
           thisPort->write(SendComand);

         if (thisPort->waitForReadyRead(500))
          {
              qDebug("Read command ");
             ReadData=thisPort->readAll();
              qDebug()<<ReadData;
           }
          else
          {
              qDebug()<<"Too long answer ";
              emit NewDebugMSG("Too long answer \r");
           //   return;
           }

          Status=ReadData.remove(ReadData.size()-1,2).remove(0,4).toInt();
        qDebug()<<"StatusY"<<Status;
          ReadyY=Status & 1;
          ZeroY=(Status & 2) >> 1;
          PErrorY=(Status & 4) >> 2;
          qDebug()<<" "<<ReadyX<<ZeroX<<" "<<PErrorX<<" "<<ReadyY<<" "<<ZeroY<<" "<<PErrorY;
          emit(Nanotec_state(ReadyX,ZeroX,PErrorX,ReadyY,ZeroY,PErrorY));
    }
else
{
    qDebug()<<"Port Closed";
    emit NewDebugMSG("Port Closed for checking state \r");

}



 //emit(Nanotec_state(1,0,1,0,1,0));


}


Port::~Port()
{

    qDebug("By in Thread!");
    emit finished_Port();
}

//-----------------------SOMETHING




Something::Something(QObject *parent) :  QObject(parent)
{

}

void Something:: BeamScan(bool asix_state,int min_axis,int max_axis,int step,int fixed){
Abort_flag=false;
CoordinateUpdState=false;
CurrentUpdState=0;

    //сделать все объекты не активными во время теста
    //по умолчанию нормальные координаты для спинбоксов

    qDebug ("Beam scan is going");
    int coordinate=min_axis;


    while (coordinate < max_axis)
    {

            if (Abort_flag) break;

            if (asix_state)
                    emit MoveItTo(coordinate*STEP_CONST,-fixed*STEP_CONST);
            else    emit MoveItTo(fixed*STEP_CONST,-coordinate*STEP_CONST);

            qDebug()<<"waiting for UpdatingCoordinate   ";
        //emit NewDebugMSG("waiting for UpdatingCoordinate   ");
        while(1)
            {
                if (CoordinateUpdState) break;
                if (Abort_flag) break;

                qDebug()<<"CoordinateUpdState" << CoordinateUpdState;
            }
            //wait until coordinate is updated. MAYBE remove?!!

                qDebug()<<"coordinate"<<coordinate<<"fixed"<<fixed;
                coordinate+=step;
            QThread::msleep(2000); //wait some time to get stable measurements of Current


            CoordinateUpdState=false;

            CurrentUpdState=0;
            emit RunItUNIDOS();




            while(1)
                {
                    if (CurrentUpdState==1) break;
                    if (CurrentUpdState==2)
                    {

                        emit NewDebugMSG("Error with UNIDOS. Try to ask it again \r");
                        emit RunItUNIDOS();
                        QThread::msleep(1000);
                    }
                    if (Abort_flag) break;

                    //qDebug()<<"CurrentUpdState ffffffffs" << CurrentUpdState;
                }
            CurrentUpdState=0;
            emit Draw_Scan();
         //   coordinate+=step;
            //qDebug()<<"Coordinate" << coordinate;
             emit NewDebugMSG("Scan Coordinate  " + QString::number(coordinate) + "\r") ;
    }

 // emit Draw_Scan(); //ai
    emit BeamScanDone();
    qDebug ("Beam scan is done");
}
void Something:: AbortScan(){
    Abort_flag=true;
    qDebug()<<"Abort flag triggere";
}


void Something:: CoordinateUpdated()
{
    CoordinateUpdState=true;
    qDebug("Even It knows, that coordinate is updated");
}

void Something:: CurrentUpdated(int stat)
{
    CurrentUpdState=stat;
    qDebug("Even It knows, that current is updated");
}

Something::~Something()
{

    qDebug("Something is killed");
    emit killed();
}


