#include "navigationcontroller.h"

NavigationController::NavigationController(QObject *parent)
    : QObject(parent)
{
}

void NavigationController::showServerList() {}
void NavigationController::showPublicServers() {}
void NavigationController::showConnectionView() {}
void NavigationController::showChannelView() {}
void NavigationController::showInChannelView() {}
