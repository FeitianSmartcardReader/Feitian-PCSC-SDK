/*
 * Stub implementations for NPA and DTrust card drivers.
 * These require OpenPACE (eac) which is not built for Android.
 * Returning NULL disables these card types.
 */
struct sc_card_driver;

struct sc_card_driver *sc_get_npa_driver(void)
{
    return 0;
}

struct sc_card_driver *sc_get_dtrust_driver(void)
{
    return 0;
}
