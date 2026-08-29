#pragma once

#include "ui_events.h"

#include <functional>
#include <map>
#include <vector>

namespace AAC {
namespace UI {
namespace Events {

using UiEventHandler = std::function<void(const UiEvent &)>;

class UiEventDispatcherInterface {
public:
    virtual ~UiEventDispatcherInterface() = default;

    virtual int subscribe(UiEventType type, UiEventHandler handler) = 0;
    virtual void unsubscribe(UiEventType type, int handlerId) = 0;
    virtual void emit(const UiEvent &event) = 0;
};

class UiEventDispatcher : public UiEventDispatcherInterface {
public:
    UiEventDispatcher();
    ~UiEventDispatcher() override;

    int subscribe(UiEventType type, UiEventHandler handler) override;
    void unsubscribe(UiEventType type, int handlerId) override;
    void emit(const UiEvent &event) override;

private:
    struct HandlerEntry {
        int id;
        UiEventHandler handler;
    };

    int nextId_;
    std::map<UiEventType, std::vector<HandlerEntry>> handlers_;
};

} // namespace Events
} // namespace UI
} // namespace AAC
