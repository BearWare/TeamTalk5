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
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import androidx.appcompat.widget.Toolbar;

public class AccessibilityHelper {

    /**
     * Replaces the window decor action bar with a toolbar placed above the
     * activity's content.
     *
     * AppCompat lays the decor action bar and the window content on top of each
     * other, both starting at the top left corner of the screen. Accessibility
     * services order the two by size rather than by position, so the content,
     * being the taller of the two, always comes first. That leaves "Navigate
     * up", the screen title and the menu at the very end of the screen, after
     * every control the screen contains.
     *
     * A toolbar occupies a band of its own above the content, so the two no
     * longer overlap and are ordered by position instead.
     *
     * The activity must use a theme without a decor action bar, and must have
     * set its content view. The returned toolbar still has to be handed to
     * setSupportActionBar().
     */
    public static Toolbar installToolbar(Activity activity) {
        ViewGroup content = activity.findViewById(android.R.id.content);
        if (content == null || content.getChildCount() != 1)
            return null;

        View view = content.getChildAt(0);
        ViewGroup.LayoutParams viewparams = view.getLayoutParams();
        content.removeView(view);

        LinearLayout wrapper = new LinearLayout(activity);
        wrapper.setOrientation(LinearLayout.VERTICAL);

        Toolbar toolbar = (Toolbar) activity.getLayoutInflater().inflate(R.layout.toolbar, wrapper, false);
        wrapper.addView(toolbar);

        // Content filling the activity must not keep doing so now that the
        // toolbar has taken a part of it, or it pushes itself off the screen.
        boolean fills = viewparams == null ||
            viewparams.height == ViewGroup.LayoutParams.MATCH_PARENT;
        wrapper.addView(view, new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            fills ? 0 : viewparams.height,
                            fills ? 1 : 0));

        content.addView(wrapper, new ViewGroup.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.MATCH_PARENT));
        return toolbar;
    }
}
