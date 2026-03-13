package com.ftsafe.pcsc.demo;

import android.content.Intent;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

public class Pkcs11HubActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pkcs11_hub);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle(R.string.pkcs11_hub_title);
        }
        toolbar.setNavigationOnClickListener(v -> finish());

        findViewById(R.id.btn_integration_test).setOnClickListener(v ->
                startActivity(new Intent(this, PKCS11TestActivity.class)));
        findViewById(R.id.btn_piv_demo).setOnClickListener(v ->
                startActivity(new Intent(this, PKCS11DemoActivity.class)));
    }
}
