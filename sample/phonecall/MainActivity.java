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

package org.zsshen.phonecall;

import android.net.Uri;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.view.View.OnClickListener;

public class MainActivity extends Activity
{
    private Button callBtn;
    private Button dialBtn;
    private EditText number;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        number = (EditText) findViewById(R.id.phoneNumber);
        callBtn = (Button) findViewById(R.id.call);
        dialBtn = (Button) findViewById(R.id.dial);

        // Add PhoneStateListener for monitoring.
        MyPhoneListener phoneListener = new MyPhoneListener();
        TelephonyManager telephonyManager =
                (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        // Receive notifications of telephony state changes.
        telephonyManager.listen(phoneListener, PhoneStateListener.LISTEN_CALL_STATE);

        callBtn.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                try {
                    // Set the data.
                    String uri = "tel:" + number.getText().toString();
                    Intent callIntent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));

                    startActivity(callIntent);
                }catch(Exception e) {
                    Toast.makeText(getApplicationContext(), "Your call has failed...",
                            Toast.LENGTH_LONG).show();
                    e.printStackTrace();
                }
            }
        });

        dialBtn.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(View v) {
                try {
                    String uri = "tel:" + number.getText().toString();
                    Intent dialIntent = new Intent(Intent.ACTION_DIAL, Uri.parse(uri));

                    startActivity(dialIntent);
                }catch(Exception e) {
                    Toast.makeText(getApplicationContext(), "Your call has failed...",
                            Toast.LENGTH_LONG).show();
                    e.printStackTrace();
                }
            }
        });
    }

    private class MyPhoneListener extends PhoneStateListener
    {
        private boolean onCall = false;

        @Override
        public void onCallStateChanged(int state, String incomingNumber)
        {
            switch (state) {
                case TelephonyManager.CALL_STATE_RINGING:
                    // Phone ringing...
                    Toast.makeText(MainActivity.this, incomingNumber + " calls you",
                            Toast.LENGTH_LONG).show();
                    break;

                case TelephonyManager.CALL_STATE_OFFHOOK:
                    // One call exists that is dialing, active, or on hold
                    Toast.makeText(MainActivity.this, "on call...",
                            Toast.LENGTH_LONG).show();
                    // Because user answers the incoming call
                    onCall = true;
                    break;

                case TelephonyManager.CALL_STATE_IDLE:
                    // In initialization of the class and at the end of phone call.

                    // Detect flag from CALL_STATE_OFFHOOK.
                    if (onCall == true) {
                        Toast.makeText(MainActivity.this, "restart app after call",
                                Toast.LENGTH_LONG).show();

                        // Restart our application.
                        Intent restart = getBaseContext().getPackageManager().
                                getLaunchIntentForPackage(getBaseContext().getPackageName());
                        restart.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                        startActivity(restart);

                        onCall = false;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
