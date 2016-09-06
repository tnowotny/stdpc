
#include "DestexheSynDlg.h"
#include <QMessageBox>

DestexheSynDlg::DestexheSynDlg(int no, ChannelListModel *in, ChannelListModel *out, QWidget *parent)
     : QDialog(parent)
 {
     setupUi(this);

     No= no;
     DestexheSynDlgLabel->setText(DestexheSynDlgLabel->text().arg(no));

     STDP= new STDPDlg(this, No);
     ODESTDP= new ODESTDPDlg(this, No);

     connect(PlasticityCombo, SIGNAL(currentIndexChanged(QString)), SLOT(PlastMethodChange()));
     connect(ResCloseBox, SIGNAL(clicked(QAbstractButton *)), SLOT(ResCloseClicked(QAbstractButton *)));

     QVector<Dropdown<SynapseAssignment>> vec;
     vec.push_back(Dropdown<SynapseAssignment>(&SynapseAssignment::PreSynChannel, in, "Presyn V", 95));
     vec.push_back(Dropdown<SynapseAssignment>(&SynapseAssignment::PostSynChannel, in, "Postsyn V", 95));
     vec.push_back(Dropdown<SynapseAssignment>(&SynapseAssignment::OutSynChannel, out, "Postsyn I", 95));
     assignments->init(vec);
}

void DestexheSynDlg::ResCloseClicked(QAbstractButton *but)
{
  if (but->text() == QString("Close")) {
    hide();
  }
  if (but->text() == QString("Reset")) {
//    reset();
  }
}

void DestexheSynDlg::PlastMethodChange()
{
  int index= PlasticityCombo->currentIndex();
  STDP->hide();
  ODESTDP->hide();
  PlastParaBut->disconnect();

  if (index == 0) {
    PlastParaBut->setEnabled(false);
  }
  else {
    PlastParaBut->setEnabled(true);
  }

  if (index == 1) {
    connect(PlastParaBut, SIGNAL(clicked()), STDP, SLOT(show()));
  }
  if (index == 2) {
    connect(PlastParaBut, SIGNAL(clicked()), ODESTDP, SLOT(show()));
  }
}

void DestexheSynDlg::exportData(DestexheSynData &p)
{
  p.LUTables= (LUCombo->currentIndex() == 1);
  p.gSyn= gSynE->text().toDouble()*1e-9;
  p.Vpre= VpreE->text().toDouble()*1e-3;
  p.Vrev= VrevE->text().toDouble()*1e-3;
  p.trelease= treleaseE->text().toDouble()*1e-3;
  p.alpha= alphaE->text().toDouble()*1e3;
  p.beta= betaE->text().toDouble()*1e3;
  p.fixVpost= fixVpostCombo->currentIndex();
  p.Vpost= VpostE->text().toDouble()*1e-3;
  p.Plasticity= PlasticityCombo->currentIndex();
  // ST plasticity
  STDP->exportData(p.ST);
  // ODE plasticity
  ODESTDP->exportData(p.ODE);

  assignments->exportData(p.assign);
}

void DestexheSynDlg::importData(DestexheSynData p)
{
  QString num;
  if (p.LUTables) LUCombo->setCurrentIndex(1);
  else LUCombo->setCurrentIndex(0);
  num.setNum(p.gSyn*1e9);
  gSynE->setText(num);
  num.setNum(p.Vpre*1e3);
  VpreE->setText(num);
  num.setNum(p.Vrev*1e3);
  VrevE->setText(num);
  num.setNum(p.trelease*1e3);
  treleaseE->setText(num);
  num.setNum(p.alpha*1e-3);
  alphaE->setText(num);
  num.setNum(p.beta*1e-3);
  betaE->setText(num);
  fixVpostCombo->setCurrentIndex(p.fixVpost);
  num.setNum(p.Vpost*1e3);
  VpostE->setText(num);
  PlasticityCombo->setCurrentIndex(p.Plasticity);
  // ST plasticity
  STDP->importData(p.ST);
  // ODE plasticity
  ODESTDP->importData(p.ODE);

  assignments->importData(p.assign);
}
