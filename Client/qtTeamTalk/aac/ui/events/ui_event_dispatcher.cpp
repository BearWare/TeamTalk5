#include "ui_event_dispatcher.h"

#include <algorithm>

namespace AAC {
namespace UI {
namespace Events {

UiEventDispatcher::UiEventDispatcher()
    : nextId_(1)
{
}

UiEventDispatcher::~UiEventDispatcher() = default;

int UiEventDispatcher::subscribe(UiEventType type, UiEventHandler handler)
{
    const int id = nextId_++;
    handlers_[type].push_back({ id, std::move(handler) });
    return id;
}

void UiEventDispatcher::unsubscribe(UiEventType type, int handlerId)
{
    auto it = handlers_.find(type);
    if (it == handlers_.end())
        return;

    auto &list = it->second;
    list.erase(
        std::remove_if(list.begin(), list.end(),
                       [handlerId](const HandlerEntry &entry) {
                           return entry.id == handlerId;
                       }),
        list.end()
    );

    if (list.empty())
        handlers_.erase(it);
}

void UiEventDispatcher::emit(const UiEvent &event)
{
    auto it = handlers_.find(event.type);
    if (it == handlers_.end())
        return;

    const auto handlersCopy = it->second;
    for (const auto &entry : handlersCopy) {
        if (entry.handler)
            entry.handler(event);
    }
}

} // namespace Events
} // namespace UI
} // namespace AAC
