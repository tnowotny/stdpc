#include "MainWin.h"
#include "ui_MainWin.h"
#include "LUtables.h"
#include "AP.h"
#include <windows.h>
#include <QScrollBar>
#include "ModelOpts.h"
#include "DaqOpts.h"

MyMainWindow::MyMainWindow(QWidget *parent)
     : QMainWindow(parent),
       ui(new Ui::MainWindow)
 {
     ui->setupUi(this);

     DSDlg= new DataSavingDlg(this);
     TrigDlg= new TriggerDlg(this);

     QVector<ComponentPrototype *> prototypes;
     for ( ConductanceProxy *proxy : ConductanceManager::Currents() )
         prototypes.push_back(new ComponentPrototype(proxy));
     ui->currentTable->init(prototypes);

     prototypes.clear();
     for ( ConductanceProxy *proxy : ConductanceManager::Synapses() )
         prototypes.push_back(new ComponentPrototype(proxy));
     ui->synapseTable->init(prototypes);

     QVector<DaqOptsPrototypeBase*> dprot;
     for ( DAQProxy *proxy : DeviceManager::Register() )
         dprot.push_back(new DaqOptsPrototype(proxy));
     for ( ModelProxy *proxy : ModelManager::Register() )
         dprot.push_back(new ModelOptsPrototype(proxy));
     ui->DAQTable->init(dprot, this);

     rateIndicator = new QLabel("Ready");
     ui->statusbar->addPermanentWidget(rateIndicator);
     loadedProtocolStatus = new QLabel("");
     ui->statusbar->addPermanentWidget(loadedProtocolStatus);
         
     DCT= new DCThread();
     
     connect(ui->StartBut, SIGNAL(clicked()), SLOT(StartButClicked()));
     connect(ui->StopBut, SIGNAL(clicked()), SLOT(StopButClicked()));
     connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MyMainWindow::TabChanged);
     
     connect(ui->actionExit, SIGNAL(triggered()), SLOT(close()));
     connect(ui->actionExport_Log, SIGNAL(triggered()), this, SLOT(ExportLog()));
     connect(ui->actionClear_Log, SIGNAL(triggered()), SLOT(ClearLog()));
     connect(ui->actionLoad_Protocol, SIGNAL(triggered()), this, SLOT(LoadProtocol()));
     connect(ui->actionSave_Protocol, SIGNAL(triggered()), this, SLOT(SaveProtocol()));
     connect(ui->actionLoad_Script, SIGNAL(triggered()), this, SLOT(LoadScript()));
     connect(ui->actionUnload_Script, SIGNAL(triggered()), SLOT(UnLoadScript()));
     connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(DisplayAbout()));
     connect(ui->actionStart_trigger, SIGNAL(triggered(bool)), TrigDlg, SLOT(open()));
     connect(ui->actionData_saving, SIGNAL(triggered(bool)), DSDlg, SLOT(open()));
     connect(this, SIGNAL(destroyed()), SLOT(close()));

     connect(this, &MyMainWindow::channelsChanged, &ChannelListModel::updateChns_static_noargs);
     connect(this, &MyMainWindow::modelRemoved, &ChannelListModel::updateChns_static);
     connect(this, &MyMainWindow::channelsChanged, TrigDlg, &TriggerDlg::updateChannels);

     connect(ui->HHActivate, SIGNAL(clicked(bool)), ui->currentTable, SLOT(activateAll()));
     connect(ui->HHDeactivate, SIGNAL(clicked(bool)), ui->currentTable, SLOT(deactivateAll()));
     connect(ui->HHClear, &QPushButton::clicked, [=](){ui->currentTable->importData(true);});
     connect(ui->HHReset, SIGNAL(clicked(bool)), ui->currentTable, SLOT(importData()));

     connect(ui->SynActivate, SIGNAL(clicked(bool)), ui->synapseTable, SLOT(activateAll()));
     connect(ui->SynDeactivate, SIGNAL(clicked(bool)), ui->synapseTable, SLOT(deactivateAll()));
     connect(ui->SynClear, &QPushButton::clicked, [=](){ui->synapseTable->importData(true);});
     connect(ui->SynReset, SIGNAL(clicked(bool)), ui->synapseTable, SLOT(importData()));

     connect(ui->DAQClear, &QPushButton::clicked, [=](){ui->DAQTable->importData(true);});
   
     connect(DCT,SIGNAL(message(QString)),SLOT(DisplayMessage(QString)));
     connect(DCT, &DCThread::updateRate, [this](int rate){
         rateIndicator->setText(QString("Update rate: %1 Hz").arg(rate));
     });
     connect(DCT, &DCThread::done, [this](){
         rateIndicator->setText("Ready");
     });
     connect(DCT,SIGNAL(CloseToLimit(QString, QString, double, double, double)),SLOT(CloseToLimitWarning(QString, QString, double, double, double)));
     
     initAP();
     LoadConfig();

     DisplayMessage(QString("Main: Dynamic Clamp starting ..."));
     exportData(true);
     updateDeviceStatus();
 } 

MyMainWindow::~MyMainWindow()
{
  delete ui;
  delete DSDlg;
  delete TrigDlg;
  delete DCT;

  ChannelListModel::destruct();
}

void MyMainWindow::CloseToLimitWarning(QString what, QString channelName, double lowLimit, double highLimit, double value)
{
   QString msg = what+QString(" is close to channel limit!\nChannel: ")+channelName+QString("\nHigh limit: ")+QString::number(highLimit)+QString("\nValue: ")+QString::number(value)+QString("\nLow limit: ")+QString::number(lowLimit);
   //QMessageBox::warning(this, tr("Warning"), msg);
   DisplayMessage(msg);
   return;
}


void MyMainWindow::DisplayMessage(QString message)
{
  QTime tstamp= QTime::currentTime();
  QScrollBar *sb = ui->MessageWindow->verticalScrollBar();
  bool down = sb->value() == sb->maximum();
  ui->MessageWindow->addItem(tstamp.toString()+QString(": ")+message);
  if ( down )
    ui->MessageWindow->scrollToBottom();
}

void MyMainWindow::updateDeviceStatus(DeviceStatus status, const QString &name)
{
    switch ( status ) {
    case DeviceStatus::Active :
        DisplayMessage(QString("Good news: %1 found and opened successfully!").arg(name));
        break;
    case DeviceStatus::Failed :
        DisplayMessage(QString("Bad news: %1 not found or not opened successfully!").arg(name));
        break;
    case DeviceStatus::Inactive :
    default:
        break;
    }

    emit channelsChanged();
    updateStartButton();
}

void MyMainWindow::updateStartButton()
{
    ui->StartBut->setEnabled(!Devices.active().empty() || Models.empty());
}


void MyMainWindow::closeEvent(QCloseEvent *event)
{
  QString fname;
  if (DCT) {
    //DCT->terminate();
    DCT->stopped= true;
    DisplayMessage(QString("Main: Dynamic Clamp stopped."));
    while (!DCT->finished) Sleep(100);
  }
  SaveConfig();
  fname= QString("StdpC_last.log");
  doExportLog(fname);
  event->accept();
}

void MyMainWindow::StartButClicked() 
{
  if ( !DCT->stopped && DCT->settling ) {
      DCT->wakeFromSettling = true;
      return;
  }
  ui->actionLoad_Protocol->setEnabled(false);
  ui->actionSave_Protocol->setEnabled(false);
  ui->actionLoad_Script->setEnabled(false);
  ui->actionUnload_Script->setEnabled(false);
  ui->actionExport_Log->setEnabled(false);
  ui->actionExit->setEnabled(false);
  ui->actionSave_config->setEnabled(false);
  ui->menuConfigure->setEnabled(false);
  ui->cbDatasaving->setEnabled(false);
  ui->cbTrigger->setEnabled(false);
  ui->cbSettle->setEnabled(false);
  ui->dblSettleDuration->setEnabled(false);
  ui->DAQTable->setEnabled(false);
  if (!DCT->stopped) {
    DCT->stopped= true;
    DisplayMessage(QString("Main: Dynamic Clamp stopped."));
  }
  while (!DCT->finished) Sleep(100);
  exportData();

  if ( !ui->graphtab->startPlotting(DCT) )
      ui->tabWidget->setTabEnabled(1, false);
  if ( !ui->performancetab->startPlotting(DCT) )
      ui->tabWidget->setTabEnabled(2, false);
  rateIndicator->setText("Started");
  DCT->setup_and_go();
}

void MyMainWindow::StopButClicked()
{
  ui->actionLoad_Protocol->setEnabled(true);
  ui->actionSave_Protocol->setEnabled(true);
  ui->actionLoad_Script->setEnabled(!DCT->scripting);
  ui->actionUnload_Script->setEnabled(DCT->scripting);
  ui->actionExport_Log->setEnabled(true);
  ui->actionExit->setEnabled(true);
  ui->actionSave_config->setEnabled(true);
  ui->menuConfigure->setEnabled(true);
  ui->cbDatasaving->setEnabled(true);
  ui->cbTrigger->setEnabled(true);
  ui->cbSettle->setEnabled(true);
  ui->dblSettleDuration->setEnabled(true);
  ui->DAQTable->setEnabled(true);
  if (!DCT->stopped) {
    DCT->stopped= true;
    DisplayMessage(QString("Main: Dynamic Clamp stopped."));
  }
  ui->graphtab->stopPlotting();
  ui->performancetab->stopPlotting();
  ui->tabWidget->setTabEnabled(1, true);
  ui->tabWidget->setTabEnabled(2, true);
}

void MyMainWindow::TabChanged(int idx)
{
    static int prevIdx = 0;
    if ( prevIdx == 0 && idx != 0 )
        exportData();
    prevIdx = idx;
}

void MyMainWindow::exportData(bool ignoreDAQ)
{
  ui->synapseTable->exportData();
  ui->currentTable->exportData();
  ui->DAQTable->exportData(ignoreDAQ);
  DSDlg->exportData();
  dataSavingPs.enabled = ui->cbDatasaving->isChecked();
  TrigDlg->exportData();
  Triggerp.active = ui->cbTrigger->isChecked();
  Settlingp.active = ui->cbSettle->isChecked();
  Settlingp.duration = ui->dblSettleDuration->value();
  ui->graphtab->exportData();
  ui->performancetab->exportData();
  emit channelsChanged();
}
 
void MyMainWindow::importData()
{
  ui->DAQTable->importData();
  ui->synapseTable->importData();
  ui->currentTable->importData();
  DSDlg->importData();
  ui->cbDatasaving->setChecked(dataSavingPs.enabled);
  TrigDlg->importData();
  ui->cbTrigger->setChecked(Triggerp.active);
  ui->cbSettle->setChecked(Settlingp.active);
  ui->dblSettleDuration->setValue(Settlingp.duration);
  ui->graphtab->importData();
  ui->performancetab->importData();
  updateStartButton();
}

void MyMainWindow::SaveConfig()
{
  ofstream os("StdpC.conf");
  DoSaveProtocol(os);
  os.close();
}

void MyMainWindow::LoadConfig()
{
    ifstream is("StdpC.conf");
    if ( !DoLoadProtocol(is) )
        DisplayMessage(QString("No valid config file found; reverting to default settings"));
    is.close();
}

void MyMainWindow::SaveProtocol()
{
  QString fname = QFileDialog::getSaveFileName(this, "Save protocol to file", "", "StdpC protocols (*.cpr);;All files(*)");
  if ( fname.isEmpty() )
      return;
  if ( !fname.endsWith(".cpr") && !QFileInfo(fname).exists() && !QFileInfo(QString("%1.cpr").arg(fname)).exists() )
      fname.append(".cpr");

  ofstream os(fname.toLatin1());
  DoSaveProtocol(os);
  os.close();
}

void MyMainWindow::DoSaveProtocol(ofstream &os)
{
  os << STDPC_PROTOCOL_HEADER << " " << STDPC_PROTOCOL_VERSION << endl << endl;

  exportData();

  for ( auto const& ap : AP::params() ) {
      ap->write(os);
  }
}


void MyMainWindow::LoadProtocol()
{
  QString fname = QFileDialog::getOpenFileName(this, "Load protocol file", "", "StdpC protocols (*.cpr);;All files(*)");
  if ( fname.isEmpty() )
    return;

  ifstream is(fname.toLatin1());
  if (!is.good()) {
    DisplayMessage(QString("Error opening protocol file"));
    return;
  }

  bool ret = DoLoadProtocol(is);
  is.close();

  if ( ret )
      loadedProtocolStatus->setText(QString("Loaded protocol file: %1").arg(fname));
  else
      DisplayMessage(QString("Error reading protocol file"));
}

bool MyMainWindow::DoLoadProtocol(ifstream &is)
{
  // Clear params before loading
  Models.clear();
  Devices.clear();
  Conductances.clear();
  Plotp.plot.clear();

  std::function<bool(QString)> callback = [=](QString name) {
      DisplayMessage(QString("Warning: Failed to read parameter \"%1\"").arg(name));
      return false;
  };
  bool ret = readProtocol(is, &callback);

  importData();

  return ret;
}


void MyMainWindow::ExportLog()
{
  QString fname = QFileDialog::getSaveFileName(this, "Export log to file", "", "Log files (*.log);;All files (*)");
  if ( fname.isEmpty() )
      return;
  if ( !fname.endsWith(".log") && !QFileInfo(fname).exists() && !QFileInfo(QString("%1.log").arg(fname)).exists() )
      fname.append(".log");
  doExportLog(fname);
}

void MyMainWindow::doExportLog(QString fname)
{
  ofstream os(fname.toLatin1());
  int i= 0, done= 0;
  QListWidgetItem *it;
  
  while (!done) {
    it= ui->MessageWindow->item(i);
    if (it != NULL) {
      os << it->text().toStdString() << endl;
    }
    else done= 1;
    i++;
  }
  DisplayMessage(QString("Log exported to ")+fname);
}
    
void MyMainWindow::ClearLog()
{
  ui->MessageWindow->clear();
}

void MyMainWindow::LoadScript()
{
  QString fname = QFileDialog::getOpenFileName(this, "Load script file", "", "StdpC scripts(*.scr);;All files(*)");
  if ( !fname.isEmpty() && DCT->LoadScript(fname) ) {
    ui->actionLoad_Script->setEnabled(false);
    ui->actionUnload_Script->setEnabled(true);
  }
}

void MyMainWindow::UnLoadScript()
{
  DCT->UnloadScript();
  ui->actionLoad_Script->setEnabled(true);
  ui->actionUnload_Script->setEnabled(false);
}

void MyMainWindow::DisplayAbout()
{
    QMessageBox::information(this,tr("About StdpC"),tr("StdpC is free dynamic clamp software including plasticity of synapses and active electrode compensation. \n It is distributed under the GPL v2 license. \n You are running version StdpC 2017."));
}
