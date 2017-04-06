#include "ConductanceManager.h"
#include "DCThread.h"

ConductanceManager::~ConductanceManager()
{
    for ( Conductance *c : preD )
        delete c;
    for ( Conductance *c : inD )
        delete c;
    for ( Conductance *c : postD )
        delete c;
}

void ConductanceManager::init(DCThread *DCT)
{
    for ( Conductance *c : preD )
        delete c;
    for ( Conductance *c : inD )
        delete c;
    for ( Conductance *c : postD )
        delete c;
    preD.clear();
    inD.clear();
    postD.clear();

    for ( ConductanceProxy *proxy : Register() ) {
        for ( size_t j = 0; j < proxy->size(); j++ ) {
            if ( proxy->param(j).active ) {
                for ( size_t i = 0; i < proxy->param(j).numAssignments(); i++ ) {
                    if ( proxy->param(j).assignment(i).active ) {
                        proxy->instantiate(j, i, DCT, preD, inD, postD);
                    }
                }
            }
        }
    }
}

void ConductanceManager::clear()
{
    for ( Conductance *c : preD )
        delete c;
    for ( Conductance *c : inD )
        delete c;
    for ( Conductance *c : postD )
        delete c;
    preD.clear();
    inD.clear();
    postD.clear();
    for ( ConductanceProxy *proxy : Register() )
        while ( proxy->size() )
            proxy->remove(proxy->size());
}

const double *match(const std::vector<Conductance*> ref, const ChannelIndex &dex)
{
    for ( Conductance *c : ref )
        if ( c->proxy()->conductanceClass() == dex.conductanceClass &&
             c->conductanceID() == dex.conductanceID &&
             c->assignmentID() == dex.assignID )
            return &c->conductance();
    return nullptr;
}

const double *ConductanceManager::conductance(const ChannelIndex &dex)
{
    const double *ret = nullptr;
    if ( !dex.isConductance )
        return ret;
    if ( (ret = match(preD, dex)) || (ret = match(inD, dex)) || (ret = match(postD, dex)) )
        return ret;
    return ret;
}

QStringList ConductanceManager::getStatus() const
{
    QStringList statuses;
    for ( ConductanceProxy *proxy : Register() ) {
        int euler = 0, rk4 = 0;
        for ( Conductance *c : preD )
            if ( c->proxy() == proxy )
                ++euler;
        for ( Conductance *c : inD )
            if ( c->proxy() == proxy )
                ++rk4;
        for ( Conductance *c : postD )
            if ( c->proxy() == proxy )
                ++euler;
        if ( euler+rk4 > 0 )
            statuses << QString("DC: %1 %2 conductances (%3 Euler, %4 Runge-Kutta)")
                        .arg(euler+rk4)
                        .arg(proxy->prettyName())
                        .arg(euler)
                        .arg(rk4);
    }
    return statuses;
}

QPair<QVector<QString>, QVector<const double *>> ConductanceManager::toSave() const
{
    QVector<QString> labels;
    QVector<const double *> values;
    for ( auto&& any : {preD, inD, postD} ) {
        for ( Conductance *c : any ) {
            if ( c->assignment().save ) {
                labels.push_back(QString("%1_%2_g%3_multi%4")
                                 .arg(c->proxy()->conductanceClass())
                                 .arg(c->conductanceID())
                                 .arg(c->assignmentID())
                                 .arg(c->multiplexID()));
                values.push_back(&c->conductance());
            }
        }
    }
    return qMakePair(labels, values);
}
