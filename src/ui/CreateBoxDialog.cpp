#include "CreateBoxDialog.h"

#include <Standard_WarningsDisable.hxx>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

CreateBoxDialog::CreateBoxDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Create Box");

  myDx = new QDoubleSpinBox(this);
  myDy = new QDoubleSpinBox(this);
  myDz = new QDoubleSpinBox(this);
  for (auto sb : {myDx, myDy, myDz}) {
    sb->setRange(0.001, 1e6);
    sb->setDecimals(3);
    sb->setSingleStep(1.0);
    sb->setValue(10.0);
  }

  QFormLayout* form = new QFormLayout();
  form->addRow("X (dx)", myDx);
  form->addRow("Y (dy)", myDy);
  form->addRow("Z (dz)", myDz);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

double CreateBoxDialog::dx() const { return myDx->value(); }
double CreateBoxDialog::dy() const { return myDy->value(); }
double CreateBoxDialog::dz() const { return myDz->value(); }

