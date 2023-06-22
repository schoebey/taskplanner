#include "linkwidget.h"
#include "ui_linkwidget.h"

#include <QPainter>
#include <QContextMenuEvent>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QImage>

#include <cmath>

namespace
{
static const int c_iIconSize = 32;

QPixmap ToScaledPixmap(const QImage& image)
{
    if (image.width() < c_iIconSize || image.height() < c_iIconSize)
    {
        QImage target(c_iIconSize, c_iIconSize, QImage::Format_ARGB32);
        target.fill(0);
        QPainter p(&target);
        int iOffsetX = (c_iIconSize - image.width())/2;
        int iOffsetY = (c_iIconSize - image.height())/2;
        p.drawImage(iOffsetX, iOffsetY, image);
        return QPixmap::fromImage(target);
    }
    else
    {
        return QPixmap::fromImage(image).scaled(c_iIconSize, c_iIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

QString toString(const QUrl& url)
{
    return url.toString();
}

QString sanitize(const QString& s)
{
    QString sSanitized(s);
    sSanitized.replace("/", "_").replace(":","_").replace(";","_").replace("?","_").replace(".", "_");
    return sSanitized;
}

class ImgCache
{
public:
    ImgCache()
    {
        readFromTemp();
    }
    ~ImgCache()
    {
        writeToTemp();
    }

    template<typename T>
    QImage* get(const T& t)
    {
        return get(toString(t));
    }

    QImage* get(const QString& sKey)
    {
        QString sSanitized = sanitize(sKey);
        auto it = imageCache.find(sSanitized);
        if (it != imageCache.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    template<typename T>
    void add(const T& t, const QImage& img)
    {
        add(toString(t), img);
    }

    void add(const QString& sKey, const QImage& img)
    {
        QString sSanitized = sanitize(sKey);
        imageCache[sSanitized] = img;
        writeToTemp(sSanitized, img);
    }
private:
    bool writeToTemp(const QString& sKey, const QImage& img)
    {
        QString sLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(sLocation);

        if (!dir.cd("url_icons"))
        {
            return false;
        }

        return img.save(dir.filePath(sKey), "PNG");
    }
    void writeToTemp()
    {
        for (const auto &[sKey, img] : imageCache)
        {
            writeToTemp(sKey, img);
        }
    }

    void readFromTemp()
    {
        QString sLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);


        QDir dir(sLocation);
        if (!dir.exists())
        {
            dir.mkpath(sLocation);
            dir.setPath(sLocation);
        }
        dir.mkdir("url_icons");
        dir.cd("url_icons");

        auto fileInfos = dir.entryInfoList();
        for (const auto& fileInfo : fileInfos)
        {
            if (fileInfo.isFile())
            {
                QImage img;
                img.load(fileInfo.absoluteFilePath(), "PNG");
                if (!img.isNull())
                {
                    QString sSanitized = sanitize(fileInfo.fileName());
                    imageCache[sSanitized] = img;
                }
            }
        }
    }

    std::map<QString, QImage> imageCache;
};

ImgCache& Cache()
{
    static ImgCache cache;
    return cache;
}


QString extractHrefFromTag(const QString& sTag)
{
    QString sPattern = R"url(<link.*href="?([^ "]*)"?)url";
    QRegularExpression rx(sPattern);
    auto match = rx.match(sTag);
    if (match.hasMatch())
    {
        QString sCapture = match.captured(1);
        return sCapture;
    }

    return {};
}

QString extractTagFromDocument(const QByteArray& baDoc)
{
    QRegularExpression rxTag("<link[^>]*>", QRegularExpression::MultilineOption);


    // todo: parse header to find correct encoding
    QString sDoc = QString::fromUtf8(baDoc);

    QStringList links;
    int iOffset = 0;
    auto it = rxTag.globalMatch(sDoc);
    while (it.hasNext())
    {
        auto match = it.next();
        iOffset = match.capturedStart() + match.capturedLength();
        links.push_back(match.captured());
    }


    QStringList relationships = {"icon", "shortcut icon", "alternative icon"};

    QString sPattern = R"url(<link.*?(?=rel)rel\s*=\s*"%1"[^>]*>)url";

    for (const QString& sRel : relationships)
    {
        QRegularExpression rx((sPattern.arg(sRel)));
        for (const auto& sLink : links)
        {
            auto match = rx.match(sLink);
            if (match.hasMatch())
            {
                QString sCapture = match.captured(0);
                return sCapture;
            }
        }
    }

    return {};
}
}
//"<link href=/assets/img/favicons/favicon.ico rel="shortcut icon">"

QStyleOptionLinkWidget::QStyleOptionLinkWidget()
  : QStyleOption(1, eLinkWidget)
{
}



LinkWidget::LinkWidget(const QUrl& link)
  : ui(new Ui::LinkWidget),
    m_link(link)
{
  ui->setupUi(this);
  setAttribute(Qt::WA_StyledBackground, false);
  setFocusPolicy(Qt::ClickFocus);
  setConstrainLabelToSize(true);

  QString s(m_link.toLocalFile());
  QFileInfo info(s);
  if (info.exists())
  {
    setProperty("isFile", true);
    QFileIconProvider p;
    QIcon icon = p.icon(info);
    if (!icon.isNull())
    {
      ui->pIcon->setPixmap(icon.pixmap(QSize(32, 32)));
    }

    if (!info.fileName().isEmpty())
    {
      ui->pLabel->setText(info.fileName());
    }
    else
    {
      auto elements = info.filePath().split("/");
      if (2 <= elements.size())
      {
        ui->pLabel->setText(elements[elements.size() - 2]);
      }
    }
  }
  else
  {
    setProperty("isFile", false);
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    ui->pIcon->setPixmap(pm);
    ui->pLabel->setText(m_link.toString());


    // try to grab the favicon of the website
    m_spNetworkAccessManager = std::make_shared<QNetworkAccessManager>(this);

    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setProtocol(QSsl::AnyProtocol);


    // TODO: query local cache before issuing network request
    auto* pImg = Cache().get(m_link);
    if (nullptr == pImg)
    {
        QNetworkRequest request;
        request.setSslConfiguration(sslConfiguration);

        request.setUrl(m_link);
    
        QNetworkReply *reply = m_spNetworkAccessManager->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(fileDownloaded()));
    }
    else
    {
        ui->pIcon->setPixmap(ToScaledPixmap(*pImg));
    }
  }



  //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resize(32, 32);
  setMinimumSize(32, 32);


  m_pContextMenu = new QMenu();
  QAction* pOpenAction = new QAction(tr("Open link..."), this);
  m_pContextMenu->addAction(pOpenAction);
  connect(pOpenAction, SIGNAL(triggered()), this, SLOT(openLink()));
  QAction* pDeleteAction = new QAction(tr("Delete"), this);
  pDeleteAction->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete << Qt::Key_Backspace);
  pDeleteAction->setShortcutContext(Qt::WidgetShortcut);
  m_pContextMenu->addAction(pDeleteAction);
  connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(onDeleteTriggered()));


  setContextMenuPolicy(Qt::DefaultContextMenu);
}

LinkWidget::~LinkWidget()
{

}

void LinkWidget::setConstrainLabelToSize(bool bConstrain)
{
  m_bConstrainLabelToSize = bConstrain;
  if (bConstrain)
  {
//    ui->pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
    ui->pLabel->setVisible(false);
  }
  else
  {
    ui->pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    ui->pLabel->setVisible(true);
  }
}

bool LinkWidget::constrainLabelToSize() const
{
  return m_bConstrainLabelToSize;
}

void LinkWidget::setDrawFrame(bool bDraw)
{
  m_bDrawFrame = bDraw;
}

bool LinkWidget::drawFrame() const
{
  return m_bDrawFrame;
}

void LinkWidget::setBorderRadius(double dRadius)
{
  m_dBorderRadius = dRadius;
}

double LinkWidget::borderRadius() const
{
  return m_dBorderRadius;
}

void LinkWidget::showOverlay()
{
  if (nullptr == m_pToolTip)
  {
    m_pToolTip = new LinkWidget(m_link);
    m_pToolTip->setConstrainLabelToSize(false);
    m_pToolTip->setDrawFrame(true);
    m_pToolTip->setParent(window());
    m_pToolTip->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_pToolTip->setFocusPolicy(Qt::NoFocus);
    m_pToolTip->show();
    QSize s(m_pToolTip->sizeHint());
    m_pToolTip->resize(s);
    QPoint ptTl = ui->pIcon->mapTo(m_pToolTip->parentWidget(), ui->pIcon->rect().topLeft());
    QPointF ptCenter = ptTl + QPointF(ui->pIcon->rect().width() / 2.,
                                      ui->pIcon->rect().height() / 2.);

    QPointF ptTooltipTl = m_pToolTip->ui->pIcon->mapTo(m_pToolTip, m_pToolTip->ui->pIcon->rect().topLeft());
    QPointF ptTooltipCenter = ptTooltipTl + QPointF(m_pToolTip->ui->pIcon->rect().width() / 2.,
                                                    m_pToolTip->ui->pIcon->rect().height() / 2.);
    QPointF offset = ptTooltipCenter;
    m_pToolTip->move(ptCenter.x() - floor(offset.x()), ptCenter.y() - floor(offset.y()));
  }
}

void LinkWidget::hideOverlay()
{
  delete m_pToolTip;
  m_pToolTip = nullptr;
}

void LinkWidget::onDeleteTriggered()
{
  hideOverlay();
  emit deleteTriggered(m_link);
}

void LinkWidget::enterEvent(QEvent* /*pEvent*/)
{
  showOverlay();
}

void LinkWidget::leaveEvent(QEvent* /*pEvent*/)
{
  hideOverlay();
}

void LinkWidget::contextMenuEvent(QContextMenuEvent* pEvent)
{
  if (nullptr != m_pContextMenu)
  {
    hideOverlay();
    pEvent->accept();
    m_pContextMenu->exec(pEvent->globalPos());
  }
  else
  {
    pEvent->ignore();
  }
}

void LinkWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QStyleOptionLinkWidget opt;
  opt.initFrom(this);

  QRect r(rect());
  QRect cr(childrenRect());
  opt.dPaddingX = (r.width() - cr.width()) / 2.;
  opt.dPaddingY = (r.height() - cr.height()) / 2.;
  opt.dBorderRadius = m_dBorderRadius;

  QRect iconLabelRect = ui->pIcon->rect();
  opt.iconRect = ui->pIcon->pixmap()->rect();
  int iOffsetX = (iconLabelRect.width() - opt.iconRect.width()) / 2;
  int iOffsetY = (iconLabelRect.height() - opt.iconRect.height()) / 2;
  opt.iconRect.moveTo(ui->pIcon->mapTo(this, QPoint(iOffsetX, iOffsetY)));
  opt.labelRect = ui->pLabel->rect();
  opt.labelRect.moveTo(ui->pLabel->mapTo(this, QPoint(0,0)));
  opt.bDrawFrame = m_bDrawFrame;

  QPainter painter(this);
  style()->drawControl(static_cast<QStyle::ControlElement>(CE_LinkWidget), &opt, &painter, this);
}

void LinkWidget::mousePressEvent(QMouseEvent*)
{

}

void LinkWidget::mouseReleaseEvent(QMouseEvent*)
{

}

void LinkWidget::mouseDoubleClickEvent(QMouseEvent*)
{
  openLink();
}

void LinkWidget::openLink()
{
  QDesktopServices::openUrl(m_link);
  hideOverlay();
}

void LinkWidget::fileDownloaded()
{
  QNetworkReply* pReply = dynamic_cast<QNetworkReply*>(sender());
  if (nullptr != pReply)
  {
    QByteArray ba = pReply->readAll();

    QImage img;
    if (img.loadFromData(ba) && !img.isNull())
    {
        Cache().add(m_link, img);
        ui->pIcon->setPixmap(ToScaledPixmap(img));
    }
    else
    {
        // ba contains the site source.
        // scan it for favicon links like "shortcut icon"
      QString sTag = extractTagFromDocument(ba);
      QString sIconUrl = extractHrefFromTag(sTag);
      if (!sIconUrl.isEmpty())
      {
        QUrl url(sIconUrl);
        if (url.isRelative())
        {
            QString sHost = m_link.scheme() + "://" + m_link.host();
            QString sUrl = sHost + sIconUrl;
            url = sUrl;
        }
        // TODO: query local cache before issuing network request
        QNetworkRequest request;
        request.setUrl(url);
        QNetworkReply *reply = m_spNetworkAccessManager->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(fileDownloaded()));
      }
    }
  }
}


