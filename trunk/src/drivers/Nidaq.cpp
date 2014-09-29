
#include "Nidaq.h"
#include "limits.h"
#include <sstream>
#include "NIDAQmx.h"
#include <QMessageBox>
#include <QString>
#include <cmath>

// NIDAQmx device driver

//---------------------------------------------------------------------------

NIDAQ::NIDAQ()
{
  char data[1024];
  float64 fdata[128];
  istringstream *istr;
  stringstream ostr;
  char cbuf;
  
  devName= new char[80];
  strcpy(devName, (const char *) NIDAQp.deviceName.toLatin1());
  
  DevicePresent= (DAQmxGetDevAIPhysicalChans (devName, data, 1024) == 0);
  //if (!DevicePresent)
//  QMessageBox::warning(NULL, QString("My Application"),
//                QString(data),
//                QMessageBox::Ok);
    
  if (DevicePresent) {
    istr=new istringstream(data);
    int k= 0;
    while (istr->good()) {
      iChnNm[k]= new char[80];
      istr->getline(iChnNm[k++], 80, ',');
      istr->get(cbuf);
    }
    delete istr;
    inChnNo= k;
    inIdx= new short int[inChnNo];
    inGainFac= new double[inChnNo];
  
    for (int i= 0; i < 128; i++) fdata[i]= 0.0;
    DAQmxGetDevAIVoltageRngs(devName, fdata, 128);
    k= 0;
    while ((abs(fdata[k]) > 1e-10) || (abs(fdata[k+1]) > 1e-10)) k+=2;
    inGainNo= k/2;
    inGainText= new char*[inGainNo];
    inLow= QVector<double>(inGainNo);
    inHigh= QVector<double>(inGainNo);
    for (int i= 0; i < inGainNo; i++) {
      ostr.clear();
      ostr << fdata[2*i] << " to " << fdata[2*i+1] << " V" << ends;
      inGainText[i]= new char[80];
      ostr.getline(inGainText[i], 80);
      inLow[i]= fdata[2*i];
      inHigh[i]= fdata[2*i+1];
    }
  
    DAQmxGetDevAOPhysicalChans (devName, data, 1024);
    istr=new istringstream(data);
    k= 0;
    while (istr->good()) {
      oChnNm[k]= new char[80];
      istr->getline(oChnNm[k++], 80, ',');
      istr->get(cbuf);
    }
    delete istr;
    outChnNo= k;
    outIdx= new short int[outChnNo];
    outGainFac= new double[outChnNo];

    for (int i= 0; i < 128; i++) fdata[i]= 0.0;
    DAQmxGetDevAOVoltageRngs(devName, fdata, 128);
    k= 0;
    while ((fdata[k] != 0.0) || (fdata[k+1] != 0.0)) k+=2;
    outGainNo= k/2;
    outGainText= new char*[outGainNo];
    outLow= QVector<double>(outGainNo);
    outHigh= QVector<double>(outGainNo);
    for (int i= 0; i < outGainNo; i++) {
      ostr.clear();
      ostr << fdata[2*i] << " to " << fdata[2*i+1] << " V" << ends;
      outGainText[i]= new char[80];
      ostr.getline(outGainText[i], 80);
      outLow[i]= fdata[2*i];
      outHigh[i]= fdata[2*i+1];
    }
  }
  else {
    inChnNo= 0;
    inGainNo= 0;
    outChnNo= 0;
    outGainNo= 0;
  }

  inTaskActive= 0;
  outTaskActive= 0;  
  
  // setup of the system clock
  QueryPerformanceFrequency(&intClock_frequency);
  clock_frequency= (double) intClock_frequency.LowPart;
  clock_cycle= ((double) UINT_MAX + 1.0)/clock_frequency;
  
//   QMessageBox::warning(NULL, QString("My Application"),
//                QString("Finished scanning ouput ranges, exiting NIDAQ constructor"),
//               QMessageBox::Close); 

//  init();
};

NIDAQ::~NIDAQ()
{
  if (DevicePresent) {
    for (int i= 0; i < inChnNo; i++) {
      delete[] iChnNm[i];
    }
    for (int i= 0; i < inGainNo; i++) {
      delete[] inGainText[i];
    }
    delete[] inGainText;
    for (int i= 0; i < outChnNo; i++) {
      delete[] oChnNm[i];
    }
    for (int i= 0; i < outGainNo; i++) {
      delete[] outGainText[i];
    }
    delete[] outGainText;
    //delete[] inChnGain;
    delete[] inIdx;
    delete[] inGainFac;
    delete[] outIdx;
    delete[] outGainFac;
    DAQmxResetDevice(devName);
    delete[] devName;
  }
}

void NIDAQ::init()
{
}

//---------------------------------------------------------------------------
bool NIDAQ::initialize_board(QString &name)
{
  char cname[1024];
  bool success= false;

  if (DevicePresent) {
    success= (DAQmxGetDevProductType(devName, cname, 1024) == 0);
    if (success) name= QString(cname);
    else name= QString(devName);
    DAQmxResetDevice(devName);
    initialized= success;
  }
  return success;
}

//---------------------------------------------------------------------------
void NIDAQ::reset_RTC()
{
  // this is called when the dynamic clamp starts ... start tasks
  static LARGE_INTEGER inT;
   
  if (DevicePresent) {
    DAQmxStartTask(inTask);
    DAQmxStartTask(outTask);
  }

  QueryPerformanceCounter(&inT);
  sysT= ((double) inT.LowPart)/clock_frequency;
  t= 0.0;
}

//---------------------------------------------------------------------------
double NIDAQ::get_RTC(void)
{
  static LARGE_INTEGER inT;
  static double dt, lastT;
  
  QueryPerformanceCounter(&inT);
  lastT= sysT;
  sysT= ((double) inT.LowPart)/clock_frequency;
  dt= sysT-lastT;
  if (dt < 0.0) dt+= clock_cycle;
  t+= dt;
 
  return dt;
}

//---------------------------------------------------------------------------
void NIDAQ::digital_out(unsigned char outbyte)
{
// not supported for the NIDAQ driver
}


//---------------------------------------------------------------------------
void NIDAQ::generate_scan_list(short int chnNo, short int *Chns)
{
  if (DevicePresent) {
    actInChnNo= chnNo;
    if (inTaskActive != 0) {
       DAQmxClearTask(inTask);
       delete[] inBuf;
    }
    inBuf= new float64[chnNo];
    DAQmxCreateTask ("", &inTask);
  
    for (int i= 0; i < chnNo; i++) {
      inIdx[i]= Chns[i];
      DAQmxCreateAIVoltageChan (inTask, iChnNm[inIdx[i]], "", DAQmx_Val_RSE, inLow[inChnp[inIdx[i]].gain],
                                inHigh[inChnp[inIdx[i]].gain], DAQmx_Val_Volts, "");
      inGainFac[i]= inChnp[inIdx[i]].gainFac;
    }
    DAQmxSetSampTimingType(inTask, DAQmx_Val_OnDemand);
    DAQmxSetReadOverWrite(inTask, DAQmx_Val_OverwriteUnreadSamps);
//  DAQmxSetBufInputBufSize(inTask, actInChnNo);
    inTaskActive= 1;
  }
}

//---------------------------------------------------------------------------
void NIDAQ::get_scan(inChannel *in)
{
  // assume device is present if this is called (responsibility of calling code!)
  static int i;
  static int32 spr;
//  QString temp;

  DAQmxReadAnalogF64(inTask, 1, 1.0e-4, DAQmx_Val_GroupByScanNumber, inBuf, actInChnNo, &spr, NULL);
//  temp.setNum(inBuf[0]);
//  QMessageBox::warning(NULL, QString("My Application"),
//                temp,
//                QMessageBox::Ok);
  for (i= 0; i < actInChnNo; i++) in[inIdx[i]].V= inGainFac[i]*inBuf[i];
}

//---------------------------------------------------------------------------
void NIDAQ::get_single_scan(inChannel *in, int which)
{
  // assume device is present if this is called (responsibility of calling code!)
  static int i;
  static int32 spr;
//  QString temp;

  DAQmxReadAnalogF64(inTask, 1, 1.0e-4, DAQmx_Val_GroupByScanNumber, inBuf, actInChnNo, &spr, NULL);
//  temp.setNum(inBuf[0]);
//  QMessageBox::warning(NULL, QString("My Application"),
//                temp,
//                QMessageBox::Ok);
  for (i= 0; i < actInChnNo; i++) {
      if (inIdx[i] == which) {
          in[inIdx[i]].V= inGainFac[i]*inBuf[i];
      }
  }
}


//---------------------------------------------------------------------------
void NIDAQ::generate_analog_out_list(short int chnNo, short int *Chns)
{
  if (DevicePresent) {
    actOutChnNo= chnNo;
    if (outTaskActive != 0) {
      DAQmxClearTask(outTask);
      delete[] outBuf;
    }
    outBuf= new float64[actOutChnNo];
    DAQmxCreateTask("", &outTask);
   
    for (int i= 0; i < chnNo; i++) {
      outIdx[i]= Chns[i];
      DAQmxCreateAOVoltageChan (outTask, oChnNm[outIdx[i]], "", outLow[outChnp[outIdx[i]].gain],
                                outHigh[outChnp[outIdx[i]].gain], DAQmx_Val_Volts, "");
      outGainFac[i]= outChnp[outIdx[i]].gainFac*1e9;
    }
    DAQmxSetSampTimingType(outTask, DAQmx_Val_OnDemand);
    //DAQmxSetBufOutputBufSize(outTask, actOutChnNo*10);
    outTaskActive= 1;
  }
}


//---------------------------------------------------------------------------
void NIDAQ::write_analog_out(outChannel *out)
{
  static int i;
  static int32 spw;
  for (i= 0; i < actOutChnNo; i++) outBuf[i]= out[outIdx[i]].I*outGainFac[i];
  DAQmxWriteAnalogF64(outTask, 1, 1, 1.0e-4, DAQmx_Val_GroupByScanNumber, outBuf, &spw, NULL);
  //outBuf[0]= sin(t);
  //DAQmxWriteAnalogF64(outTask, 1, 1, 1.0e-4, DAQmx_Val_GroupByScanNumber, outBuf, &spw, NULL);
}


void NIDAQ::reset_board()
{
 static int32 spw;
  // stop tasks first
 if (DevicePresent) {
   if (outTaskActive) {
      for (int i= 0; i < actOutChnNo; i++) outBuf[i]= 0.0;
      DAQmxWriteAnalogF64(outTask, 1, 1, 1.0e-4, DAQmx_Val_GroupByScanNumber, outBuf, &spw, NULL);
      DAQmxStopTask(outTask);
      DAQmxClearTask(outTask);
      delete[] outBuf;
      outTaskActive= 0;
    }
    if (inTaskActive) {
      DAQmxStopTask(inTask);
      DAQmxClearTask(inTask);
      delete[] inBuf;
      inTaskActive= 0;
    }
    DAQmxResetDevice(devName);
 }
}
