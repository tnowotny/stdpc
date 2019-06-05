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

#ifndef ABHH_H
#define ABHH_H

#include "ObjectDataTypes.h"
#include "Channels.h"
#include "LUtables.h"
#include "Global_func.h"

class abHH
{
  private:
    abHHData *p;
    inChannel *pre;
    outChannel *out;
    CurrentAssignment *a;
    stdpc::function *theExp;
    stdpc::function *theExpSigmoid;
    stdpc::function *theTanh;
    
  protected:
    double m, h;
    double ma, mb, ha, hb;
    double I;
    double km[4], kh[4], mi, hi;
    
  public:
    abHH(abHHData *inp, CurrentAssignment *a, inChannel *pre, outChannel *out);
    void currentUpdate(double t, double dt);
    double mhFunc(double, int);
    void RK4(double t, double dt, size_t n);

    typedef abHHData param_type;
};

#endif
