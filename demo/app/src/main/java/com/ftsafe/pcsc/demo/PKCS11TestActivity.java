package com.ftsafe.pcsc.demo;

import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.ftsafe.ftreader.pcsc.FTReaderPCSC;
import com.ftsafe.pcsc.server.PCSCServerService;

/**
 * opensc-pkcs11 + libftreader-pcsc integration test.
 * 1. Start PCSCServer
 * 2. Call FTReader_InitSocket
 * 3. Load opensc-pkcs11 (dlopen)
 * 4. Call C_Initialize, C_GetSlotList
 */
public class PKCS11TestActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pkcs11_test);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.pkcs11_test_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        TextView result = findViewById(R.id.result);
        Button btnRun = findViewById(R.id.btn_run);

        btnRun.setOnClickListener(v -> {
            if (!PCSCServerService.isRunning()) {
                Toast.makeText(this, R.string.pkcs11_test_toast_start_first, Toast.LENGTH_SHORT).show();
                return;
            }
            result.setText("Running...");
            new Thread(() -> {
                try {
                    // 1. libftreader-pcsc loaded via FTReaderPCSC
                    // 2. Initialize socket
                    FTReaderPCSC.initSocket(SettingsPrefs.getHost(this), SettingsPrefs.getPort(this));
                    // 3. Run test (internally dlopen opensc-pkcs11)
                    String report = PKCS11Test.nativeRunTest();
                    runOnUiThread(() -> {
                        result.setText(report);
                        Toast.makeText(this, R.string.pkcs11_test_toast_done, Toast.LENGTH_SHORT).show();
                    });
                } catch (UnsatisfiedLinkError e) {
                    runOnUiThread(() -> {
                        result.setText("Error: opensc-pkcs11 not built\n" + e.getMessage());
                        Toast.makeText(this, "opensc-pkcs11 not built", Toast.LENGTH_LONG).show();
                    });
                } catch (Exception e) {
                    runOnUiThread(() -> {
                        result.setText("Error: " + e.getMessage());
                        Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
                    });
                }
            }).start();
        });
    }
}
