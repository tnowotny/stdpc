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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QVector>
#include <vector>
#include "ObjectDataTypes.h"
#include "LUtables.h"
#include "Clock.h"

#define MAX_ST_HISTORY 20
#define MAX_IN_CHN_NO 64
#define VTHRESH -30

#define STDPC_PROTOCOL_VERSION 3
#define STDPC_PROTOCOL_HEADER "#StdpC_config_version"
extern int LOADED_PROTOCOL_VERSION;
extern QString LEGACY_DAQ_CLASS;

// Forward
class DeviceManager;
class ModelManager;


extern std::vector<CSynData> CSynp;
extern std::vector<abSynData> abSynp;
extern std::vector<GJunctData> ESynp;
extern std::vector<DestexheSynData> DxheSynp;

extern std::vector<mhHHData> mhHHp;
extern std::vector<abHHData> abHHp;

extern dataSavingParams dataSavingPs;

extern PlotData Plotp;
extern PerformanceMonitorData PerfMonp;

// Attilas Sample-and-Hold
extern SampleHoldData SampleHoldp;

// LU tables
extern tanhLUtable tanhLU;
extern expLUtable expLU;
extern expSigmoidLUtable expSigmoidLU;

extern Clock DAQClock;
extern DeviceManager Devices;
extern ModelManager Models;

#endif
