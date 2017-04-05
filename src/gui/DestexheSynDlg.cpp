#include <QDoubleSpinBox>
#include "DestexheSynDlg.h"
#include <QMessageBox>
#include "DestexheSyn.h"

DestexheSynDlg::DestexheSynDlg(int no, QWidget *parent)
     : ConductanceDlg(no, parent)
 {
     setupUi(this);

     STDP= new STDPDlg(this);
     ODESTDP= new ODESTDPDlg(this);

     label = DestexheSynDlgLabel->text();
     setIndex(no);

     connect(PlasticityCombo, SIGNAL(currentIndexChanged(QString)), SLOT(PlastMethodChange()));
     connect(ResCloseBox, SIGNAL(clicked(QAbstractButton *)), SLOT(ResCloseClicked(QAbstractButton *)));

     ChannelListModel *in = ChannelListModel::getModel(ChannelListModel::In | ChannelListModel::Blank);
     ChannelListModel *out = ChannelListModel::getModel(ChannelListModel::Out | ChannelListModel::Blank);

     QVector<AssignmentCellBase<SynapseAssignment>*> vec;
     vec.push_back(new AssignmentCellBool<SynapseAssignment>(&SynapseAssignment::active, "Active", 47));
     vec.push_back(new AssignmentCellChannel<SynapseAssignment>(&SynapseAssignment::PreSynChannel, "Presyn V", 95, in));
     vec.push_back(new AssignmentCellChannel<SynapseAssignment>(&SynapseAssignment::PostSynChannel, "Postsyn V", 95, in));
     vec.push_back(new AssignmentCellChannel<SynapseAssignment>(&SynapseAssignment::OutSynChannel, "Postsyn I", 95, out));
     AssignmentCellDouble<SynapseAssignment> *tmp = new AssignmentCellDouble<SynapseAssignment>
             (&SynapseAssignment::delay, "Delay (ms)", 95);
     tmp->setRange(0., 1000.);
     tmp->setDecimals(3);
     tmp->setFactor(1e-3);
     vec.push_back(tmp);
     assignments->init(vec);
}

void DestexheSynDlg::setIndex(int no)
{
    QString lb = label.arg(no);
    DestexheSynDlgLabel->setText(lb);
    STDP->setLabel(lb);
    ODESTDP->setLabel(lb);
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

void DestexheSynDlg::exportData()
{
  DestexheSynData &p = DestexheSynProxy::p[idx];
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

void DestexheSynDlg::importData()
{
  DestexheSynData &p = DestexheSynProxy::p[idx];
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
