package dk.bearware.gui;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.View.AccessibilityDelegate;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.widget.Button;

public class AccessibilityAssistant extends AccessibilityDelegate {

    private final Activity hostActivity;
    private final AccessibilityManager accessibilityService;

    private volatile boolean discourageUiUpdates;
    private volatile boolean eventsLocked;

    public AccessibilityAssistant(Activity activity) {
        hostActivity = activity;
        accessibilityService = (AccessibilityManager) activity.getSystemService(Context.ACCESSIBILITY_SERVICE);
        discourageUiUpdates = false;
        eventsLocked = false;
    }

    public boolean isUiUpdateDiscouraged() {
        return discourageUiUpdates && accessibilityService.isEnabled();
    }

    public void lockEvents() {
        eventsLocked = true;
    }

    public void unlockEvents() {
        if (!hostActivity.getWindow().getDecorView().post(new Runnable() {
                @Override
                public void run() {
                    eventsLocked = false;
                }
            }))
            eventsLocked = false;
    }

    @Override
    public void sendAccessibilityEvent(View host, int eventType) {
        if (host instanceof Button)
            checkEvent(eventType);
        if (!eventsLocked)
            super.sendAccessibilityEvent(host, eventType);
    }

    @Override
    public void sendAccessibilityEventUnchecked(View host, AccessibilityEvent event) {
        if (host instanceof Button)
            checkEvent(event.getEventType());
        if (!eventsLocked)
            super.sendAccessibilityEventUnchecked(host, event);
    }

    private void checkEvent(int eventType) {
        switch (eventType) {
        case AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED:
            discourageUiUpdates = true;
            break;
        case AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED:
            discourageUiUpdates = false;
            break;
        default:
            break;
        }
    }

}
