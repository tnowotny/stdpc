#ifndef CHEMSYNDLG_H
#define CHEMSYNDLG_H


#include <QAbstractButton>
#include "ui_ChemSynDlg.h"
#include "STDPDlg.h"
#include "ODESTDPDlg.h"
#include "ObjectDataTypes.h"
#include "SynapseAssignmentDlg.h"

class ChemSynDlg : public QDialog, private Ui::ChemSynDlg
{
     Q_OBJECT

  public:
     ChemSynDlg(int, QWidget *parent= 0);
     void exportData(CSynData &);
     void importData(CSynData);

     int No;   
     STDPDlg *STDP;
     ODESTDPDlg *ODESTDP;
     
  public slots:
     void PlastMethodChange();
     void STDComboChange();
     void ResCloseClicked(QAbstractButton *);
     void updateOutChn(int, int*);
     void updateInChn(int, int*);
     
  private:
     SynapseAssignmentDlg sa;
}; 

#endif
