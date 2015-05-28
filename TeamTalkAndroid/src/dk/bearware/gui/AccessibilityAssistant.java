/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.gui;

import android.app.Activity;
import android.content.Context;
import android.util.SparseArray;
import android.view.View;
import android.view.View.AccessibilityDelegate;
import android.view.ViewGroup;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.widget.Button;

public class AccessibilityAssistant extends AccessibilityDelegate {

    private final Activity hostActivity;
    private final AccessibilityManager accessibilityService;

    private SparseArray<View> monitoredPages;
    private View visiblePage;
    private int visiblePageId;

    private volatile boolean discourageUiUpdates;
    private volatile boolean eventsLocked;

    public AccessibilityAssistant(Activity activity) {
        hostActivity = activity;
        accessibilityService = (AccessibilityManager) activity.getSystemService(Context.ACCESSIBILITY_SERVICE);
        monitoredPages = new SparseArray<View>();
        visiblePage = null;
        visiblePageId = 0;
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

    public void registerPage(View page, int id) {
        monitoredPages.put(id, page);
        if (id == visiblePageId)
            visiblePage = page;
        page.setAccessibilityDelegate(this);
    }

    public void setVisiblePage(int id) {
        visiblePageId = id;
        visiblePage = monitoredPages.get(id);
    }

    @Override
    public boolean onRequestSendAccessibilityEvent(ViewGroup host, View child, AccessibilityEvent event) {
        return ((monitoredPages.indexOfValue(host) < 0) || (host == visiblePage)) && super.onRequestSendAccessibilityEvent(host, child, event);
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
