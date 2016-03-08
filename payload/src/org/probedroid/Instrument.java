package org.probedroid;

import org.probedroid.support.MethodBundle;

public abstract class Instrument {
    public void prepareNativeBridge(String pathLib) {
        System.load(pathLib);
    }

    public void instrumentMethod(String nameClass, String nameMethod,
            String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException {
        instrumentMethodNative(nameClass, nameMethod, signatureMethod, bundle);
    }

    public abstract void onApplicationStart();

    public abstract void onApplicationStop();

    private native void instrumentMethodNative(String nameClass,
            String nameMethod, String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException;
}
