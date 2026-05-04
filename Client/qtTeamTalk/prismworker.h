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

#ifndef PRISMWORKER_H
#define PRISMWORKER_H

#if defined(ENABLE_PRISM)

#include <QObject>
#include <QString>
#include <prism.h>

// Runs all blocking Prism calls (initialize, speak, braille, output) in a
// dedicated thread so they cannot freeze the main/UI thread.
class PrismWorker : public QObject
{
    Q_OBJECT

public:
    explicit PrismWorker(QObject* parent = nullptr);
    ~PrismWorker();

public slots:
    void initialize(quint64 backendId);
    void speak(const QString& text, bool interrupt);
    void braille(const QString& text);
    void output(const QString& text, bool interrupt);
    void shutdown();

private:
    PrismContext* m_context = nullptr;
    PrismBackend* m_backend = nullptr;
};

#endif // ENABLE_PRISM
#endif // PRISMWORKER_H
