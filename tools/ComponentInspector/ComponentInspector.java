/**
 *   The MIT License (MIT)
 *   Copyright (c) <2016> <ZongXian Shen, andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

package org.zsshen.componentinspector;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import org.probedroid.Instrument;

import android.util.Log;

public class ComponentInspector extends Instrument {

    public static final String NAME_MODULE = "ComponentInspector";
    public static final String KEY_ACTIVITY = "Activity";
    public static final String KEY_SERVICE = "Service";
    public static HashMap<String, ArrayList<String>> gLogMap;

    @Override
    public void onApplicationStart() {
        gLogMap = new HashMap<String, ArrayList<String>>();

        // Monitor started activities.
        String nameClass = "android.app.Activity";
        String nameMethod = "startActivity";
        String signatureMethod = "(Landroid/content/Intent;Landroid/os/Bundle;)V";
        StartedActivity bundleActivity = new StartedActivity(true, false);
        try {
            instrumentMethod(false, nameClass, nameMethod, signatureMethod,
                    bundleActivity);
        } catch (ClassNotFoundException e) {
            Log.d(NAME_MODULE, e.toString());
        } catch (NoSuchMethodException e) {
            Log.d(NAME_MODULE, e.toString());
        }

        // Monitor started services.
        nameClass = "android.app.Activity";
        nameMethod = "startService";
        signatureMethod = "(Landroid/content/Intent;)Landroid/content/ComponentName;";
        StartedService bundleService = new StartedService(true, false);
        try {
            instrumentMethod(false, nameClass, nameMethod, signatureMethod,
                    bundleService);
        } catch (ClassNotFoundException e) {
            Log.d(NAME_MODULE, e.toString());
        } catch (NoSuchMethodException e) {
            Log.d(NAME_MODULE, e.toString());
        }
    }

    @Override
    public void onApplicationStop() {
        Log.d(NAME_MODULE, "OK Terminate now");

        StringBuffer sb = new StringBuffer();
        ArrayList<String> list = gLogMap.get(KEY_ACTIVITY);
        sb.append(KEY_ACTIVITY).append('\n');
        for (String msg : list)
            sb.append(msg).append('\n');

        sb.append('\n');
        list = gLogMap.get(KEY_SERVICE);
        sb.append(KEY_SERVICE).append('\n');
        for (String msg : list)
            sb.append(msg).append('\n');

        File file = new File("/data/data/com.kkbox.kktix/InstrumentResult.txt");
        BufferedWriter output = null;
        try {
            output = new BufferedWriter(new FileWriter(file));
            output.write(sb.toString());
            output.close();
        } catch (IOException e) {
            Log.d(NAME_MODULE, e.toString());
        }
    }
}
