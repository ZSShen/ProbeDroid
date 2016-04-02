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

package org.zsshen.stringinspector;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import org.probedroid.Instrument;

import android.util.Log;

public class StringInspector extends Instrument {

    public static final String NAME_MODULE = "StringInspector";
    public static ArrayList<String> gLogList;

    @Override
    public void onApplicationStart() {
        gLogList = new ArrayList<String>();

        // Monitor strings converted from StringBuilder.
        String nameClass = "java.lang.StringBuilder";
        String nameMethod = "toString";
        String signatureMethod = "()Ljava/lang/String;";
        ConvertedStringBuilder bundleBuilder = new ConvertedStringBuilder(false,
                true);
        try {
            instrumentMethod(false, nameClass, nameMethod, signatureMethod,
                    bundleBuilder);
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
        for (String str : gLogList)
            sb.append(str).append('\n');
        File file = new File(
                "/data/data/com.google.android.apps.maps/InstrumentResult.txt");
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
