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

#include "VStepsDlg.h"
#include "ui_VStepsDlg.h"
#include "SpkGen.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <fstream>

VStepsDlg::VStepsDlg(size_t idx, QWidget *parent) :
    ModelDlg(idx, parent),
    ui(new Ui::VStepsDlg)
{
    ui->setupUi(this);
    setIndex(idx);
}

VStepsDlg::~VStepsDlg()
{
    delete ui;
}

void VStepsDlg::setIndex(size_t no)
{
    ui->titleLabel->setText(QString("%1 %2").arg(VStepsProxy::get()->prettyName()).arg(no));
    ModelDlg::setIndex(no);
}

void VStepsDlg::importData()
{
    ui->leLabel->setText(VStepsProxy::p[idx].label);
    QString num;
    num.setNum(VStepsProxy::p[idx].holdV*1e3);
    ui->holdVE->setText(num);
    num.setNum(VStepsProxy::p[idx].startV*1e3);
    ui->startVE->setText(num);
    num.setNum(VStepsProxy::p[idx].endV*1e3);
    ui->endVE->setText(num);
    num.setNum(VStepsProxy::p[idx].deltaV*1e3);
    ui->deltaVE->setText(num);
    num.setNum(VStepsProxy::p[idx].t0*1e3);
    ui->t0E->setText(num);
    num.setNum(VStepsProxy::p[idx].tStep*1e3);
    ui->tStepE->setText(num);
    num.setNum(VStepsProxy::p[idx].tHold*1e3);
    ui->tHoldE->setText(num);
    ui->cbSave->setChecked(VStepsProxy::p[idx].inst.inChn.chnlSaving);
}


void VStepsDlg::exportData()
{
    VStepsProxy::p[idx].label = ui->leLabel->text();
    VStepsProxy::p[idx].holdV= ui->holdVE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].startV= ui->startVE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].endV= ui->endVE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].deltaV= ui->deltaVE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].t0= ui->t0E->text().toDouble()*1e-3;
    VStepsProxy::p[idx].tStep= ui->tStepE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].tHold= ui->tHoldE->text().toDouble()*1e-3;
    VStepsProxy::p[idx].inst.inChn.chnlSaving = ui->cbSave->isChecked();

    emit channelsChanged();
    emit modelStatusChanged();
}
