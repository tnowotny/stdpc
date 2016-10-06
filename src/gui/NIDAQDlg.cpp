#include "NIDAQDlg.h"
#include "Tools.cpp"
#include "Nidaq.h"
#include "DeviceManager.h"

NIDAQDlg::NIDAQDlg(int no, QWidget *parent) : DAQDlg(no, parent)
{
    setupUi(this);
    label = DAQLabel->text();
    setIndex(no);
    inDlg->init(board);
    outDlg->init(board);
}

void NIDAQDlg::setIndex(int no)
{
    idx = no;
    DAQLabel->setText(label.arg(no));
    ChannelIndex dex;
    dex.isValid = true;
    dex.isAnalog = true;
    dex.daqClass = DAQClass::NI;
    dex.devID = no;
    board = Devices.getDevice(dex);
    inDlg->dex = dex;
    emit channelsChanged();
}

bool NIDAQDlg::exportData(bool forceInit)
{
    bool change= false;
    bool devOK = true;
    getEntry(NIDAQp[idx].deviceName, DeviceNameE->text(), change);
    inDlg->exportData();
    outDlg->exportData();
    if ( change || forceInit ) {
        devOK = initDAQ() != DeviceStatus::Failed;
        inDlg->importData();
        outDlg->importData();
    }
    return devOK;
}

void NIDAQDlg::importData()
{
    DeviceNameE->setText(NIDAQp[idx].deviceName);
    inDlg->importData();
    outDlg->importData();
}

void NIDAQDlg::open()
{
    didInit = false;
    backup = NIDAQp[idx];
    QDialog::open();
}

void NIDAQDlg::accept()
{
    exportData();
    QDialog::accept();
}

void NIDAQDlg::reject()
{
    NIDAQp[idx] = backup;
    if ( didInit )
        initDAQ();
    importData();
    QDialog::reject();
}

DeviceStatus NIDAQDlg::initDAQ()
{
    QString name;
    DeviceStatus status = Devices.initSingle<NIDAQData>(name, idx);
    emit deviceStatusChanged(status, name);
    inDlg->init(board);
    outDlg->init(board);
    didInit = true;
    return status;
}

void NIDAQDlg::on_inChannels_clicked()
{
    exportData();
    inDlg->open();
}

void NIDAQDlg::on_outChannels_clicked()
{
    exportData();
    outDlg->open();
}
