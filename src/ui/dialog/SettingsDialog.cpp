#include "SettingsDialog.h"

#include <AppSettings.h>

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <Standard_WarningsRestore.hxx>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(QStringLiteral("Settings"));
  auto* lay = new QVBoxLayout(this);
  m_chkShowDatumRelated = new QCheckBox(QStringLiteral("Show datum-related items in History and Bodies"), this);
  m_chkShowDatumRelated->setChecked(AppSettings::instance().showDatumRelatedItems());
  lay->addWidget(m_chkShowDatumRelated);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  lay->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
    AppSettings::instance().setShowDatumRelatedItems(m_chkShowDatumRelated->isChecked());
    accept();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

