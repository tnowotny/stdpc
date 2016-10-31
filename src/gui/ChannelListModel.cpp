#include "ChannelListModel.h"
#include "Global.h"
#include "ChannelIndex.h"
#include "DeviceManager.h"

ChannelListModel::ChannelListModel(int displayFlags, QObject *parent)
    : QAbstractListModel(parent),
      displayFlags(displayFlags),
      size(0),
      hSimul(DAQHelper<SDAQData>(this)),
      hDD1200(DAQHelper<DigiDataData>(this)),
#ifdef NATIONAL_INSTRUMENTS
      hNI(DAQHelper<NIDAQData>(this))
#endif
{
    for ( ModelProxy *proxy : ModelManager::Register )
        modelHelpers.push_back(ModelHelper(proxy, this));
    connect(&Devices, SIGNAL(removedDevice(ChannelIndex)), this, SLOT(updateChns(ChannelIndex)));
    // Owner connects model removals
}

void ChannelListModel::updateCount(ChannelListModel *from)
{
    if ( from ) {
        size = from->size;

        for ( size_t i = 0; i < modelHelpers.size(); i++ )
            modelHelpers[i].nInst = from->modelHelpers[i].nInst;

        hSimul.nAI = from->hSimul.nAI;
        hSimul.nAO = from->hSimul.nAO;
        hDD1200.nAI = from->hDD1200.nAI;
        hDD1200.nAO = from->hDD1200.nAO;
#ifdef NATIONAL_INSTRUMENTS
        hNI.nAI = from->hNI.nAI;
        hNI.nAO = from->hNI.nAO;
#endif
    } else {
        size = 0;
        if ( displayFlags & Blank )
            size++;
        if ( displayFlags & None )
            size++;

        for ( ModelHelper &h : modelHelpers )
            h.updateCount();

        hSimul.updateCount();
        hDD1200.updateCount();
#ifdef NATIONAL_INSTRUMENTS
        hNI.updateCount();
#endif
    }
}

template <typename T>
void ChannelListModel::DAQHelper<T>::updateCount()
{
    if ( parent->displayFlags & AnalogIn ) {
        nAI.clear();
        for ( DAQ *daq : Devices.get<T>() ) {
            int s = daq->params()->inChn.size();
            nAI.push_back(s);
            parent->size += s;
        }
    }
    if ( parent->displayFlags & AnalogOut ) {
        nAO.clear();
        for ( DAQ *daq : Devices.get<T>() ) {
            int s = daq->params()->outChn.size();
            nAO.push_back(s);
            parent->size += s;
        }
    }
}

void ChannelListModel::ModelHelper::updateCount()
{
    nInst.resize(proxy->size());
    if ( parent->displayFlags & Prototype ) {
        parent->size += nInst.size();
    }
    if ( parent->displayFlags & Virtual ) {
        for ( size_t i = 0; i < proxy->size(); i++ ) {
            nInst[i] = proxy->param(i).numInst();
            parent->size += proxy->param(i).numInst();
        }
    }
}

void ChannelListModel::updateChns(ChannelIndex removeDeviceDex)
{
    rmDevDex = removeDeviceDex;
    QModelIndexList currentIdx, newIdx;
    ChannelListModel newM(displayFlags);
    newM.updateCount();

    // Ignore always-unchanged Blank, None

    for ( ModelHelper &h : modelHelpers )
        h.updateChns(currentIdx, newIdx, newM);

    hSimul.updateChns(currentIdx, newIdx, newM);
    hDD1200.updateChns(currentIdx, newIdx, newM);
#ifdef NATIONAL_INSTRUMENTS
    hNI.updateChns(currentIdx, newIdx, newM);
#endif

    emit layoutAboutToBeChanged();
    updateCount(&newM);
    rmDevDex = ChannelIndex();
    changePersistentIndexList(currentIdx, newIdx);
    emit layoutChanged();
}

template <typename T>
void ChannelListModel::DAQHelper<T>::updateChns(QModelIndexList &currentIdx, QModelIndexList &newIdx, ChannelListModel &newM)
{
    bool mvDev;
    int mvOffset = 0;
    if ( parent->displayFlags & AnalogIn ) {
        ChannelIndex dex(T::daqClass, 0, true);
        for ( dex.devID = 0; dex.devID < nAI.size(); dex.devID++ ) {
            mvDev = parent->rmDevDex.isValid && parent->rmDevDex.isAnalog
                    && parent->rmDevDex.daqClass == T::daqClass
                    && parent->rmDevDex.devID == dex.devID;
            for ( dex.chanID = 0; dex.chanID < nAI[dex.devID]; dex.chanID++ ) {
                currentIdx.append(parent->index(dex, AnalogIn));
                if ( mvDev ) {
                    newIdx.append(QModelIndex());
                } else {
                    dex.devID -= mvOffset;
                    newIdx.append(parent->createIndex(newM.index(dex, AnalogIn).row(), 0));
                    dex.devID += mvOffset;
                }
            }
            if ( mvDev )
                mvOffset = 1;
        }
    }

    mvOffset = 0;
    if ( parent->displayFlags & AnalogOut ) {
        ChannelIndex dex(T::daqClass, 0, false);
        for ( dex.devID = 0; dex.devID < nAO.size(); dex.devID++ ) {
            mvDev = parent->rmDevDex.isValid && parent->rmDevDex.isAnalog
                    && parent->rmDevDex.daqClass == T::daqClass
                    && parent->rmDevDex.devID == dex.devID;
            for ( dex.chanID = 0; dex.chanID < nAO[dex.devID]; dex.chanID++ ) {
                currentIdx.append(parent->index(dex, AnalogOut));
                if ( mvDev ) {
                    newIdx.append(QModelIndex());
                } else {
                    dex.devID -= mvOffset;
                    newIdx.append(parent->createIndex(newM.index(dex, AnalogOut).row(), 0));
                    dex.devID += mvOffset;
                }
            }
            if ( mvDev )
                mvOffset = 1;
        }
    }
}

void ChannelListModel::ModelHelper::updateChns(QModelIndexList &currentIdx, QModelIndexList &newIdx, ChannelListModel &newM)
{
    bool rmModel;
    int mvOffset = 0;
    if ( parent->displayFlags & Prototype ) {
        ChannelIndex dex(ChannelIndex::Prototype, proxy->modelClass());
        for ( dex.modelID = 0; dex.modelID < nInst.size(); dex.modelID++ ) {
            currentIdx.append(parent->index(dex, Prototype));
            if ( parent->rmDevDex == ChannelIndex(ChannelIndex::Prototype, proxy->modelClass(), dex.modelID) ) {
                newIdx.append(QModelIndex());
                mvOffset = 1;
            } else {
                dex.modelID -= mvOffset;
                newIdx.append(parent->createIndex(newM.index(dex, Prototype).row(), 0));
                dex.modelID += mvOffset;
            }
        }
    }

    mvOffset = 0;
    if ( parent->displayFlags & Virtual ) {
        ChannelIndex dex(ChannelIndex::Virtual, proxy->modelClass());
        for ( dex.modelID = 0; dex.modelID < nInst.size(); dex.modelID++ ) {
            rmModel = parent->rmDevDex == ChannelIndex(ChannelIndex::Prototype, proxy->modelClass(), dex.modelID);
            for ( dex.instID = 0; dex.instID < nInst[dex.modelID]; dex.instID++ ) {
                currentIdx.append(parent->index(dex, Virtual));
                if ( rmModel ) {
                    newIdx.append(QModelIndex());
                } else {
                    dex.modelID -= mvOffset;
                    newIdx.append(parent->createIndex(newM.index(dex, Virtual).row(), 0));
                    dex.modelID += mvOffset;
                }
            }
            if ( rmModel )
                mvOffset = 1;
        }
    }
}

int ChannelListModel::rowCount(const QModelIndex &) const
{
    return size;
}

QVariant ChannelListModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if ( !index.isValid() && role == Qt::UserRole ) {
        ret.setValue(ChannelIndex(true));
        return ret;
    }
    if (!index.isValid() || !(role==Qt::DisplayRole || role>=Qt::UserRole))
        return QVariant();

    int offset = 0, row = index.row();
    if ( displayFlags & Blank ) {
        if ( row == offset ) {
            ret.setValue(ChannelIndex(true));
            switch ( role ) {
            case Qt::DisplayRole:   return QString();
            case Qt::UserRole:      return ret;
            case Qt::UserRole + 1:  return QVariant(true);
            }
        }
        offset++;
    }
    if ( displayFlags & None ) {
        if ( row == offset ) {
            ret.setValue(ChannelIndex(true));
            switch ( role ) {
            case Qt::DisplayRole:   return QString("None");
            case Qt::UserRole:      return ret;
            case Qt::UserRole + 1:  return QVariant(true);
            }
        }
        offset++;
    }
    for ( ModelHelper const& h : modelHelpers )
        if ( h.data(row, role, offset, ret) )
            return ret;
    if ( hSimul.data(row, role, offset, ret) )
        return ret;
    if ( hDD1200.data(row, role, offset, ret) )
        return ret;
#ifdef NATIONAL_INSTRUMENTS
    if ( hNI.data(row, role, offset, ret) )
        return ret;
#endif

    if ( role == Qt::UserRole ) {
        ret.setValue(ChannelIndex(true));
        return ret;
    } else {
        return QVariant();
    }
}

template <typename T>
bool ChannelListModel::DAQHelper<T>::data(int row, int role, int &offset, QVariant &ret) const
{
    bool rmDev;
    int mvOffset = 0;
    if ( parent->displayFlags & AnalogIn ) {
        for ( int i = 0; i < nAI.size(); i++ ) {
            // parent->rmDevDex comes through parent->updateChns(rmDevDex), called when a device is removed.
            // During the update process, data(...) is called while ChannelListModel internals and the caller
            // (i.e. the dropdown's ListView) are stale, but the outside world (i.e. Devices, params) has already changed.
            // Thus, we need to adjust indexing for calls to the outside world, omitting the rmDevDex device and shifting
            // its later siblings down into its space.
            // Correctness of data returned probably isn't too important, as this occurs /before/ the model is
            // fully updated, but let's give it the new values anyway, just in case it sticks.
            rmDev = parent->rmDevDex.isValid && parent->rmDevDex.isAnalog
                    && parent->rmDevDex.daqClass == T::daqClass
                    && parent->rmDevDex.devID == i;
            if ( row-offset < nAI[i] ) { // Caller and internal: Stale index
                if ( rmDev ) { // Stale caller meets removed device: Return rubbish
                    ret = QVariant();
                    return true;
                }
                ChannelIndex dex(T::daqClass, i - mvOffset, true, row - offset); // DevID display value: Adjusted index
                DAQData *p = Devices.get<T>()[dex.devID]->params(); // Outside world: Adjusted index
                switch ( role ) {
                case Qt::DisplayRole:   ret = dex.prettyName();                         return true;
                case Qt::UserRole:      ret.setValue(dex);                              return true;
                case Qt::UserRole + 1:  ret = p->active && p->inChn[dex.chanID].active; return true;
                }
            }
            offset += nAI[i];
            if ( rmDev ) // Stale caller meets moved device(s): Adjust external indices accordingly
                mvOffset = 1;
        }
    }

    mvOffset = 0;
    if ( parent->displayFlags & AnalogOut ) {
        for ( int i = 0; i < nAO.size(); i++ ) {
            rmDev = parent->rmDevDex.isValid && parent->rmDevDex.isAnalog
                    && parent->rmDevDex.daqClass == T::daqClass
                    && parent->rmDevDex.devID == i;
            if ( row-offset < nAO[i] ) {
                if ( rmDev ) {
                    ret = QVariant();
                    return true;
                }
                ChannelIndex dex(T::daqClass, i - mvOffset, false, row - offset);
                DAQData *p = Devices.get<T>()[dex.devID]->params();
                switch ( role ) {
                case Qt::DisplayRole:   ret = dex.prettyName();                          return true;
                case Qt::UserRole:      ret.setValue(dex);                               return true;
                case Qt::UserRole + 1:  ret = p->active && p->outChn[dex.chanID].active; return true;
                }
            }
            offset += nAO[i];
            if ( rmDev )
                mvOffset = 1;
        }
    }
    return false;
}

bool ChannelListModel::ModelHelper::data(int row, int role, int &offset, QVariant &ret) const
{
    int mvOffset = 0;
    if ( parent->displayFlags & Prototype ) {
        if ( row-offset < (int)nInst.size() ) {
            if ( parent->rmDevDex.isValid && parent->rmDevDex.isPrototype && parent->rmDevDex.modelClass == proxy->modelClass() ) {
                if ( (int)parent->rmDevDex.modelID == row-offset ) {
                    ret = QVariant();
                    return true;
                } else if ( (int)parent->rmDevDex.modelID < row-offset ) {
                    mvOffset = 1;
                }
            }
            ChannelIndex dex(ChannelIndex::Prototype, proxy->modelClass(), row - offset - mvOffset);
            switch ( role ) {
            case Qt::DisplayRole:   ret = dex.prettyName();                 return true;
            case Qt::UserRole:      ret.setValue(dex);                      return true;
            case Qt::UserRole + 1:  ret = proxy->param(dex.modelID).active; return true;
            }
        }
        offset += nInst.size();
    }

    bool rmModel;
    mvOffset = 0;
    if ( parent->displayFlags & Virtual ) {
        for ( size_t i = 0; i < nInst.size(); i++ ) {
            rmModel = parent->rmDevDex == ChannelIndex(ChannelIndex::Prototype, proxy->modelClass(), i);
            if ( row-offset < (int)nInst[i] ) {
                if ( rmModel ) {
                    ret = QVariant();
                    return true;
                }
                ChannelIndex dex(ChannelIndex::Virtual, proxy->modelClass(), i - mvOffset, row - offset);
                ModelData &p = proxy->param(dex.modelID);
                switch ( role ) {
                case Qt::DisplayRole:   ret = dex.prettyName();                          return true;
                case Qt::UserRole:      ret.setValue(dex);                               return true;
                case Qt::UserRole + 1:  ret = p.active && p.instance(dex.instID).active; return true;
                }
            }
            offset += nInst[i];
            if ( rmModel )
                mvOffset = 1;
        }
    }
    return false;
}

Qt::ItemFlags ChannelListModel::flags(const QModelIndex &index) const
{
    return data(index, Qt::UserRole + 1).toBool()
            ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
            : Qt::NoItemFlags;
}

int ChannelListModel::index(const ChannelIndex &dex) const
{
    ChannelType type = displayFlags & Blank ? Blank : None;
    if ( dex.isPrototype )
        type = Prototype;
    else if ( dex.isVirtual )
        type = Virtual;
    else if ( dex.isNone )
        type = None;
    else if ( dex.isAnalog && !dex.isInChn ) {
        type = AnalogOut;
    } else if ( dex.isAnalog && dex.isInChn ) {
        type = AnalogIn;
    }
    return index(dex, type).row();
}

QModelIndex ChannelListModel::index(const ChannelIndex &dex, ChannelType type) const {
    int row = 0;
    // Walk the list
    if ( type & displayFlags ) {
        if ( displayFlags & Blank ) {
            if ( type & Blank )
                return createIndex(row, 0);
            row++;
        }
        if ( displayFlags & None ) {
            if ( type & None )
                return createIndex(row, 0);
            row++;
        }
        QModelIndex ret;
        for ( ModelHelper const& h : modelHelpers )
            if ( h.index(dex, type, row, ret) )
                return ret;
        if ( hSimul.index(dex, type, row, ret) )
            return ret;
        if ( hDD1200.index(dex, type, row, ret) )
            return ret;
#ifdef NATIONAL_INSTRUMENTS
        if ( hNI.index(dex, type, row, ret) )
            return ret;
#endif
    }
    return QModelIndex();
}

template <typename T>
bool ChannelListModel::DAQHelper<T>::index(const ChannelIndex &dex, ChannelType type, int &offset, QModelIndex &ret) const
{
    if ( parent->displayFlags & AnalogIn ) {
        for ( int i = 0; i < nAI.size(); i++ ) {
            if ( type & AnalogIn && dex.daqClass == T::daqClass && dex.devID == i ) {
                if ( dex.chanID >= 0 && dex.chanID < nAI[i] )
                    ret = parent->createIndex(dex.chanID + offset, 0);
                return true;
            }
            offset += nAI[i];
        }
    }
    if ( parent->displayFlags & AnalogOut ) {
        for ( int i = 0; i < nAO.size(); i++ ) {
            if ( type & AnalogOut && dex.daqClass == T::daqClass && dex.devID == i ) {
                if ( dex.chanID >= 0 && dex.chanID < nAO[i] )
                    ret = parent->createIndex(dex.chanID + offset, 0);
                return true;
            }
            offset += nAO[i];
        }
    }
    return false;
}

bool ChannelListModel::ModelHelper::index(const ChannelIndex &dex, ChannelType type, int &offset, QModelIndex &ret) const
{
    if ( parent->displayFlags & Prototype ) {
        if ( type & Prototype && dex.modelClass == proxy->modelClass() ) {
            if ( dex.modelID < nInst.size() )
                ret = parent->createIndex(dex.modelID + offset, 0);
            return true;
        }
        offset += nInst.size();
    }
    if ( parent->displayFlags & Virtual ) {
        for ( size_t i = 0; i < nInst.size(); i++ ) {
            if ( type & Virtual && dex.modelClass == proxy->modelClass() && dex.modelID == i ) {
                if ( dex.instID < nInst[i] )
                    ret = parent->createIndex(dex.instID + offset, 0);
                return true;
            }
            offset += nInst[i];
        }
    }
    return false;
}
