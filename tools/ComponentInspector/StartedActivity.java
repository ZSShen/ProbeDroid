/**
 *   The MIT License (MIT)
 *   Copyright (c) <2016> <ZongXian Shen, andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

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
