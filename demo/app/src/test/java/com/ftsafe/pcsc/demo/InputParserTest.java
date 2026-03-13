package com.ftsafe.pcsc.demo;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * Unit tests for InputParser.parsePort and InputParser.parseHost.
 */
public class InputParserTest {

    private static final int DEFAULT_PORT = 35963;

    @Test
    public void parsePort_empty_returnsFallback() {
        assertEquals(DEFAULT_PORT, InputParser.parsePort("", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("   ", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort(null, DEFAULT_PORT));
    }

    @Test
    public void parsePort_valid_returnsParsed() {
        assertEquals(1, InputParser.parsePort("1", DEFAULT_PORT));
        assertEquals(80, InputParser.parsePort("80", DEFAULT_PORT));
        assertEquals(65535, InputParser.parsePort("65535", DEFAULT_PORT));
        assertEquals(35963, InputParser.parsePort("35963", DEFAULT_PORT));
    }

    @Test
    public void parsePort_withWhitespace_returnsParsed() {
        assertEquals(8080, InputParser.parsePort("  8080  ", DEFAULT_PORT));
    }

    @Test
    public void parsePort_invalid_returnsFallback() {
        assertEquals(DEFAULT_PORT, InputParser.parsePort("0", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("-1", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("65536", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("99999", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("abc", DEFAULT_PORT));
        assertEquals(DEFAULT_PORT, InputParser.parsePort("12.34", DEFAULT_PORT));
    }

    @Test
    public void parseHost_empty_returnsFallback() {
        assertEquals("127.0.0.1", InputParser.parseHost("", "127.0.0.1"));
        assertEquals("127.0.0.1", InputParser.parseHost("   ", "127.0.0.1"));
        assertEquals("127.0.0.1", InputParser.parseHost(null, "127.0.0.1"));
    }

    @Test
    public void parseHost_valid_returnsParsed() {
        assertEquals("127.0.0.1", InputParser.parseHost("127.0.0.1", "0.0.0.0"));
        assertEquals("localhost", InputParser.parseHost("localhost", "127.0.0.1"));
        assertEquals("192.168.1.1", InputParser.parseHost("192.168.1.1", "127.0.0.1"));
    }

    @Test
    public void parseHost_withWhitespace_trims() {
        assertEquals("127.0.0.1", InputParser.parseHost("  127.0.0.1  ", "0.0.0.0"));
    }
}
