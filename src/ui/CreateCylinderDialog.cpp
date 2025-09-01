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

  m_radius = new QDoubleSpinBox(this);
  m_height = new QDoubleSpinBox(this);
  for (auto sb : {m_radius, m_height}) {
    sb->setRange(0.001, 1e6);
    sb->setDecimals(3);
    sb->setSingleStep(1.0);
  }
  m_radius->setValue(10.0);
  m_height->setValue(20.0);

  QFormLayout* form = new QFormLayout();
  form->addRow("Radius", m_radius);
  form->addRow("Height", m_height);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

double CreateCylinderDialog::radius() const { return m_radius->value(); }
double CreateCylinderDialog::height() const { return m_height->value(); }
