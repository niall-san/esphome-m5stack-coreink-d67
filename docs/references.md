# References

## Primary Target

- Good Display `GDEY0154D67` product page: https://www.good-display.com/product/388.html
- Public product summary: 1.54 inch, `200x200`, black/white, `SSD1681`, SPI, 3.3V.
- Good Display panel marking on our unit: `0154BN-D67-D2`; this appears to be an OEM/custom marking rather than the public product string.

## Vendor Downloads

These are linked from the Good Display product page and should be reviewed
before refining the init sequence:

- `GDEY0154D67 Specification(24.10.22)`
- `IC Driver SSD1681`: https://www.good-display.com/companyfile/520.html
- `GDEY0154D67 ESP32 Sample Code`: https://www.good-display.com/companyfile/1128.html
- `GDEY0154D67 Arduino Sample Code`: https://www.good-display.com/companyfile/611.html

Do not blindly copy vendor sample code into this repository. Use it to confirm
register sequences, command meanings, refresh timings, and busy polarity, then
record the findings in the ledger.

## ESPHome Context

- ESPHome Waveshare e-paper docs: https://esphome.io/components/display/waveshare_epaper/
- ESPHome external components docs: https://esphome.io/components/external_components/
- Matching ESPHome issue: https://github.com/esphome/esphome/issues/10913
- ESPHome PR that added the documented CoreInk M09 model: https://github.com/esphome/esphome/pull/4939

## Upstream Notes

Before opening an upstream ESPHome PR, collect:

- Hardware photos proving the alternate panel marking.
- Exact ESPHome version and build logs.
- Before/after config showing `1.54in-m5coreink-m09` failure, `1.54inv2` workaround, and this component's result.
- Confirmation of busy polarity, reset behaviour, and color polarity.
- A minimal, maintainable init sequence traced to public docs or clean-room interpretation of sample code.

