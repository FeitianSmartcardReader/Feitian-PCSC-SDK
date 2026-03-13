package com.ftsafe.pcsc.demo;

import android.os.Bundle;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import androidx.appcompat.widget.Toolbar;

import com.ftsafe.pcsc.server.PCSCServerService;

public class SettingsActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.settings_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        EditText etHost = findViewById(R.id.et_host);
        EditText etPort = findViewById(R.id.et_port);

        etHost.setText(SettingsPrefs.getHost(this));
        etPort.setText(String.valueOf(SettingsPrefs.getPort(this)));

        SwitchCompat switchBleCcidDebug = findViewById(R.id.switch_ble_ccid_debug);
        switchBleCcidDebug.setChecked(SettingsPrefs.getBleCcidDebug(this));
        switchBleCcidDebug.setOnCheckedChangeListener((buttonView, isChecked) -> {
            SettingsPrefs.setBleCcidDebug(SettingsActivity.this, isChecked);
            Toast.makeText(this, R.string.settings_toast_saved, Toast.LENGTH_SHORT).show();
        });

        findViewById(R.id.btn_save).setOnClickListener(v -> {
            String host = etHost.getText() != null ? etHost.getText().toString().trim() : "";
            if (host.isEmpty()) host = "127.0.0.1";
            int port;
            try {
                port = Integer.parseInt(etPort.getText() != null ? etPort.getText().toString() : "0");
            } catch (NumberFormatException e) {
                port = PCSCServerService.getDefaultPort();
            }
            if (port <= 0 || port > 65535) port = PCSCServerService.getDefaultPort();

            SettingsPrefs.setHost(this, host);
            SettingsPrefs.setPort(this, port);
            Toast.makeText(this, R.string.settings_toast_saved, Toast.LENGTH_SHORT).show();
        });
    }
}
