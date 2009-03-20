#ifndef ABSYN_H
#define ABSYN_H

#include "global_func.h"
#include "Channels.h"

class abSyn {
  private:
    abSynData *p;
    function *theExp;
    function *theTanh;
    function *theExpSigmoid;
    void learn();
    inChannel *pre;
    inChannel *post;
    outChannel *out;

    double P_f(double);
    double D_f(double);
    double invGFilter(double);
    double gFilter(double);
    
  protected:
    double I;
    double S;
    double R;
    double g;
    double graw;
    double P;  // "potentiation var" in ODE STDP
    double D;  // "depression var" in ODE STDP
    double Pslope; // slope of the sigmoid for P
    double Dslope; // slope of the sigmoid for D

  public:
    abSyn();
    void init(abSynData *, short int *, short int *, inChannel *, outChannel *);
    void currentUpdate(double, double);
    void STlearn(double);
    double STDPFunc(double);
    void ODElearn(double);
};

#endif
