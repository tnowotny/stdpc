#ifndef MODELOPTS_H
#define MODELOPTS_H

#include "DaqOptsPrototype.h"

class ModelOpts : public DaqOptsBase
{
public:
    ModelOpts(QWidget *parent, ModelProxy *proxy, size_t idx);
    ~ModelOpts();

    void removeDevice(size_t idx);
    void importData();
    void exportData();
    void setIndex(size_t i);
    inline DaqWidget *widget() { return _widget; }
    void regenerateWidget();

protected:
    void activeChanged();

    ModelProxy *proxy;
    ModelDlg *dlg;
    size_t idx;
    DaqWidget *_widget;
};

class ModelOptsPrototype : public DaqOptsPrototypeBase
{
public:
    ModelOptsPrototype(ModelProxy *proxy) : proxy(proxy) {}
    ~ModelOptsPrototype() {}

    inline QString label() { return proxy->prettyName(); }

    DaqOptsBase *create(size_t idx, QWidget *parent);
    void createAll(QWidget *parent);
    void exportData(bool);

protected:
    ModelProxy *proxy;
};

#endif // MODELOPTS_H
