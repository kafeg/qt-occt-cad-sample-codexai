#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

class QComboBox;

// Dialog to select a reference plane for a new sketch
class CreateSketchDialog : public QDialog
{
  Q_OBJECT
public:
  explicit CreateSketchDialog(QWidget* parent = nullptr);

  void setPlaneNames(const QStringList& names); // populate selection list
  int  selectedPlaneIndex() const;              // index into provided list

private:
  QComboBox* m_planes{nullptr};
};
