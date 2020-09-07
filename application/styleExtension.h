#ifndef STYLEEXTENSION_H
#define STYLEEXTENSION_H

#include <QStyle>
#include <QStyleOption>

namespace customStyleOptions
{
  const QStyleOption::OptionType SO_LinkWidget = static_cast<QStyleOption::OptionType>(QStyleOption::SO_CustomBase + 1);
}

namespace customControlElements
{
  const QStyle::ControlElement CE_LinkWidget     = static_cast<QStyle::ControlElement>(QStyle::CE_CustomBase + 1);
  const QStyle::ControlElement CE_FloatingBorder = static_cast<QStyle::ControlElement>(QStyle::CE_CustomBase + 2);
}

namespace customPixelMetrics
{
    const QStyle::PixelMetric PM_FloatingBorderSize = static_cast<QStyle::PixelMetric>(QStyle::PM_CustomBase + 1);
    const QStyle::PixelMetric PM_ResizerActiveArea = static_cast<QStyle::PixelMetric>(QStyle::PM_CustomBase + 2);
}

#endif // STYLEEXTENSION_H
