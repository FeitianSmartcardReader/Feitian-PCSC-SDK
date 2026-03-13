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

public class PcscTestActivity extends AppCompatActivity {

    private static final String TAG = "PcscTest";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pcsc_test);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.pcsc_test_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        bindApi(R.id.btn_list_readers, R.id.result_list_readers, this::apiListReaders);
        bindApi(R.id.btn_run_tests, R.id.result_run_tests, this::apiRunTests);
    }

    private void bindApi(int btnId, int resultId, Runnable action) {
        Button btn = findViewById(btnId);
        TextView result = findViewById(resultId);
        btn.setOnClickListener(v -> {
            result.setVisibility(TextView.VISIBLE);
            result.setText(R.string.api_running);
            btn.setEnabled(false);
            new Thread(() -> {
                try {
                    action.run();
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

    private void setResult(int resultId, String text) {
        runOnUiThread(() -> {
            TextView tv = findViewById(resultId);
            tv.setVisibility(TextView.VISIBLE);
            tv.setText(text != null ? text : "(null)");
        });
    }

    private void apiListReaders() {
        if (!PCSCServerService.isRunning()) {
            setResult(R.id.result_list_readers, getString(R.string.api_error_start_first));
            runOnUiThread(() -> Snackbar.make(findViewById(android.R.id.content), R.string.api_error_start_first, Snackbar.LENGTH_LONG)
                    .setAction(R.string.api_action_start_server, x -> {
                        PCSCServerService.start(this, PCSCServerService.getDefaultPort());
                        Toast.makeText(this, R.string.main_toast_server_started, Toast.LENGTH_SHORT).show();
                    })
                    .show());
            return;
        }
        ensureInit();
        Log.d(TAG, "apiListReaders");
        String json = FTReaderPCSC.nativeListReaders();
        setResult(R.id.result_list_readers, json);
    }

    private void apiRunTests() {
        if (!PCSCServerService.isRunning()) {
            setResult(R.id.result_run_tests, getString(R.string.api_error_start_first));
            runOnUiThread(() -> Snackbar.make(findViewById(android.R.id.content), R.string.api_error_start_first, Snackbar.LENGTH_LONG)
                    .setAction(R.string.api_action_start_server, x -> {
                        PCSCServerService.start(this, PCSCServerService.getDefaultPort());
                        Toast.makeText(this, R.string.main_toast_server_started, Toast.LENGTH_SHORT).show();
                    })
                    .show());
            return;
        }
        ensureInit();
        String report = FTReaderPCSC.nativeRunAllTests();
        setResult(R.id.result_run_tests, report);
    }

    private void ensureInit() {
        String host = SettingsPrefs.getHost(this);
        int port = SettingsPrefs.getPort(this);
        Log.d(TAG, "ensureInit host=" + host + " port=" + port);
        FTReaderPCSC.initSocket(host, port);
    }
}
