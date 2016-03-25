package org.zsshen.componentinspector;

import android.content.Intent;
import android.os.Bundle;

import org.probedroid.support.MethodBundle;

import java.util.ArrayList;

public class StartedService extends MethodBundle {
    public StartedService(boolean interceptBefore, boolean interceptAfter) {
        super(interceptBefore, interceptAfter);
    }

    @Override
    public void beforeMethodExecute(Object[] objects) {
        Intent intent = (Intent) objects[0];

        // Log the Intent summary content.
        StringBuffer sb = new StringBuffer();
        sb.append(intent.toString());

        // Also log the embedded bundle if necessary.
        Bundle extras = intent.getExtras();
        if (extras != null)
            sb.append(" ").append(intent.getExtras().toString());

        if (!ComponentInspector.gLogMap
                .containsKey(ComponentInspector.KEY_SERVICE))
            ComponentInspector.gLogMap.put(ComponentInspector.KEY_SERVICE,
                    new ArrayList<String>());
        ArrayList<String> list = ComponentInspector.gLogMap
                .get(ComponentInspector.KEY_SERVICE);
        list.add(sb.toString());
    }

    @Override
    public void afterMethodExecute(Object o) {

    }
}
