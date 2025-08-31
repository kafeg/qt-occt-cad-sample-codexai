#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class QDoubleSpinBox;

class CreateBoxDialog : public QDialog
{
  Q_OBJECT
public:
  CreateBoxDialog(QWidget* parent = nullptr);

  double dx() const;
  double dy() const;
  double dz() const;

private:
  QDoubleSpinBox* myDx;
  QDoubleSpinBox* myDy;
  QDoubleSpinBox* myDz;
};

