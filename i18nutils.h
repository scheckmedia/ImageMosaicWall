/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-07-30
 * Description : i18n helper functions.
 *
 * Copyright (C) 2020 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_I18N_UTILS_H
#define DIGIKAM_I18N_UTILS_H

// Qt includes

#include <QApplication>
#include <QString>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

// NOTE: do not use namespace here.

void s_initI18nResource()
{
    Q_INIT_RESOURCE(i18n);
}

void s_cleanupI18nResource()
{
    Q_CLEANUP_RESOURCE(i18n);
}

/**
 * Load translators system based i18n. Internal method.
 */
bool s_loadTranslation(const QString& lang, const QString& name)
{
    qDebug() << "Loading i18n" << lang << "for" << name;

    QTranslator* const i18n = new QTranslator(qApp);

    if (!i18n->load(QString::fromLatin1(":/i18n/%1.qm").arg(lang)))
    {
        delete i18n;
        return false;
    }

    qApp->installTranslator(i18n);

    return true;
}

/**
 * Load one translation file from normal i18n packaging installation.
 * 'name' is the module name to print on the console to debug.
 */
void s_loadI18n(const QString& name)
{
    // Quote from ecm_create_qm_loader created code:
    // The way Qt translation system handles plural forms makes it necessary to
    // have a translation file which contains only plural forms for `en`.
    // That's why we load the `en` translation unconditionally, then load the
    // translation for the current locale to overload it.

    const QString en(QStringLiteral("en"));

    s_loadTranslation(en, name);

    QLocale locale = QLocale::system();

    if (locale.name() != en)
    {
        if (!s_loadTranslation(locale.name(), name))
        {
            s_loadTranslation(locale.bcp47Name(), name);
        }
    }
}

#endif // DIGIKAM_I18N_UTILS_H
