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

  m_dx = new QDoubleSpinBox(this);
  m_dy = new QDoubleSpinBox(this);
  m_dz = new QDoubleSpinBox(this);
  for (auto sb : {m_dx, m_dy, m_dz}) {
    sb->setRange(0.001, 1e6);
    sb->setDecimals(3);
    sb->setSingleStep(1.0);
    sb->setValue(10.0);
  }

  QFormLayout* form = new QFormLayout();
  form->addRow("X (dx)", m_dx);
  form->addRow("Y (dy)", m_dy);
  form->addRow("Z (dz)", m_dz);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

double CreateBoxDialog::dx() const { return m_dx->value(); }
double CreateBoxDialog::dy() const { return m_dy->value(); }
double CreateBoxDialog::dz() const { return m_dz->value(); }

