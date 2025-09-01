#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class QDoubleSpinBox;

class CreateCylinderDialog : public QDialog
{
  Q_OBJECT
public:
  CreateCylinderDialog(QWidget* parent = nullptr);

  double radius() const;
  double height() const;

private:
  QDoubleSpinBox* myRadius;
  QDoubleSpinBox* myHeight;
};
