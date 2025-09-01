#include "CreateExtrudeDialog.h"

#include <Standard_WarningsDisable.hxx>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

CreateExtrudeDialog::CreateExtrudeDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Create Extrude");

  m_sketches = new QComboBox(this);
  m_distance = new QDoubleSpinBox(this);
  m_distance->setRange(0.001, 1e6);
  m_distance->setDecimals(3);
  m_distance->setSingleStep(1.0);
  m_distance->setValue(10.0);

  QFormLayout* form = new QFormLayout();
  form->addRow("Sketch", m_sketches);
  form->addRow("Distance", m_distance);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

void CreateExtrudeDialog::setSketchNames(const QStringList& names)
{
  m_sketches->clear();
  m_sketches->addItems(names);
}

int CreateExtrudeDialog::selectedSketchIndex() const
{
  return m_sketches->currentIndex();
}

double CreateExtrudeDialog::distance() const
{
  return m_distance->value();
}

