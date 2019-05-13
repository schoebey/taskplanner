#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QDesktopServices>
#include <QDateTime>
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent) :
  OverlayWidget(parent),
  ui(new Ui::AboutDialog),
  m_pWidget(new QWidget)
{
  m_pWidget->setObjectName("contents");
  ui->setupUi(m_pWidget);
  addWidget(m_pWidget);

  setTitle(tr("About %1").arg(qApp->applicationName()));
  ui->pAppName->setText(qApp->applicationName());
  ui->pAppVersion->setText(QString("Version %1").arg(QString(GIT_VERSION)));

  bool bOk(false);
  uint timestamp = static_cast<uint>(QString(GIT_TIMESTAMP).toInt(&bOk));
  if (bOk)
  {
    QDateTime dt = QDateTime::fromTime_t(timestamp);
    ui->pCopyright->setText(QString("Copyright 2018-%1 Roman Schaub").arg(dt.toString("yyyy")));
  }

  connect(ui->pIconsBy, SIGNAL(linkActivated(QString)), this, SLOT(onLinkClicked(QString)));
}

AboutDialog::~AboutDialog()
{
  delete ui;
}

void AboutDialog::onLinkClicked(const QString& sLink)
{
  QDesktopServices::openUrl(sLink);
}
