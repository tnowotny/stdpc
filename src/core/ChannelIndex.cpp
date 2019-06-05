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

#include "ChannelIndex.h"
#include <QStringList>
#include <QChar>
#include "Global.h"
#include "ModelManager.h"
#include "DeviceManager.h"

ChannelIndex::ChannelIndex() :
    isValid(false),
    isNone(false),
    isPrototype(false),
    isVirtual(false),
    modelClass(""),
    modelID(0),
    instID(0),
    isAnalog(false),
    daqClass(""),
    devID(0),
    isInChn(true),
    chanID(0),
    isLegacy(false)
{

}

ChannelIndex::ChannelIndex(DAQProxy *proxy, size_t devID, size_t chanID, bool isInChn) :
    isValid(proxy != nullptr),
    isNone(false),
    isPrototype(false),
    isVirtual(false),
    isAnalog(proxy != nullptr),
    daqClass(proxy->daqClass()),
    devID(devID),
    isInChn(isInChn),
    chanID(chanID),
    isLegacy(false)
{}

ChannelIndex::ChannelIndex(ModelProxy *proxy, size_t modelID) :
    isValid(proxy != nullptr),
    isNone(false),
    isPrototype(true),
    isVirtual(false),
    modelClass(proxy->modelClass()),
    modelID(modelID),
    instID(0),
    isAnalog(false),
    isLegacy(false)
{}

ChannelIndex::ChannelIndex(ModelProxy *proxy, size_t modelID, size_t instID) :
    isValid(proxy != nullptr),
    isNone(false),
    isPrototype(false),
    isVirtual(true),
    modelClass(proxy->modelClass()),
    modelID(modelID),
    instID(instID),
    isAnalog(false),
    isLegacy(false)
{}

ChannelIndex ChannelIndex::toInstance(size_t instID) const
{
    if ( !isPrototype || !isValid )
        return *this;
    ChannelIndex dex(*this);
    dex.isPrototype = false;
    dex.isVirtual = true;
    dex.instID = instID;
    return dex;
}

inChnData *ChannelIndex::getInChnData() const
{
    if ( !isValid || !isAnalog || !isInChn )
        return nullptr;
    DAQProxy *proxy = DeviceManager::Register().value(daqClass);
    if ( proxy && proxy->size() > devID && proxy->param(devID).inChn.size() > chanID )
        return &(proxy->param(devID).inChn[chanID]);
    return nullptr;
}

outChnData *ChannelIndex::getOutChnData() const
{
    if ( !isValid || !isAnalog || isInChn )
        return nullptr;
    DAQProxy *proxy = DeviceManager::Register().value(daqClass);
    if ( proxy && proxy->size() > devID && proxy->param(devID).outChn.size() > chanID )
        return &(proxy->param(devID).outChn[chanID]);
    return nullptr;
}

QString ChannelIndex::prettyName() const
{
    QString ret;
    if ( !isValid ) {
        return ret;
    } else if ( isNone ) {
        ret = "None";
    } else if ( isAnalog ) {
        ret = QString("%1 %2 on %3, device %4").arg(isInChn ? "AI" : "AO").arg(chanID).arg(daqClass).arg(devID);
    } else if ( isPrototype || isVirtual ) {
        if ( isPrototype )
            ret = QString("%1 %2:all (model %2, all instances)").arg(modelClass).arg(modelID);
        else
            ret = QString("%1 %2:%3 (model %2, instance %3)").arg(modelClass).arg(modelID).arg(instID);
    } else ret = "Oops: Invalid channel index";
    return ret;
}

QString ChannelIndex::toString(QChar sep) const
{
    QString ret;
    if ( !isValid || isNone ) {
        ret = "None";
    } else if ( isAnalog ) {
        ret = QString("Analog/%1/%2/%3%4").arg(daqClass).arg(devID).arg(isInChn ? "ai" : "ao").arg(chanID);
    } else if ( isPrototype ) {
        ret = QString("Prototype/%1/%2").arg(modelClass).arg(modelID);
    } else if ( isVirtual ) {
        ret = QString("Virtual/%1/%2/%3").arg(modelClass).arg(modelID).arg(instID);
    }
    if ( sep != QChar('/') )
        ret.replace('/', sep);
    return ret;
}

std::ostream &operator<<(std::ostream &os, const ChannelIndex &dex)
{
    os << dex.toString().toStdString();
    return os;
}

std::istream &operator>>(std::istream &is, ChannelIndex &dex)
{
    ChannelIndex blank;
    dex = blank;
    std::string stdstr;
    is >> stdstr;
    QString str = QString::fromStdString(stdstr);
    QStringList parts = str.split("/");
    if ( !parts.at(0).compare("None", Qt::CaseInsensitive) ) {
        dex.isNone = true;
        dex.isValid = true;
    } else if ( !parts.at(0).compare("Analog", Qt::CaseInsensitive) ) {
        if ( parts.size() != 4 )
            return is;

        if ( DeviceManager::Register().contains(parts.at(1)) )
            dex.daqClass = parts.at(1);
        else
            return is;

        if ( parts.at(3).startsWith("ai", Qt::CaseInsensitive) )
            dex.isInChn = true;
        else if ( parts.at(3).startsWith("ao", Qt::CaseInsensitive) )
            dex.isInChn = false;
        else
            return is;

        dex.isAnalog = true;
        dex.devID = parts.at(2).toUInt();
        dex.chanID = parts.at(3).mid(2).toUInt();
        dex.isValid = true;
    } else if ( !parts.at(0).compare("Prototype", Qt::CaseInsensitive) ) {
        if ( parts.size() != 3 )
            return is;

        if ( ModelManager::Register().contains(parts.at(1)) )
            dex.modelClass = parts.at(1);
        else
            return is;

        dex.isPrototype = true;
        dex.modelID = parts.at(2).toUInt();
        dex.isValid = true;
    } else if ( !parts.at(0).compare("Virtual", Qt::CaseInsensitive) ) {
        if ( parts.size() != 4 )
            return is;

        if ( ModelManager::Register().contains(parts.at(1)) )
            dex.modelClass = parts.at(1);
        else
            return is;

        dex.isVirtual = true;
        dex.modelID = parts.at(2).toUInt();
        dex.instID = parts.at(3).toUInt();
        dex.isValid = true;
    } else if ( parts.size() == 1 && LOADED_PROTOCOL_VERSION == 0 ) {
        dex.isLegacy = true;
        dex.chanID = parts.at(0).toInt();
    } else if ( parts.size() == 1 && LOADED_PROTOCOL_VERSION < 3 ) {
        if ( !parts.at(0).compare("SpikeGen", Qt::CaseInsensitive) ) {
            dex.isVirtual = true;
            dex.modelClass = "SG";
            dex.modelID = 0;
            dex.instID = 0;
            dex.isValid = true;
        }
    }
    return is;
}

bool operator==(ChannelIndex const& a, ChannelIndex const& b)
{
    return a.isValid==b.isValid &&
            a.isNone==b.isNone &&
            a.isPrototype==b.isPrototype &&
            a.isVirtual==b.isVirtual &&
            (!a.isPrototype || (a.modelClass==b.modelClass && a.modelID==b.modelID)) &&
            (!a.isVirtual || (a.modelClass==b.modelClass && a.modelID==b.modelID && a.instID==b.instID)) &&
            a.isAnalog==b.isAnalog &&
            (!a.isAnalog || (a.daqClass==b.daqClass && a.devID==b.devID && a.isInChn==b.isInChn && a.chanID==b.chanID)) &&
            a.isLegacy == b.isLegacy;
}
