#include "CreateSketchDialog.h"

#include <Standard_WarningsDisable.hxx>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

CreateSketchDialog::CreateSketchDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("New Sketch");

  m_planes = new QComboBox(this);

  QFormLayout* form = new QFormLayout();
  form->addRow("Plane", m_planes);

  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addLayout(form);
  lay->addWidget(btns);
  setLayout(lay);
}

void CreateSketchDialog::setPlaneNames(const QStringList& names)
{
  m_planes->clear();
  m_planes->addItems(names);
}

int CreateSketchDialog::selectedPlaneIndex() const
{
  return m_planes->currentIndex();
}

