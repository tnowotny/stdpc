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

#include "Daq.h"
#include "DeviceManager.h"

DAQ::DAQ(size_t devID, DAQProxy *proxy) :
    devID(devID),
    proxy(proxy),
    t(DAQClock.t),
    digInChnNo(0)
{
  initialized= false;
}


DAQ::~DAQ()
{
}

DAQData *DAQ::params()
{
     return &proxy->param(devID);
}


//---------------------------------------------------------------------------
void DAQ::init_chans()
{
    DAQData *p = params();
    short int No = 0;
    QVector<short int> Chns(p->inChn.size());
    in.resize(p->inChn.size());
    vHiLim.resize(p->inChn.size());
    vLoLim.resize(p->inChn.size());
    inChnLabels.resize(p->inChn.size());
    for ( size_t i = 0; i < p->inChn.size(); i++ ) {
        if ( p->inChn[i].active ) {
            in[i].init(&p->inChn[i]);
            Chns[No++] = i;
            vHiLim[i] = p->inChn[i].minVoltage + upTolFac*(p->inChn[i].maxVoltage - p->inChn[i].minVoltage);
            vLoLim[i] = p->inChn[i].minVoltage + loTolFac*(p->inChn[i].maxVoltage - p->inChn[i].minVoltage);
        } else {
            in[i].V = 0.0;
            in[i].active = false;
        }
    }
    generate_scan_list(No, Chns);

    No = 0;
    out.resize(p->outChn.size());
    iHiLim.resize(p->outChn.size());
    iLoLim.resize(p->outChn.size());
    outChnLabels.resize(p->outChn.size());
    for ( size_t i = 0; i < p->outChn.size(); i++ ) {
        if ( p->outChn[i].active ) {
            out[i].init(&p->outChn[i]);
            Chns[No++] = i;
            iHiLim[i] = p->outChn[i].minCurrent+ upTolFac*(p->outChn[i].maxCurrent- p->outChn[i].minCurrent);
            iLoLim[i] = p->outChn[i].minCurrent + loTolFac*(p->outChn[i].maxCurrent- p->outChn[i].minCurrent);
        } else {
            out[i].I = 0.0;
            out[i].active = false;
        }
    }
    generate_analog_out_list(No, Chns);
}


//---------------------------------------------------------------------------
void DAQ::reset_chans()
{
    for ( outChannel &o : out )
        o.I = 0.0;
}


//---------------------------------------------------------------------------
void DAQ::process_scan(double t)
{
    DAQData *p = params();
    for ( int i = 0; i < actOutChnNo; i++ )
        out[outIdx[i]].I = p->outChn[outIdx[i]].bias;
    for ( int i = 0; i < actInChnNo; i++ ) {
        in[inIdx[i]].V += p->inChn[inIdx[i]].bias;
        in[inIdx[i]].process(t);
    }
}

//---------------------------------------------------------------------------
QPair<QVector<ChannelIndex>, QVector<const double *>> DAQ::valuesToSave()
{
    QVector<ChannelIndex> indices;
    QVector<const double *> values;
    for ( int i = 0; i < actInChnNo; i++ ) {
        if ( in[inIdx[i]].save ) {
            indices.push_back(ChannelIndex(proxy, devID, inIdx[i], true));
            values.push_back(&in[inIdx[i]].V);
        }
    }
    for ( int i = 0; i < actOutChnNo; i++ ) {
        if ( out[outIdx[i]].save ) {
            indices.push_back(ChannelIndex(proxy, devID, outIdx[i], false));
            values.push_back(&out[outIdx[i]].I);
        }
    }
    return qMakePair(indices, values);
}

//---------------------------------------------------------------------------
QVector<AECChannel*> DAQ::aecChans()
{
    QVector<AECChannel*> chans;
    for ( int i = 0; i < actInChnNo; i++ ) {
        if ( in[inIdx[i]].aec.IsActive() ) {
            chans.push_back(&in[inIdx[i]].aec);
        }
    }
    return chans;
}


//---------------------------------------------------------------------------
bool DAQ::check_limits(bool checkV_and_warn, ChannelLimitWarning &w)
{
    bool warn = false;
    for ( int i = 0; i < actOutChnNo; i++ ) {
        if( out[outIdx[i]].I > iHiLim[outIdx[i]] ) {
            if ( checkV_and_warn ) {
                w = {"Current", outChnLabels[outIdx[i]], iHiLim[outIdx[i]], iLoLim[outIdx[i]], out[outIdx[i]].I};
                warn = true;
            }
            out[outIdx[i]].I = iHiLim[outIdx[i]];
        } else if ( out[outIdx[i]].I < iLoLim[outIdx[i]] ) {
            if ( checkV_and_warn ) {
                w = {"Current", outChnLabels[outIdx[i]], iHiLim[outIdx[i]], iLoLim[outIdx[i]], out[outIdx[i]].I};
                warn = true;
            }
            out[outIdx[i]].I = iLoLim[outIdx[i]];
        }
    }
    if ( checkV_and_warn ) for ( int i = 0; i < actInChnNo && !warn; i++ ) {
        if (in[inIdx[i]].V > vHiLim[inIdx[i]] ) {
            w = {"Voltage", inChnLabels[inIdx[i]], vHiLim[inIdx[i]], vLoLim[inIdx[i]], in[inIdx[i]].V};
            warn = true;
        } else if (in[inIdx[i]].V < vLoLim[inIdx[i]] ) {
            w = {"Voltage", inChnLabels[inIdx[i]], vHiLim[inIdx[i]], vLoLim[inIdx[i]], in[inIdx[i]].V};
            warn = true;
        }
    }
    return warn;
}
