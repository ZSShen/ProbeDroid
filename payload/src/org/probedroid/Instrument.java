package org.probedroid;

import org.probedroid.support.MethodBundle;

public abstract class Instrument {
    public void prepareNativeBridge(String pathLib) {
        System.load(pathLib);
    }

    public static void hookMethod(String nameClass, String nameMethod,
            String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException {
    }

    public abstract void onApplicationStart();
    public abstract void onApplicationStop();

    private static native void hookMethodNative(String nameClass,
            String nameMethod, String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException;
}
