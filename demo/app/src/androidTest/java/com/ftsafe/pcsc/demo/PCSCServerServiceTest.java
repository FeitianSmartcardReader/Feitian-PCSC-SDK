package com.ftsafe.pcsc.demo;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import com.ftsafe.pcsc.server.PCSCServerService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * Instrumentation tests for PCSCServerService start/stop flow.
 */
@RunWith(AndroidJUnit4.class)
public class PCSCServerServiceTest {

    private Context context;

    @Before
    public void setUp() {
        context = ApplicationProvider.getApplicationContext();
        PCSCServerService.stop();
    }

    @After
    public void tearDown() {
        PCSCServerService.stop();
    }

    @Test
    public void start_stop_isRunningReflectsState() {
        assertFalse(PCSCServerService.isRunning());

        PCSCServerService.start(context, PCSCServerService.getDefaultPort());
        assertTrue(PCSCServerService.isRunning());

        PCSCServerService.stop();
        assertFalse(PCSCServerService.isRunning());
    }

    @Test
    public void startWithInvalidPort_usesDefaultPort() {
        PCSCServerService.start(context, 0);
        assertTrue(PCSCServerService.isRunning());
        PCSCServerService.stop();
    }

    @Test
    public void startWhenAlreadyRunning_remainsRunning() {
        PCSCServerService.start(context, PCSCServerService.getDefaultPort());
        assertTrue(PCSCServerService.isRunning());

        PCSCServerService.start(context, 12345);
        assertTrue(PCSCServerService.isRunning());

        PCSCServerService.stop();
        assertFalse(PCSCServerService.isRunning());
    }
}
