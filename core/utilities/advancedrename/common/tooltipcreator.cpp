/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tooltipcreator.h"

// Qt includes

#include <QRegExp>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>

// Local includes

#include "modifier.h"
#include "option.h"
#include "parser.h"
#include "themeengine.h"

using namespace Digikam;

namespace Digikam
{

TooltipCreator& TooltipCreator::getInstance()
{
    static TooltipCreator m_instance;
    return m_instance;
}

QString TooltipCreator::additionalInformation()
{
    QStringList infoItems;
    infoItems << i18n("Modifiers can be applied to every renaming option.");
    infoItems << i18n("It is possible to assign multiple modifiers to an option, "
                      "they are applied in the order you assign them.");
    infoItems << i18n("Be sure to use the quick access buttons: They might provide "
                      "additional information about renaming and modifier options.");

    QString information;

    information += "<div style='margin-top:20px;'";

    information += tableStart(90);
    information += "<tr><td style='vertical-align:top;'><img src='" + getInfoIconResourceName() + "' /></td>";
    information += "<td><ol>";
    foreach (const QString& infoItem, infoItems)
    {
        information += "<li>" + infoItem + "</li>";

    }
    information += "</ol></td></tr>";
    information += tableEnd();

    information += "</div>";

    return information;
}

QString TooltipCreator::getInfoIconResourceName()
{
    return QString("mydata://info.png");
}

QPixmap TooltipCreator::getInfoIcon()
{
    return SmallIcon("lighttable", KIconLoader::SizeMedium);
}

QString TooltipCreator::tooltip(Parser* parser)
{
    if (!parser)
    {
        return QString();
    }

    QString tooltip;
    tooltip += "<html><head><title></title></head>";
    tooltip += "<body>";

    tooltip += tableStart();
    tooltip += createSection(i18n("Options"),   parser->options());
    tooltip += createSection(i18n("Modifiers"), parser->modifiers(), true);
    tooltip += tableEnd();

    if (!parser->modifiers().isEmpty())
    {
        tooltip += additionalInformation();
    }

    tooltip += "</body>";
    tooltip += "</html>";

    return tooltip;
}

QString TooltipCreator::tableStart(int width)
{
    QString w = QString::number(width) + "%";
    return QString("<table width=\"%1\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\">").arg(w);
}

QString TooltipCreator::tableStart()
{
    return tableStart(100);
}

QString TooltipCreator::tableEnd()
{
    return QString("</table>");
}

QString TooltipCreator::markOption(const QString& str)
{
    QString result = str;

    QRegExp optionsRegExp("\\|\\|(.*)\\|\\|");
    optionsRegExp.setMinimal(true);

    result.replace(optionsRegExp, QString("<i><font color=\"%1\">\\1</font></i>")
                   .arg(ThemeEngine::instance()->textSpecialRegColor().name()));
    return result;
}

QString TooltipCreator::createHeader(const QString& str)
{
    QString result;
    QString templateStr = QString("<tr><td style=\"background-color: %1; padding:0.25em;\" colspan=\"2\">"
                                  "<nobr><font color=\"%2\"><center><b>%3"
                                  "</b></center></font></nobr></td></tr>")
                          .arg(ThemeEngine::instance()->thumbSelColor().name())
                          .arg(ThemeEngine::instance()->textSelColor().name());

    result += templateStr.arg(str);
    return result;
}

template <class T>
QString TooltipCreator::createEntries(const QList<T*> &data)
{
    QString result;

    foreach (T* t, data)
    {
        foreach (Token* token, t->tokens())
        {
            result += QString("<tr>"
                              "<td style=\"background-color: %1;\">"
                              "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"
                              "<td>&nbsp;%4&nbsp;</td></tr>")
                      .arg(ThemeEngine::instance()->baseColor().name())
                      .arg(ThemeEngine::instance()->textRegColor().name())
                      .arg(markOption(token->id()))
                      .arg(markOption(token->description()));
        }
    }

    return result;
}

template <class T>
QString TooltipCreator::createSection(const QString& sectionName, const QList<T*> &data, bool lastSection)
{
    if (data.isEmpty())
    {
        return QString();
    }

    QString result;

    result += createHeader(sectionName);
    result += createEntries(data);

    if (!lastSection)
    {
        result += QString("<tr></tr>");
    }

    return result;
}

} // namespace Digikam
