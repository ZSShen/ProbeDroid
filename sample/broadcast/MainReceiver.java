package org.zsshen.broadcast;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class MainReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        Bundle bundle = intent.getExtras();
        String value = (String) bundle.get("Practice");
        Log.d("MainReceiver", value);
        Toast.makeText(context, value, Toast.LENGTH_SHORT).show();
    }
}
