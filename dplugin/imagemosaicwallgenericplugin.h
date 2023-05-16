/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-05-26
 * Description : ImageMosaicWall generic plugin.
 *
 * Copyright (C) 2020-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_MOSAIC_WALL_PLUGIN_H
#define DIGIKAM_IMAGE_MOSAIC_WALL_PLUGIN_H

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.ImageMosaicWall"

using namespace Digikam;

namespace DigikamGenericImageMosaicWallPlugin
{

class ImageMosaicWallPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit ImageMosaicWallPlugin(QObject* const parent = nullptr);
    ~ImageMosaicWallPlugin();

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;

private Q_SLOTS:

    void slotImageMosaicWall();
};

} // namespace DigikamGenericImageMosaicWallPlugin

#endif // DIGIKAM_IMAGE_MOSAIC_WALL_PLUGIN_H
