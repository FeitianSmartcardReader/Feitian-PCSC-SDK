package com.ftsafe.pcsc.demo;

import android.Manifest;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.ftsafe.ftreader.pcsc.FTReaderPCSC;
import com.ftsafe.pcsc.server.PCSCServerService;
import com.google.android.material.snackbar.Snackbar;

import java.util.List;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private static final int PERM_REQ = 100;
    private static final String ACTION_USB_PERMISSION = "com.ftsafe.pcsc.demo.USB_PERMISSION";

    private TextView tvServerStatus;
    private TextView tvReaderStatus;
    private TextView tvBleCount;
    private Spinner spinnerBleDevices;

    private final BroadcastReceiver usbPermissionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ACTION_USB_PERMISSION.equals(intent.getAction())) {
                UsbDevice device = Build.VERSION.SDK_INT >= 33
                        ? intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice.class)
                        : intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) && device != null) {
                    Log.d(TAG, "USB permission granted: " + device.getDeviceName());
                    Toast.makeText(MainActivity.this, R.string.main_usb_permission_granted, Toast.LENGTH_SHORT).show();
                } else {
                    Log.w(TAG, "USB permission denied: " + (device != null ? device.getDeviceName() : ""));
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SettingsPrefs.applyBleCcidDebug(this);
        setContentView(R.layout.activity_main);

        tvServerStatus = findViewById(R.id.tv_server_status);
        tvReaderStatus = findViewById(R.id.tv_reader_status);
        tvBleCount = findViewById(R.id.tv_ble_count);
        spinnerBleDevices = findViewById(R.id.spinner_ble_devices);

        findViewById(R.id.btn_start).setOnClickListener(v -> startServer());
        findViewById(R.id.btn_stop).setOnClickListener(v -> stopServer());
        findViewById(R.id.btn_scan_ble).setOnClickListener(v -> scanBleDevices());
        findViewById(R.id.btn_connect).setOnClickListener(v -> connect());
        findViewById(R.id.btn_pcsc_test).setOnClickListener(v -> startActivity(new Intent(this, PcscTestActivity.class)));
        findViewById(R.id.btn_private_api_test).setOnClickListener(v -> startActivity(new Intent(this, PrivateApiTestActivity.class)));
        findViewById(R.id.btn_pkcs11_test).setOnClickListener(v -> startActivity(new Intent(this, Pkcs11HubActivity.class)));
        findViewById(R.id.btn_settings).setOnClickListener(v -> startActivity(new Intent(this, SettingsActivity.class)));

        tvBleCount.setText(R.string.main_ble_tap_scan);
        ArrayAdapter<String> emptyAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item,
                new String[]{getString(R.string.main_ble_no_devices)});
        emptyAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerBleDevices.setAdapter(emptyAdapter);

        requestBlePermissions();
        requestUsbPermissionsForAttachedDevices();
        handleUsbDeviceAttachedIntent(getIntent());

        updateStatus();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        handleUsbDeviceAttachedIntent(intent);
    }

    private void handleUsbDeviceAttachedIntent(Intent intent) {
        if (intent == null || !UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(intent.getAction())) return;
        UsbDevice device = Build.VERSION.SDK_INT >= 33
                ? intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice.class)
                : intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
        if (device != null) {
            requestUsbPermission(device);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (Build.VERSION.SDK_INT >= 33) {
            registerReceiver(usbPermissionReceiver, new IntentFilter(ACTION_USB_PERMISSION), Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(usbPermissionReceiver, new IntentFilter(ACTION_USB_PERMISSION));
        }
        updateStatus();
    }

    @Override
    protected void onPause() {
        super.onPause();
        try {
            unregisterReceiver(usbPermissionReceiver);
        } catch (Exception ignored) {}
    }

    private void requestUsbPermission(UsbDevice device) {
        UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        if (usbManager == null || device == null) return;
        if (usbManager.hasPermission(device)) {
            Log.d(TAG, "USB already has permission: " + device.getDeviceName());
            return;
        }
        int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S ? PendingIntent.FLAG_MUTABLE : 0;
        PendingIntent pi = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), flags);
        usbManager.requestPermission(device, pi);
        Log.d(TAG, "Requesting USB permission for: " + device.getDeviceName());
    }

    private void requestUsbPermissionsForAttachedDevices() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) return;
        UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        if (usbManager == null) return;
        var deviceList = usbManager.getDeviceList();
        if (deviceList == null) return;
        int[] feitianVendors = {0x096E, 0x04E6, 0x060A, 0x170E};
        for (UsbDevice device : deviceList.values()) {
            int vid = device.getVendorId();
            for (int v : feitianVendors) {
                if (vid == v && !usbManager.hasPermission(device)) {
                    requestUsbPermission(device);
                    break;
                }
            }
        }
    }

    private void requestBlePermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED
                    || ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN}, PERM_REQ);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, PERM_REQ);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERM_REQ) {
            boolean allGranted = grantResults.length > 0;
            for (int r : grantResults) {
                if (r != PackageManager.PERMISSION_GRANTED) allGranted = false;
            }
            if (!allGranted) {
                Snackbar.make(findViewById(android.R.id.content), R.string.main_permission_denied_hint, Snackbar.LENGTH_LONG)
                        .setAction(R.string.main_retry, v -> requestBlePermissions())
                        .show();
            }
        }
    }

    private void updateStatus() {
        boolean running = PCSCServerService.isRunning();
        int port = PCSCServerService.getDefaultPort();
        tvServerStatus.setText(running ? getString(R.string.main_server_running, port) : getString(R.string.main_server_stopped));
        tvReaderStatus.setText(running ? R.string.main_reader_unknown : R.string.main_reader_na);
    }

    private void updateReaderStatus() {
        if (!PCSCServerService.isRunning()) {
            tvReaderStatus.setText(R.string.main_reader_na);
            return;
        }
        new Thread(() -> {
            try {
                String json = FTReaderPCSC.nativeListReaders();
                runOnUiThread(() -> {
                    int count = parseReaderCount(json);
                    if (count < 0) {
                        tvReaderStatus.setText(json != null && !json.isEmpty() ? json : getString(R.string.main_reader_unknown));
                    } else if (count == 0) {
                        tvReaderStatus.setText(R.string.main_reader_none);
                    } else {
                        tvReaderStatus.setText(getString(R.string.main_reader_count, count));
                    }
                });
            } catch (Exception e) {
                runOnUiThread(() -> tvReaderStatus.setText(R.string.main_reader_unknown));
            }
        }).start();
    }

    private int parseReaderCount(String json) {
        if (json == null || json.isEmpty()) return -1;
        if (json.startsWith("[")) {
            int count = 0;
            for (int i = 0; i < json.length(); i++) {
                if (json.charAt(i) == '"') count++;
            }
            return count / 2;
        }
        return -1;
    }

    private void scanBleDevices() {
        tvBleCount.setText(R.string.main_ble_scanning);
        ArrayAdapter<String> scanningAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item,
                new String[]{getString(R.string.main_ble_no_devices)});
        scanningAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerBleDevices.setAdapter(scanningAdapter);
        PCSCServerService.scanBleDevices(this, "FT_", names -> runOnUiThread(() -> {
            if (names == null || names.isEmpty()) {
                tvBleCount.setText(R.string.main_ble_none);
                ArrayAdapter<String> a = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item,
                        new String[]{getString(R.string.main_ble_no_devices)});
                a.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                spinnerBleDevices.setAdapter(a);
            } else {
                tvBleCount.setText(getString(R.string.main_ble_found_count, names.size()));
                ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, names);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                spinnerBleDevices.setAdapter(adapter);
            }
        }));
    }

    private void connect() {
        Object selected = spinnerBleDevices.getSelectedItem();
        String deviceName = (selected != null && !getString(R.string.main_ble_no_devices).equals(selected.toString()))
                ? selected.toString() : null;
        if (deviceName != null) {
            PCSCServerService.setPreferredReader(deviceName);
        }
        if (!PCSCServerService.isRunning()) {
            Snackbar.make(findViewById(android.R.id.content), R.string.main_toast_start_server_first, Snackbar.LENGTH_LONG)
                    .setAction(R.string.main_action_start_server, v -> {
                        startServer();
                        connect();
                    })
                    .show();
            return;
        }
        String host = SettingsPrefs.getHost(this);
        int port = SettingsPrefs.getPort(this);
        try {
            FTReaderPCSC.initSocket(host, port);
            Toast.makeText(this, R.string.main_toast_socket_initialized, Toast.LENGTH_SHORT).show();
            updateReaderStatus();
        } catch (UnsatisfiedLinkError e) {
            Toast.makeText(this, R.string.main_toast_native_failed, Toast.LENGTH_SHORT).show();
        }
    }

    private void startServer() {
        int port = SettingsPrefs.getPort(this);
        Log.d(TAG, "startServer port=" + port);
        PCSCServerService.start(this, port);
        updateStatus();
        Toast.makeText(this, R.string.main_toast_server_started, Toast.LENGTH_SHORT).show();
    }

    private void stopServer() {
        PCSCServerService.stop();
        FTReaderPCSC.uninit();
        updateStatus();
        Toast.makeText(this, R.string.main_toast_server_stopped, Toast.LENGTH_SHORT).show();
    }
}
