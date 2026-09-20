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

#include "prismworker.h"

#if defined(ENABLE_PRISM)

#include <QDebug>

PrismWorker::PrismWorker(QObject* parent)
    : QObject(parent)
{
}

PrismWorker::~PrismWorker()
{
    shutdown();
}

void PrismWorker::initialize(quint64 backendId)
{
    if (m_backend)
    {
        prism_backend_free(m_backend);
        m_backend = nullptr;
    }
    if (m_context)
    {
        prism_shutdown(m_context);
        m_context = nullptr;
    }

    PrismConfig cfg = prism_config_init();
    m_context = prism_init(&cfg);
    if (!m_context)
        return;

    PrismBackendId id = static_cast<PrismBackendId>(backendId);
    if (id != PRISM_BACKEND_INVALID)
        m_backend = prism_registry_create(m_context, id);
    else
        m_backend = prism_registry_create_best(m_context);

    if (!m_backend)
    {
        qWarning() << "Prism has no usable backend";
        return;
    }

    // prism_registry_create_best() initializes the backend it picks, so an
    // already initialized backend is a success here, not a failure
    PrismError err = prism_backend_initialize(m_backend);
    if (err != PRISM_OK && err != PRISM_ERROR_ALREADY_INITIALIZED)
    {
        qWarning() << "Prism backend" << prism_backend_name(m_backend)
                   << "failed to initialize:" << prism_error_string(err);
        prism_backend_free(m_backend);
        m_backend = nullptr;
    }
}

void PrismWorker::speak(const QString& text, bool interrupt)
{
    if (m_backend)
        warnOnError("speak", prism_backend_speak(m_backend, text.toUtf8().constData(), interrupt));
}

void PrismWorker::braille(const QString& text)
{
    if (m_backend)
        warnOnError("braille", prism_backend_braille(m_backend, text.toUtf8().constData()));
}

void PrismWorker::output(const QString& text, bool interrupt)
{
    if (m_backend)
        warnOnError("output", prism_backend_output(m_backend, text.toUtf8().constData(), interrupt));
}

void PrismWorker::warnOnError(const char* call, PrismError err)
{
    if (err != PRISM_OK)
        qWarning() << "Prism" << call << "failed:" << prism_error_string(err);
}

void PrismWorker::shutdown()
{
    if (m_backend)
    {
        prism_backend_free(m_backend);
        m_backend = nullptr;
    }
    if (m_context)
    {
        prism_shutdown(m_context);
        m_context = nullptr;
    }
}

#endif // ENABLE_PRISM
