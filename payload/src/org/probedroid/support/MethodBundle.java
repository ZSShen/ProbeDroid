package org.probedroid.support;

public abstract class MethodBundle {
    private final boolean mInterceptBefore;
    private final boolean mInterceptAfter;

    public MethodBundle(boolean interceptBefore, boolean interceptAfter) {
        mInterceptBefore = interceptBefore;
        mInterceptAfter = interceptAfter;
    }

    public abstract void beforeMethodExecute(Object[] inputs);
    public abstract void afterMethodExecute(Object output);
}
