package org.zsshen.stringinspector;

import org.probedroid.support.MethodBundle;

public class ConvertedStringBuilder extends MethodBundle {
    public ConvertedStringBuilder(boolean interceptBefore,
            boolean interceptAfter) {
        super(interceptBefore, interceptAfter);
    }

    @Override
    public void beforeMethodExecute(Object[] objects) {

    }

    @Override
    public void afterMethodExecute(Object o) {
        String nameThread = Thread.currentThread().getName();
        StringBuffer sb = new StringBuffer();
        sb.append(nameThread).append(" -> ").append(o.toString());
        StringInspector.gLogList.add(sb.toString());
    }
}
