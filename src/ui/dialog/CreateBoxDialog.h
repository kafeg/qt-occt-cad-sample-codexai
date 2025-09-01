#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class QDoubleSpinBox;

// Dialog to enter box dimensions (dx, dy, dz)
class CreateBoxDialog : public QDialog
{
  Q_OBJECT
public:
  CreateBoxDialog(QWidget* parent = nullptr);

  double dx() const; // X length
  double dy() const; // Y length
  double dz() const; // Z length

private:
  QDoubleSpinBox* m_dx; // spin for dx
  QDoubleSpinBox* m_dy; // spin for dy
  QDoubleSpinBox* m_dz; // spin for dz
};

