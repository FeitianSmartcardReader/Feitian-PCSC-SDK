package com.ftsafe.pcsc.demo;

import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;
import com.ftsafe.ftreader.pcsc.FTReaderPCSC;
import com.ftsafe.pcsc.server.PCSCServerService;

public class PrivateApiTestActivity extends AppCompatActivity {

    private static final String TAG = "PrivateApiTest";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_private_api_test);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.private_api_test_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        Button btn = findViewById(R.id.btn_device_info);
        TextView result = findViewById(R.id.result_device_info);
        btn.setOnClickListener(v -> {
            result.setVisibility(TextView.VISIBLE);
            result.setText(R.string.api_running);
            btn.setEnabled(false);
            new Thread(() -> {
                try {
                    apiDeviceInfo();
                } catch (Exception e) {
                    runOnUiThread(() -> {
                        result.setText("Error: " + e.getMessage());
                        Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
                    });
                } finally {
                    runOnUiThread(() -> btn.setEnabled(true));
                }
            }).start();
        });
    }

    private void apiDeviceInfo() {
        if (!PCSCServerService.isRunning()) {
            runOnUiThread(() -> {
                findViewById(R.id.result_device_info).setVisibility(TextView.VISIBLE);
                ((TextView) findViewById(R.id.result_device_info)).setText(getString(R.string.api_error_start_first));
                Snackbar.make(findViewById(android.R.id.content), R.string.api_error_start_first, Snackbar.LENGTH_LONG)
                        .setAction(R.string.api_action_start_server, x -> {
                            PCSCServerService.start(this, PCSCServerService.getDefaultPort());
                            Toast.makeText(this, R.string.main_toast_server_started, Toast.LENGTH_SHORT).show();
                        })
                        .show();
            });
            return;
        }
        ensureInit();
        Log.d(TAG, "apiDeviceInfo");
        String json = FTReaderPCSC.nativeGetDeviceInfo();
        runOnUiThread(() -> {
            TextView tv = findViewById(R.id.result_device_info);
            tv.setVisibility(TextView.VISIBLE);
            tv.setText(json != null ? json : "(null)");
        });
    }

    private void ensureInit() {
        String host = SettingsPrefs.getHost(this);
        int port = SettingsPrefs.getPort(this);
        Log.d(TAG, "ensureInit host=" + host + " port=" + port);
        FTReaderPCSC.initSocket(host, port);
    }
}
