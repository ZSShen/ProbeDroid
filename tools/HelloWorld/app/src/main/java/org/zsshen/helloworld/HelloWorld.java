package org.zsshen.helloworld;

import org.probedroid.Instrument;
import org.zsshen.helloworld.bundle.StringBuilderToString;

public class HelloWorld extends Instrument {

    @Override
    public void onApplicationStart() {
        // Instrument "String StringBuilder.toString()".
        String nameClass = "java.lang.StringBuilder";
        String nameMethod = "toString";
        String signatureMethod = "()Ljava/lang/String;";

        StringBuilderToString bundleSBToString = new StringBuilderToString(true,
                true);
        try {
            instrumentMethod(nameClass, nameMethod, signatureMethod,
                    bundleSBToString);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onApplicationStop() {

    }
}
