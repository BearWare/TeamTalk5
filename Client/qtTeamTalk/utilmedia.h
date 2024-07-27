/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(UTILMEDIA_H)
#define UTILMEDIA_H

#include "utiltt.h"

#include <QString>

#define MEDIAFILE_SLIDER_MAXIMUM 10000

QString durationToString(quint32 duration_msec, bool include_msec = true);
QString getMediaAudioDescription(const AudioFormat& audioFmt);
QString getMediaVideoDescription(const VideoFormat& videoFmt);
#endif // UTILMEDIA_H
