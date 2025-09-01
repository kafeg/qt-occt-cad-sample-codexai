#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class QDoubleSpinBox;

// Dialog to enter cylinder parameters (radius, height)
class CreateCylinderDialog : public QDialog
{
  Q_OBJECT
public:
  CreateCylinderDialog(QWidget* parent = nullptr);

  double radius() const; // Cylinder radius
  double height() const; // Cylinder height (Z)

private:
  QDoubleSpinBox* m_radius; // spin for radius
  QDoubleSpinBox* m_height; // spin for height
};

