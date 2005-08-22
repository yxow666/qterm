/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef OSDMESSAGE_H
#define OSDMESSAGE_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>

/**
 * A widget that displays messages in the top-left corner.
 */
class PageViewMessage : public QWidget
{
    public:
        PageViewMessage( QWidget * parent );

        enum Icon { None, Info, Warning, Error, Find };
        void display( const QString & message, Icon icon = Info, int durationMs = 4000 );

    protected:
        void paintEvent( QPaintEvent * e );
        void mousePressEvent( QMouseEvent * e );

    private:
        QPixmap m_pixmap;
        QTimer * m_timer;
};

#endif