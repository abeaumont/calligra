/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ANIMATION_PLAYER_H
#define KIS_ANIMATION_PLAYER_H

#include <QScopedPointer>
#include <QObject>

#include "kritaui_export.h"


class KisCanvas2;

class KRITAUI_EXPORT KisAnimationPlayer : public QObject
{
    Q_OBJECT

public:
    KisAnimationPlayer(KisCanvas2 *canvas);
    ~KisAnimationPlayer();

    void play();
    void stop();
    void displayFrame(int time);

    bool isPlaying();
    int currentTime();

    qreal playbackSpeed();

public slots:
    void slotUpdate();
    void slotCancelPlayback();
    void slotUpdatePlaybackSpeed(double value);
    void slotUpdatePlaybackTimer();

signals:
    void sigFrameChanged();
    void sigPlaybackStopped();

private:
    void connectCancelSignals();
    void disconnectCancelSignals();
    void uploadFrame(int frame);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};


#endif
