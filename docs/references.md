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

Observed ESP32 archive metadata:

- Page title: `GDEY0154D67 ESP32 Sample Code`
- Page file entry: `A32-GDEY0154D67`
- Page issue time: `2025-02-18 15:21:02`
- Download endpoint: `/comp/xcompanyFile/downloadNew.do?appId=24&fid=1777&id=1128`
- CDN target observed from the endpoint: `https://v4.cecdn.yun300.cn/100001_1909185148/A32-GDEY0154D67.rar`
- Extracted folder inspected locally: `A32-GDEY0154D67-FP4G-20250212`

## Partial Refresh LUT

The 159-byte partial waveform LUT used by this component was cross-checked
between two sources:

- Good Display `GDEY0154D67` ESP32 sample (`A32-GDEY0154D67-FP4G-20250212`),
  inspected locally as reference only.
- ESPHome `waveshare_epaper` `WaveshareEPaper2P9InV2R2` driver
  (`PARTIAL_UPD_2IN9_LUT`), which targets a 2.9 inch panel using the same
  `SSD1681` controller family.

Both sources agree on the waveform bytes. The trailing 6 bytes set VCOM and
gate voltages inline with the `0x32` register write; these have not been
validated against the `GDEY0154D67` datasheet voltage spec.

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
