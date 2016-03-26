package org.zsshen.componentinspector;

import java.util.ArrayList;

import org.probedroid.support.MethodBundle;

import android.content.Intent;
import android.os.Bundle;

public class StartedActivity extends MethodBundle {
    public StartedActivity(boolean interceptBefore, boolean interceptAfter) {
        super(interceptBefore, interceptAfter);
    }

    @Override
    public void beforeMethodExecute(Object[] objects) {
        Intent intent = (Intent) objects[0];
        Bundle option = (Bundle) objects[1];

        // Log the Intent summary content.
        StringBuffer sb = new StringBuffer();
        sb.append(intent.toString());
        // Also log the embedded bundle if necessary.
        Bundle extras = intent.getExtras();
        if (extras != null)
            sb.append(' ').append(intent.getExtras().toString());

        if (!ComponentInspector.gLogMap
                .containsKey(ComponentInspector.KEY_ACTIVITY))
            ComponentInspector.gLogMap.put(ComponentInspector.KEY_ACTIVITY,
                    new ArrayList<String>());
        ArrayList<String> list = ComponentInspector.gLogMap
                .get(ComponentInspector.KEY_ACTIVITY);
        list.add(sb.toString());
    }

    @Override
    public void afterMethodExecute(Object o) {

    }
}
