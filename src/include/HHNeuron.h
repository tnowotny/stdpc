/*
 * StdpC is a free dynamic clamp software.
 * Copyright (C) 2020 Thomas Nowotny, Felix Kern
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

#ifndef HHNEURON_H
#define HHNEURON_H

#include "ModelManager.h"

struct HHNeuronData : public ModelData
{
    double C;
    double gLeak;
    double ELeak;
    bool Vlimit;
    double Vmin;
    double Vmax;
    double V0;
    std::vector<vInstData> inst;

    inline const vInstData &instance(size_t i) const { return inst[i]; }
    inline size_t numInst() const { return inst.size(); }
    HHNeuronData() : C(3.5e-9), gLeak(20e-9), ELeak(-20e-3), Vlimit(false), Vmin(-0.2), Vmax(0.2), V0(-60e-3) {}
};

class HHNeuronProxy : public ModelProxy {
private:
    HHNeuronProxy();
public:
    HHNeuronProxy(const HHNeuronProxy &) = delete;
    void operator=(const HHNeuronProxy &) = delete;
    static HHNeuronProxy *get() { static HHNeuronProxy proxy; return &proxy; }

    inline ModelData &param(size_t i) { return p[i]; }
    inline size_t size() { return p.size(); }
    inline void resize(size_t sz) { p.resize(sz); }
    inline void remove(size_t i) { p.erase(p.begin() + i); }

    inline QString modelClass() { return "HH"; }
    inline QString prettyName() { return "HH Model"; }

    Model *instantiate(size_t modelID, size_t instID, DCThread *);
    ModelDlg *createDialog(size_t modelID, QWidget *parent=nullptr);

    static std::vector<HHNeuronData> p;
};


class HHNeuron : public Model
{
public:
    HHNeuron(size_t modelID, size_t instID, DCThread *DCT);
    ~HHNeuron() {}

    void post_init(DCThread *);

    void RK4(double t, double dt, size_t n, bool settling);

    inline const HHNeuronData &params() const { return HHNeuronProxy::p[modID]; }
    HHNeuronProxy *proxy() const;

protected:
    double V;
    double kV[4], Vi;
};

#endif // HHNEURON_H
