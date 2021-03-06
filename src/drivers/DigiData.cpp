/*
 * StdpC is a free dynamic clamp software.
 * Copyright (C) 2019 Thomas Nowotny, Felix Kern
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "DigiData.h"
#include "limits.h"
#include "AP.h"
#include "DigiDataDlg.h"

// DigiData 1200 device driver

/// Construct a single self-registering proxy
static DigiDataProxy *prox = DigiDataProxy::get();
std::vector<DigiDataData> DigiDataProxy::p;
DAQ *DigiDataProxy::createDAQ(size_t devID) { return new DigiData(devID, prox); }
DAQDlg *DigiDataProxy::createDialog(size_t devID, QWidget *parent) { return new DigiDataDlg(devID, prox, parent); }

DigiDataProxy::DigiDataProxy() :
    regAP {
        addAP("DigiDatap[#].active", &p, &DigiDataData::active),
        addAP("DigiDatap[#].label", &p, &DigiDataData::label),
        addAP("DigiDatap[#].baseAddress", &p, &DigiDataData::baseAddress),
        addAP("DigiDatap[#].syncIOMask", &p, &DigiDataData::syncIOMask),
        addAP("DigiDatap[#].inChn[#].active", &p, &DigiDataData::inChn, &inChnData::active),
        addAP("DigiDatap[#].inChn[#].gain", &p, &DigiDataData::inChn, &inChnData::gain),
        addAP("DigiDatap[#].inChn[#].gainFac", &p, &DigiDataData::inChn, &inChnData::gainFac),
        addAP("DigiDatap[#].inChn[#].spkDetect", &p, &DigiDataData::inChn, &inChnData::spkDetect),
        addAP("DigiDatap[#].inChn[#].spkDetectThresh", &p, &DigiDataData::inChn, &inChnData::spkDetectThresh),
        addAP("DigiDatap[#].inChn[#].bias", &p, &DigiDataData::inChn, &inChnData::bias),
        addAP("DigiDatap[#].inChn[#].chnlSaving", &p, &DigiDataData::inChn, &inChnData::chnlSaving),
        addAP("DigiDatap[#].outChn[#].active", &p, &DigiDataData::outChn, &outChnData::active),
        addAP("DigiDatap[#].outChn[#].gain", &p, &DigiDataData::outChn, &outChnData::gain),
        addAP("DigiDatap[#].outChn[#].gainFac", &p, &DigiDataData::outChn, &outChnData::gainFac),
        addAP("DigiDatap[#].outChn[#].bias", &p, &DigiDataData::outChn, &outChnData::bias),
        addAP("DigiDatap[#].outChn[#].chnlSaving", &p, &DigiDataData::outChn, &outChnData::chnlSaving),

        addAP("DigiDatap[#].inChn[#].calib.copyChnOn", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::copyChnOn),
        addAP("DigiDatap[#].inChn[#].calib.copyChn", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::copyChn),
        addAP("DigiDatap[#].inChn[#].calib.samplingRate", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::samplingRate),
        addAP("DigiDatap[#].inChn[#].calib.outputChannelNumber", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::outputChannelNumber),
        addAP("DigiDatap[#].inChn[#].calib.iMaxElec", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::iMaxElec),
        addAP("DigiDatap[#].inChn[#].calib.iMinElec", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::iMinElec),
        addAP("DigiDatap[#].inChn[#].calib.numberOfLevels", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::numberOfLevels),
        addAP("DigiDatap[#].inChn[#].calib.injLenPerLevel", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::injLenPerLevel),
        addAP("DigiDatap[#].inChn[#].calib.iMembStep", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::iMembStep),
        addAP("DigiDatap[#].inChn[#].calib.numberOfRepeats", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::numberOfRepeats),
        addAP("DigiDatap[#].inChn[#].calib.injLenPerRepeat", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::injLenPerRepeat),
        addAP("DigiDatap[#].inChn[#].calib.hyperpolCurr", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::hyperpolCurr),
        addAP("DigiDatap[#].inChn[#].calib.injCalAmp", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::injCalAmp),
        addAP("DigiDatap[#].inChn[#].calib.injCalLen", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::injCalLen),
        addAP("DigiDatap[#].inChn[#].calib.fullKernelLen", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::fullKernelLen),
        addAP("DigiDatap[#].inChn[#].calib.electrodeKernelLen", &p, &DigiDataData::inChn, &inChnData::calib, &elecCalibParams::electrodeKernelLen),
    }
{
    DeviceManager::RegisterDAQ(daqClass(), this);
}

//---------------------------------------------------------------------------

DigiData::DigiData(size_t devID, DAQProxy *proxy) :
    DAQ(devID, proxy)
{
  inChnNo= 16;
  inIdx= new short int[inChnNo];
  inGainFac= new double[inChnNo];
  outChnNo= 2;       
  outIdx= new short int[outChnNo];
  outGainFac= new double[outChnNo];
  inGainNo= 4;
  inLow= QVector<double>(inGainNo);
  inLow[0]= -10.24;
  inLow[1]= -5.12;
  inLow[2]= -2.56;
  inLow[3]= -1.28;
  inHigh= QVector<double>(inGainNo);
  inHigh[0]= 10.24;
  inHigh[1]= 5.12;
  inHigh[2]= 2.56;
  inHigh[3]= 1.28;

  inGainText= new char*[inGainNo];
  inGainText[0]= new char[80];
  strcpy(inGainText[0], "-10.24 to +10.24 V");
  inGainText[1]= new char[80];
  strcpy(inGainText[1], "-5.12 to +5.12 V");
  inGainText[2]= new char[80];
  strcpy(inGainText[2], "-2.56 to +2.56 V");
  inGainText[3]= new char[80];
  strcpy(inGainText[3], "-1.28 to +1.28 V");
  outGainNo= 1;
  outLow= QVector<double>(outGainNo);
  outLow[0]= -10.24;
  outHigh= QVector<double>(outGainNo);
  outHigh[0]= 10.24;

  outGainText= new char*[outGainNo];
  outGainText[0]= new char[80];
  strcpy(outGainText[0], "-10.24 to +10.24 V");

  inChnGain= new short int[inGainNo];
  inChnGain[0]= 0x0000;
  inChnGain[1]= 0x2000;
  inChnGain[2]= 0x4000;
  inChnGain[3]= 0x6000;

  DACEnable= new short int[outChnNo];  
  DACEnable[0]= DACCHANNEL0ENABLE;
  DACEnable[1]= DACCHANNEL1ENABLE;
  portsOpen= false;
  init();
}

DigiData::~DigiData()
{
  //  inGain.~QVector();
  for (int i= 0; i < inGainNo; i++) {
    delete[] inGainText[i];
  }
  delete[] inGainText;
  //  outGain.~QVector();
  for (int i= 0; i < outGainNo; i++) {
    delete[] outGainText[i];
  }
  delete[] outGainText;
  delete[] inChnGain;
  delete[] DACEnable;
  delete[] inIdx;
  delete[] inGainFac;
  delete[] outIdx;
  delete[] outGainFac;
  ClosePortTalk();
}

void DigiData::init()
{
   short int base= DigiDataProxy::p[devID].baseAddress;
   base_address=       base;
   DAC_data=           base | DACDATA;
   ADC_data=           base | ADCDATA;
   board_ID=           base | BOARDID;
   timer_data=         base | TIMERDATA;
   timer_control=      base | TIMERCONTROL;
   timer_status=       base | TIMERSTATUS;
   ADCDAC_control=     base | ADCDACCONTROL;
   INTDMA_control=     base | INTDMACONTROL;
   digital_IO=         base | DIGITALIO;
   channel_scan_list=  base | CHANNELSCANLIST;
   real_time_data_0=   base | REALTIMEDATA0;
   real_time_data_1=   base | REALTIMEDATA1;
   real_time_data_2=   base | REALTIMEDATA2;
   real_time_control=  base | REALTIMECONTROL;
   ADCDAC_status=      base | ADCDACSTATUS;
   reset_control=      base | RESETCONTROL;
}

//---------------------------------------------------------------------------
bool DigiData::initialize_board(QString &name)
{
   unsigned char BoardType;
   bool success= false;

   if (!portsOpen) {
     OpenPortTalk();
     portsOpen= true;
   }
   BoardType= ReadByte(board_ID);
   if(BoardType == DIGIDATA1200ID)
   {
      WriteWord(ADCDAC_control,DD1200AMODE);
      BoardType= ReadByte(board_ID);
   }
   if(BoardType == DIGIDATA1200AID)
   {
      WriteWord(ADCDAC_control,DD1200AMODE);
      WriteWord(INTDMA_control,ZEROWORD);
      WriteWord(reset_control,RESETWHOLEBOARD);
      success= true;
   }

   // writing 0 to the DAC outputs
   WriteWord(ADCDAC_control,DACCHANNEL0ENABLE);
   WriteWord(DAC_data,0x0000);

   WriteWord(ADCDAC_control,DACCHANNEL1ENABLE);
   WriteWord(DAC_data,0x0000);

   name= QString("DigiData1200(A) ");
   
   initialized= success;
   return success;
}


//---------------------------------------------------------------------------
void DigiData::digital_out(unsigned char outbyte)
{
   WriteByte(digital_IO,outbyte);
}


//---------------------------------------------------------------------------
void DigiData::generate_scan_list(short int chnNo, QVector<short> Chns)
{
  short int i, Chan_Gain_Code;
  DAQData *p = params();
  ChannelIndex dex(prox, devID, 0, true);

  actInChnNo= chnNo;  
  WriteWord(ADCDAC_control, ADCSCANLISTENABLE);
  for(i= 0; i < actInChnNo; i++)
  {
    inIdx[i]= Chns[i];
    inGainFac[i]= p->inChn[inIdx[i]].gainFac*inHigh[p->inChn[inIdx[i]].gain]/16.0/COUNTS; // divide by 16.0 b/c of 12 bit in chns
    Chan_Gain_Code= i + inChnGain[p->inChn[inIdx[i]].gain] + (inIdx[i] * CHANNELSHIFT);
    if(i == (actInChnNo -1)) Chan_Gain_Code+= LASTCHANNELFLAG;
    WriteWord(channel_scan_list, Chan_Gain_Code);
    dex.chanID = inIdx[i];
    inChnLabels[inIdx[i]] = dex.toString();
  }
  WriteWord(ADCDAC_control, ADCSCANLISTDISABLE);
}

//---------------------------------------------------------------------------
void DigiData::get_scan(bool)
{
   short int i, scan;

   for(i= 0; i < actInChnNo; i++) WriteWord(reset_control, ADCSTARTCONVERT);

   for(i= 0; i < actInChnNo; i++){
      scan= ReadWord(ADC_data);
      in[inIdx[i]].V= inGainFac[i] * (double) scan;
   }
}

void DigiData::get_single_scan(inChannel *in)
{
   short int i, scan;

   for(i= 0; i < actInChnNo; i++) WriteWord(reset_control, ADCSTARTCONVERT);

   for(i= 0; i < actInChnNo; i++){
      scan= ReadWord(ADC_data);
      if (&(this->in[inIdx[i]]) == in) {
          in->V= inGainFac[i] * (double) scan;
      }
   }
}


//---------------------------------------------------------------------------
void DigiData::generate_analog_out_list(short int chnNo, QVector<short int> Chns)
{
  DAQData *p = params();
  ChannelIndex dex(prox, devID, 0, false);

  // collect the active out channels
  actOutChnNo= chnNo;
  for (int i= 0; i < actOutChnNo; i++) {
    outIdx[i]= Chns[i];
    outGainFac[i]= p->outChn[outIdx[i]].gainFac/outHigh[p->outChn[outIdx[i]].gain]*1e9*COUNTS;
    dex.chanID = outIdx[i];
    outChnLabels[outIdx[i]] = dex.toString();
  }
}


//---------------------------------------------------------------------------
void DigiData::write_analog_out(bool)
{
  static short int int_I;
  for (int i= 0; i < actOutChnNo; i++) {
    int_I= ((short int) (out[outIdx[i]].I*outGainFac[i]))*16;
    int_I|= DigiDataProxy::p[devID].syncIOMask; // write synchronous digital IO
    WriteWord(ADCDAC_control,DACEnable[outIdx[i]]);
    WriteWord(DAC_data,int_I);
  } 
}


//---------------------------------------------------------------------------
void DigiData::reset_board() 
{
  for (int i= 0; i < outChnNo; i++) {
    WriteWord(ADCDAC_control,DACEnable[outIdx[i]]);
    WriteWord(DAC_data,0);
  } 
}
