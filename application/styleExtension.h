#ifndef STYLEEXTENSION_H
#define STYLEEXTENSION_H

#include <QStyle>

// ugly way of 'extending' ControlElement enum
namespace customControlElements
{
  const QStyle::ControlElement CE_FloatingBorder = (QStyle::ControlElement)(QStyle::CE_CustomBase + 1);
}

namespace customPixelMetrics
{
    const QStyle::PixelMetric PM_FloatingBorderSize = (QStyle::PixelMetric)(QStyle::PM_CustomBase + 1);
    const QStyle::PixelMetric PM_ResizerActiveArea = (QStyle::PixelMetric)(QStyle::PM_CustomBase + 2);
}

#endif // STYLEEXTENSION_H
