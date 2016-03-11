package org.probedroid;

import org.probedroid.support.MethodBundle;

public abstract class Instrument {
    public void prepareNativeBridge(String pathLib) {
        System.load(pathLib);
    }

    public void instrumentMethod(boolean isStatic, String nameClass,
            String nameMethod, String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException,
            IllegalArgumentException {
        instrumentMethodNative(isStatic, nameClass, nameMethod,
                signatureMethod, bundle);
    }

    public abstract void onApplicationStart();

    public abstract void onApplicationStop();

    private native void instrumentMethodNative(boolean isStatic,
            String nameClass, String nameMethod, String signatureMethod,
            MethodBundle bundle) throws ClassNotFoundException,
            NoSuchMethodException, IllegalArgumentException;
}
