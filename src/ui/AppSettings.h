#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <QSettings>
#include <Standard_WarningsRestore.hxx>

// Simple application-wide settings holder with persistence via QSettings
class AppSettings : public QObject
{
  Q_OBJECT
public:
  static AppSettings& instance()
  {
    static AppSettings s;
    return s;
  }

  bool showDatumRelatedItems() const { return m_showDatumRelatedItems; }

public slots:
  void setShowDatumRelatedItems(bool on)
  {
    if (m_showDatumRelatedItems == on) return;
    m_showDatumRelatedItems = on;
    m_qsettings.setValue(QStringLiteral("ui/showDatumRelatedItems"), on);
    emit showDatumRelatedItemsChanged(on);
  }

signals:
  void showDatumRelatedItemsChanged(bool);

private:
  AppSettings()
    : QObject(nullptr)
    , m_qsettings(QStringLiteral("OpenAI"), QStringLiteral("cad-app"))
  {
    // Default: hide Datum-related items in history/bodies
    m_showDatumRelatedItems = m_qsettings.value(QStringLiteral("ui/showDatumRelatedItems"), false).toBool();
  }

private:
  QSettings m_qsettings;
  bool      m_showDatumRelatedItems = false;
};

