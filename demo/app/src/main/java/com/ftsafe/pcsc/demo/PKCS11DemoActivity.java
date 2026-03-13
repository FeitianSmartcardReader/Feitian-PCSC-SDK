package com.ftsafe.pcsc.demo;

import android.os.Bundle;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.ftsafe.ftreader.pcsc.FTReaderPCSC;
import com.ftsafe.pcsc.server.PCSCServerService;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * PKCS#11 API Demo - step-by-step test for OpenSC PIV cards.
 * Prerequisites: Start PCSCServer from main screen, connect reader and insert card.
 * PKCS#11 native calls run on a single thread to avoid concurrent socket/OpenSC access.
 */
public class PKCS11DemoActivity extends AppCompatActivity {

    private EditText paramHost, paramPort, paramSlotId, paramPin;
    private long currentSession = 0;
    private static final ExecutorService pkcs11Executor = Executors.newSingleThreadExecutor();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PKCS11Demo.nativeSetOpenSCDebug(9);  /* Enable OpenSC sc_log for debugging; must run before C_Initialize */
        setContentView(R.layout.activity_pkcs11_demo);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.pkcs11_demo_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        paramHost = findViewById(R.id.param_host);
        paramPort = findViewById(R.id.param_port);
        paramSlotId = findViewById(R.id.param_slot_id);
        paramPin = findViewById(R.id.param_pin);

        bindApi(R.id.btn_init, R.id.result_init, this::apiInitialize);
        bindApi(R.id.btn_finalize, R.id.result_finalize, this::apiFinalize);
        bindApi(R.id.btn_get_slot_list, R.id.result_slot_list, this::apiGetSlotList);
        bindApi(R.id.btn_get_slot_info, R.id.result_slot_info, this::apiGetSlotInfo);
        bindApi(R.id.btn_get_token_info, R.id.result_token_info, this::apiGetTokenInfo);
        bindApi(R.id.btn_open_session, R.id.result_open_session, this::apiOpenSession);
        bindApi(R.id.btn_login, R.id.result_login, this::apiLogin);
        bindApi(R.id.btn_find_objects, R.id.result_find_objects, this::apiFindObjects);
        bindApi(R.id.btn_logout, R.id.result_logout, this::apiLogout);
        bindApi(R.id.btn_close_session, R.id.result_close_session, this::apiCloseSession);
        bindApi(R.id.btn_get_mechanism_list, R.id.result_get_mechanism_list, this::apiGetMechanismList);
        bindApi(R.id.btn_get_mechanism_info, R.id.result_get_mechanism_info, this::apiGetMechanismInfo);
        bindApi(R.id.btn_get_session_info, R.id.result_get_session_info, this::apiGetSessionInfo);
        bindApi(R.id.btn_get_attribute_value, R.id.result_get_attribute_value, this::apiGetAttributeValue);
        bindApi(R.id.btn_sign, R.id.result_sign, this::apiSign);
        bindApi(R.id.btn_digest, R.id.result_digest, this::apiDigest);
        bindApi(R.id.btn_verify, R.id.result_verify, this::apiVerify);
        bindApi(R.id.btn_decrypt, R.id.result_decrypt, this::apiDecrypt);
        bindApi(R.id.btn_encrypt, R.id.result_encrypt, this::apiEncrypt);
        bindApi(R.id.btn_generate_random, R.id.result_generate_random, this::apiGenerateRandom);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String host = SettingsPrefs.getHost(this);
        int port = SettingsPrefs.getPort(this);
        paramHost.setText(host);
        paramPort.setText(String.valueOf(port));
        if (paramSlotId.getText().toString().isEmpty()) paramSlotId.setText("0");
        if (paramPin.getText().toString().isEmpty()) paramPin.setText("123456");
    }

    private void bindApi(int btnId, int resultId, Runnable action) {
        TextView result = findViewById(resultId);
        findViewById(btnId).setOnClickListener(v -> {
            if (!PCSCServerService.isRunning()) {
                Toast.makeText(this, R.string.pkcs11_demo_toast_start_first, Toast.LENGTH_SHORT).show();
                return;
            }
            ensureInit();
            result.setVisibility(TextView.VISIBLE);
            result.setText("Running...");
            pkcs11Executor.execute(() -> {
                try {
                    action.run();
                } catch (Throwable e) {
                    runOnUiThread(() -> {
                        result.setText("Error: " + e.getMessage());
                        Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
                    });
                }
            });
        });
    }

    private void ensureInit() {
        try {
            String host = paramHost.getText() != null ? paramHost.getText().toString().trim() : "127.0.0.1";
            if (host.isEmpty()) host = "127.0.0.1";
            int port = parsePort(paramPort, 35963);
            FTReaderPCSC.initSocket(host, port);
        } catch (UnsatisfiedLinkError ignored) {}
    }

    private int parsePort(EditText et, int fallback) {
        try {
            String s = et.getText() != null ? et.getText().toString().trim() : "";
            if (s.isEmpty()) return fallback;
            int p = Integer.parseInt(s);
            return (p > 0 && p <= 65535) ? p : fallback;
        } catch (NumberFormatException e) { return fallback; }
    }

    private long parseSlotId() {
        try {
            String s = paramSlotId.getText() != null ? paramSlotId.getText().toString().trim() : "0";
            if (s.isEmpty()) return 0;
            return Long.parseLong(s.startsWith("0x") ? s.substring(2) : s, s.startsWith("0x") ? 16 : 10);
        } catch (NumberFormatException e) { return 0; }
    }

    private void setResult(int resultId, String text) {
        runOnUiThread(() -> {
            TextView tv = findViewById(resultId);
            tv.setVisibility(TextView.VISIBLE);
            tv.setText(text != null ? text : "(null)");
        });
    }

    private void apiInitialize() { setResult(R.id.result_init, PKCS11Demo.nativeInitialize()); }
    private void apiFinalize() { setResult(R.id.result_finalize, PKCS11Demo.nativeFinalize()); }
    private void apiGetSlotList() { setResult(R.id.result_slot_list, PKCS11Demo.nativeGetSlotList()); }
    private void apiGetSlotInfo() { setResult(R.id.result_slot_info, PKCS11Demo.nativeGetSlotInfo(parseSlotId())); }
    private void apiGetTokenInfo() { setResult(R.id.result_token_info, PKCS11Demo.nativeGetTokenInfo(parseSlotId())); }

    private void apiOpenSession() {
        long slotId = parseSlotId();
        String result = PKCS11Demo.nativeOpenSession(slotId);
        if (result != null && result.startsWith("OK:")) {
            try {
                long h = Long.parseLong(result.substring(3));
                currentSession = h;
                setResult(R.id.result_open_session, "OK, session=" + h);
            } catch (NumberFormatException e) {
                setResult(R.id.result_open_session, result);
            }
        } else {
            String msg = (result != null && result.startsWith("FAIL:")) ? result.substring(5) : result;
            setResult(R.id.result_open_session, msg != null && !msg.isEmpty() ? msg : getString(R.string.pkcs11_demo_open_session_6983_hint));
        }
    }

    private void apiLogin() {
        String pin = paramPin.getText() != null ? paramPin.getText().toString() : "";
        if (currentSession == 0) {
            setResult(R.id.result_login, getString(R.string.pkcs11_demo_error_open_session));
            return;
        }
        setResult(R.id.result_login, PKCS11Demo.nativeLogin(currentSession, pin));
    }

    private void apiFindObjects() {
        if (currentSession == 0) {
            setResult(R.id.result_find_objects, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        setResult(R.id.result_find_objects, PKCS11Demo.nativeFindObjects(currentSession));
    }

    private void apiLogout() {
        if (currentSession == 0) {
            setResult(R.id.result_logout, getString(R.string.pkcs11_demo_error_no_session));
            return;
        }
        setResult(R.id.result_logout, PKCS11Demo.nativeLogout(currentSession));
    }

    private void apiCloseSession() {
        if (currentSession == 0) {
            setResult(R.id.result_close_session, getString(R.string.pkcs11_demo_error_no_session));
            return;
        }
        setResult(R.id.result_close_session, PKCS11Demo.nativeCloseSession(currentSession));
        currentSession = 0;
    }

    private void apiGetMechanismList() {
        setResult(R.id.result_get_mechanism_list, PKCS11Demo.nativeGetMechanismList(parseSlotId()));
    }

    private void apiSign() {
        if (currentSession == 0) {
            setResult(R.id.result_sign, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        EditText paramKeyHandle = findViewById(R.id.param_key_handle);
        EditText paramDataHex = findViewById(R.id.param_data_hex);
        EditText paramMechType = findViewById(R.id.param_mech_type);
        long keyHandle = parseLong(paramKeyHandle, 0);
        String dataHex = paramDataHex != null ? paramDataHex.getText().toString().trim() : "";
        long mechType = parseLong(paramMechType, 0x40); // default CKM_SHA256_RSA_PKCS
        if (dataHex.isEmpty()) {
            setResult(R.id.result_sign, "dataHex required (e.g. SHA256 hash as hex)");
            return;
        }
        setResult(R.id.result_sign, PKCS11Demo.nativeSign(currentSession, keyHandle, dataHex, mechType));
    }

    private long parseLong(EditText et, long fallback) {
        if (et == null) return fallback;
        try {
            String s = et.getText() != null ? et.getText().toString().trim() : "";
            if (s.isEmpty()) return fallback;
            return Long.parseLong(s.startsWith("0x") ? s.substring(2) : s, s.startsWith("0x") ? 16 : 10);
        } catch (NumberFormatException e) { return fallback; }
    }

    private void apiGetMechanismInfo() {
        EditText paramMechType = findViewById(R.id.param_mech_info_type);
        long mechType = parseLong(paramMechType, 0x250); // CKM_SHA256
        setResult(R.id.result_get_mechanism_info, PKCS11Demo.nativeGetMechanismInfo(parseSlotId(), mechType));
    }

    private void apiGetSessionInfo() {
        if (currentSession == 0) {
            setResult(R.id.result_get_session_info, getString(R.string.pkcs11_demo_error_open_session));
            return;
        }
        setResult(R.id.result_get_session_info, PKCS11Demo.nativeGetSessionInfo(currentSession));
    }

    private void apiGetAttributeValue() {
        if (currentSession == 0) {
            setResult(R.id.result_get_attribute_value, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        EditText paramObj = findViewById(R.id.param_obj_handle);
        EditText paramAttr = findViewById(R.id.param_attr_type);
        long hObj = parseLong(paramObj, 0);
        long attrType = parseLong(paramAttr, 0x11); // CKA_VALUE
        setResult(R.id.result_get_attribute_value, PKCS11Demo.nativeGetAttributeValue(currentSession, hObj, attrType));
    }

    private void apiDigest() {
        if (currentSession == 0) {
            setResult(R.id.result_digest, getString(R.string.pkcs11_demo_error_open_session));
            return;
        }
        EditText paramData = findViewById(R.id.param_digest_data);
        EditText paramMech = findViewById(R.id.param_digest_mech);
        String dataHex = paramData != null ? paramData.getText().toString().trim() : "";
        long mechType = parseLong(paramMech, 0x250); // CKM_SHA256
        if (dataHex.isEmpty()) {
            setResult(R.id.result_digest, "dataHex required");
            return;
        }
        setResult(R.id.result_digest, PKCS11Demo.nativeDigest(currentSession, dataHex, mechType));
    }

    private void apiVerify() {
        if (currentSession == 0) {
            setResult(R.id.result_verify, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        EditText paramKey = findViewById(R.id.param_verify_key);
        EditText paramData = findViewById(R.id.param_verify_data);
        EditText paramSig = findViewById(R.id.param_verify_sig);
        EditText paramMech = findViewById(R.id.param_verify_mech);
        long keyHandle = parseLong(paramKey, 0);
        String dataHex = paramData != null ? paramData.getText().toString().trim() : "";
        String sigHex = paramSig != null ? paramSig.getText().toString().trim() : "";
        long mechType = parseLong(paramMech, 0x40);
        if (dataHex.isEmpty() || sigHex.isEmpty()) {
            setResult(R.id.result_verify, "dataHex and sigHex required");
            return;
        }
        setResult(R.id.result_verify, PKCS11Demo.nativeVerify(currentSession, keyHandle, dataHex, sigHex, mechType));
    }

    private void apiDecrypt() {
        if (currentSession == 0) {
            setResult(R.id.result_decrypt, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        EditText paramKey = findViewById(R.id.param_decrypt_key);
        EditText paramCipher = findViewById(R.id.param_decrypt_cipher);
        EditText paramMech = findViewById(R.id.param_decrypt_mech);
        long keyHandle = parseLong(paramKey, 0);
        String cipherHex = paramCipher != null ? paramCipher.getText().toString().trim() : "";
        long mechType = parseLong(paramMech, 0x01); // CKM_RSA_PKCS
        if (cipherHex.isEmpty()) {
            setResult(R.id.result_decrypt, "cipherHex required");
            return;
        }
        setResult(R.id.result_decrypt, PKCS11Demo.nativeDecrypt(currentSession, keyHandle, cipherHex, mechType));
    }

    private void apiEncrypt() {
        if (currentSession == 0) {
            setResult(R.id.result_encrypt, getString(R.string.pkcs11_demo_error_login));
            return;
        }
        EditText paramKey = findViewById(R.id.param_encrypt_key);
        EditText paramPlain = findViewById(R.id.param_encrypt_plain);
        EditText paramMech = findViewById(R.id.param_encrypt_mech);
        long keyHandle = parseLong(paramKey, 0);
        String plainHex = paramPlain != null ? paramPlain.getText().toString().trim() : "";
        long mechType = parseLong(paramMech, 0x01); // CKM_RSA_PKCS
        if (plainHex.isEmpty()) {
            setResult(R.id.result_encrypt, "plainHex required");
            return;
        }
        setResult(R.id.result_encrypt, PKCS11Demo.nativeEncrypt(currentSession, keyHandle, plainHex, mechType));
    }

    private void apiGenerateRandom() {
        if (currentSession == 0) {
            setResult(R.id.result_generate_random, getString(R.string.pkcs11_demo_error_open_session));
            return;
        }
        EditText paramLen = findViewById(R.id.param_random_len);
        int len = 32;
        try {
            String s = paramLen != null ? paramLen.getText().toString().trim() : "";
            if (!s.isEmpty()) len = Integer.parseInt(s);
        } catch (NumberFormatException ignored) {}
        setResult(R.id.result_generate_random, PKCS11Demo.nativeGenerateRandom(currentSession, len));
    }
}
