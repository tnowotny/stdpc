#ifndef DCTHREAD_H
#define DCTHREAD_H

#include <QThread>
#include <functional>
#include "Global.h"
#include "ChemSyn.h"
#include "AbSyn.h"
#include "GapJunction.h"
#include "DestexheSyn.h"
#include "HH.h"
#include "AbHH.h"
#include "SpkGen.h"
#include "DigiData.h"
#include "SimulDAQ.h"
#include "Nidaq.h"
#include "AECChannel.h"
#include "DataSaver.h"
#include "HHNeuron.h"


class DCThread : public QThread 
{
     Q_OBJECT

 protected:
     void run();
     
 public slots:

private:
     DataSaver *dataSaver;

 public:
     DCThread();
     virtual ~DCThread();
     bool LoadScript(QString &);
     void UnloadScript();

     inChannel *getInChan(ChannelIndex const& dex);
     outChannel *getOutChan(ChannelIndex const& dex);
     std::vector<ChannelIndex> getChanIndices(ChannelIndex const& dex);

     template <typename T>
     void instantiate(std::vector<T> &, typename T::param_type &, CurrentAssignment &);
     template <typename T>
     void instantiate(std::vector<T> &, typename T::param_type &, SynapseAssignment &);
     template <typename T>
     void instantiate(std::vector<T> &, typename T::param_type &, GapJunctionAssignment &);

     // List of AEC channels (io-channels pairs, kernels and current buffers)
     QVector<AECChannel*> aecChannels;

     // Temporary data storing object
     QVector<double> data;
     QVector<inChannel *> inChnsToSave;
     QVector<outChannel *> outChnsToSave;
     
     bool stopped;
     bool finished;
     bool scripting;
     SpkGen SG;
     std::vector<ChemSyn> csyn;
     std::vector<abSyn> absyn;
     std::vector<GapJunction> esyn;
     std::vector<DestexheSyn> dsyn;
     std::vector<HH> hh;
     std::vector<abHH> abhh;

     inChannel inChnSG;
     outChannel outChnNone;

     std::vector<HHNeuronModel> hhNeuron;
     
 private:
     bool initial;   
     double t;
     double lastT;
     double dt;
     double lastWrite[2];
    
     int grpNo[2];
     int pen[2][4];
     double *grp[2][4];


     QList<QPair<double, std::function<void()>>> scriptq;
     QList<QPair<double, std::function<void()>>>::iterator scrIter;
    
 signals:
     void error(QString message);
     void message(QString message);
     void addPoint1(double, double, int);
     void addPoint2(double, double, int);
     void CloseToLimit(QString, QString, double, double, double);


};


#endif
