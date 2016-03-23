package org.zsshen.helloworld;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import org.probedroid.Instrument;
import org.zsshen.helloworld.bundle.StringBuilderToString;

import android.util.Log;

public class HelloWorld extends Instrument {

    public static ArrayList<String> gListString;

    @Override
    public void onApplicationStart() {
        // Instrument "String StringBuilder.toString()".
        // String nameClass = "java.lang.StringBuffer";
        // String nameMethod = "append";
        // String signatureMethod =
        // "(Ljava/lang/String;)Ljava/lang/StringBuffer;";
        String nameClass = "java.lang.StringBuilder";
        String nameMethod = "toString";
        String signatureMethod = "()Ljava/lang/String;";

        StringBuilderToString bundleSBToString = new StringBuilderToString(true,
                true);
        try {
            instrumentMethod(false, nameClass, nameMethod, signatureMethod,
                    bundleSBToString);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

        gListString = new ArrayList<String>();
    }

    @Override
    public void onApplicationStop() {
        Log.d("HelloWorld", "OK Terminate now");

        StringBuffer sb = new StringBuffer();
        for (String str : gListString) {
            sb.append(str);
            sb.append("\n");
        }

        //Log.d("HelloWorld", sb.toString());
        File file = new File("./InstrumentResult.txt");
        String path = file.getAbsolutePath();
        Log.d("HelloWorld", path);
        BufferedWriter output = null;
        try {
            output = new BufferedWriter(new FileWriter(file));
            output.write(sb.toString());
            output.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
