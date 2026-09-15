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

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;

public class DialogHelper {
    public static void showWithImeSubmit(AlertDialog.Builder alert, EditText input) {
        // The keyboard reads the IME options when it opens, and the listener
        // needs a created dialog to reach its buttons, so the dialog is shown
        // here rather than by the caller
        input.setImeOptions(EditorInfo.IME_ACTION_DONE);
        AlertDialog dialog = alert.create();
        dialog.show();
        input.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_NULL) {
                dialog.getButton(DialogInterface.BUTTON_POSITIVE).performClick();
                return true;
            }
            return false;
        });
    }
}
