package org.zsshen.sms;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.widget.Toast;

public class SMSReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        Bundle bundle = intent.getExtras();
        StringBuffer sb = null;

        if (bundle != null) {
            Object[] pdus = (Object[]) bundle.get("pdus");
            SmsMessage[] msgs = new SmsMessage[pdus.length];
            sb = new StringBuffer();
            for (int i = 0 ; i < msgs.length ; ++i) {
                msgs[i] = SmsMessage.createFromPdu((byte[])pdus[i]);
                sb.append("SMS from ");
                sb.append(msgs[i].getOriginatingAddress());
                sb.append(" :");
                sb.append(msgs[i].getMessageBody().toString());
                sb.append("\n");
            }
            Toast.makeText(context, sb.toString(), Toast.LENGTH_SHORT).show();
        }
    }
}
