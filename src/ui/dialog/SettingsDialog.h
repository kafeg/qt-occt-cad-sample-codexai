#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class QCheckBox;

class SettingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);

private:
  QCheckBox* m_chkShowDatumRelated = nullptr;
};

