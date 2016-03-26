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
