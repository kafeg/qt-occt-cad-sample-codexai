#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

class QComboBox;
class QDoubleSpinBox;

// Dialog to select a sketch (by name) and an extrusion distance
class CreateExtrudeDialog : public QDialog
{
  Q_OBJECT
public:
  CreateExtrudeDialog(QWidget* parent = nullptr);

  void setSketchNames(const QStringList& names); // populate selection list
  int  selectedSketchIndex() const;              // index into provided list
  double distance() const;                       // extrusion distance (+Z)

private:
  QComboBox*     m_sketches; // sketch selection
  QDoubleSpinBox* m_distance; // distance spin
};

