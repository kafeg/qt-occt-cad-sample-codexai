#include "CreateCylinderDialog.h"

#include <Standard_WarningsDisable.hxx>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

CreateCylinderDialog::CreateCylinderDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Create Cylinder");

  myRadius = new QDoubleSpinBox(this);
  myHeight = new QDoubleSpinBox(this);
  for (auto sb : {myRadius, myHeight}) {
    sb->setRange(0.001, 1e6);
    sb->setDecimals(3);
    sb->setSingleStep(1.0);
  }
  myRadius->setValue(10.0);
  myHeight->setValue(20.0);

  QFormLayout* form = new QFormLayout();
  form->addRow("Radius", myRadius);
  form->addRow("Height", myHeight);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

double CreateCylinderDialog::radius() const { return myRadius->value(); }
double CreateCylinderDialog::height() const { return myHeight->value(); }
