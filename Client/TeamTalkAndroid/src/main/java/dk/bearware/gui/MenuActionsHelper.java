/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

package dk.bearware.gui;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.text.TextUtils;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.HashSet;
import java.util.Set;

import androidx.appcompat.widget.ActionMenuView;
import androidx.core.view.AccessibilityDelegateCompat;
import androidx.core.view.ViewCompat;
import androidx.core.view.accessibility.AccessibilityNodeInfoCompat;
import androidx.core.view.accessibility.AccessibilityNodeInfoCompat.AccessibilityActionCompat;

public class MenuActionsHelper {

    /**
     * Offers the items hidden behind the overflow button as accessibility
     * actions on it.
     *
     * Reaching one of them with a screen reader otherwise means opening the
     * menu and swiping through it. Publishing them as actions on the button
     * puts them one gesture away, without changing what the menu itself looks
     * like or does.
     *
     * Items the menu shows as buttons of their own are left out, being a swipe
     * away already. Which items those are depends on how much room the menu has
     * and so changes with the screen, which is why they are looked up when
     * asked for rather than when the menu is built.
     *
     * Call from onCreateOptionsMenu() once the menu has been inflated. The menu
     * is read back each time an accessibility service asks, so items that come
     * and go are reflected without any further work.
     */
    public static void addMenuActions(Activity activity, Menu menu) {
        View decor = activity.getWindow().getDecorView();

        // The overflow button is created along with the rest of the menu, so it
        // is not in the view hierarchy yet when the menu is inflated.
        decor.post(() -> {
            ActionMenuView menuview = findMenuView(decor);
            if (menuview == null)
                return;

            View overflow = findOverflowButton(menuview);
            if (overflow == null)
                return;

            ViewCompat.setAccessibilityDelegate(overflow, new AccessibilityDelegateCompat() {

                @Override
                public void onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfoCompat info) {
                    super.onInitializeAccessibilityNodeInfo(host, info);

                    Set<String> buttons = titlesShownAsButtons(menuview);

                    for (int i = 0; i < menu.size(); i++) {
                        MenuItem item = menu.getItem(i);
                        if (item.isVisible() && item.isEnabled() &&
                            !buttons.contains(title(item.getTitle())))
                            info.addAction(new AccessibilityActionCompat(item.getItemId(), item.getTitle()));
                    }
                }

                @Override
                public boolean performAccessibilityAction(View host, int action, Bundle arguments) {
                    MenuItem item = menu.findItem(action);
                    if ((item != null) && item.isVisible() && item.isEnabled())
                        return activity.onOptionsItemSelected(item);

                    return super.performAccessibilityAction(host, action, arguments);
                }
            });
        });
    }

    /**
     * The overflow button is the only child of the menu view which shows an
     * icon rather than a title, the items themselves being text views.
     */
    private static View findOverflowButton(ActionMenuView menuview) {
        for (int i = 0; i < menuview.getChildCount(); i++) {
            if (menuview.getChildAt(i) instanceof ImageView button)
                return button;
        }
        return null;
    }

    /**
     * Titles of the items the menu is showing as buttons of their own. A menu
     * item can be shown with its title, or as an icon carrying the title as its
     * description, so both are collected.
     */
    private static Set<String> titlesShownAsButtons(ActionMenuView menuview) {
        Set<String> titles = new HashSet<>();

        for (int i = 0; i < menuview.getChildCount(); i++) {
            View child = menuview.getChildAt(i);
            if (child instanceof ImageView)
                continue; // the overflow button itself

            if ((child instanceof TextView text) && !TextUtils.isEmpty(text.getText()))
                titles.add(title(text.getText()));
            if (!TextUtils.isEmpty(child.getContentDescription()))
                titles.add(title(child.getContentDescription()));
        }
        return titles;
    }

    private static String title(CharSequence title) {
        return (title != null) ? title.toString().toLowerCase() : "";
    }

    private static ActionMenuView findMenuView(View view) {
        if (view instanceof ActionMenuView menuview)
            return menuview;

        if (view instanceof ViewGroup group) {
            for (int i = 0; i < group.getChildCount(); i++) {
                ActionMenuView menuview = findMenuView(group.getChildAt(i));
                if (menuview != null)
                    return menuview;
            }
        }
        return null;
    }
}
