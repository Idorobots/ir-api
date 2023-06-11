# IR API

This project runs on an ESP8266 or ESP32 board and exposes two endpoints that can be used to clone an IR remote commands and then send them on demand. You will need an IR diode for the transmission and an IR demodulator for cloning.

## Scan

```shell
curl -x GET "$HOST/ir/scan"
```

Returns either 404 if no scanned data exists or 200 otherwise:

```json
{
  "protocol": "protocol that was used",
  "code": "data that was sent"
}
```

## Send

```shell
curl -x GET "$HOST/ir/send?protocol=NEC&code=00ff00ff"
```

Query params:

- `protocol` - accepts IR library protocols as well as `RAW_TICKS`, `RAW_BITS` and `RAW_TIMINGS`, defaults to `NEC`.
- `code` - the data to send out via IR,defaults to `""`.
- `frequency` - frequency of transmission, default is `38 kHz`.
- `repeat` - how many times to repeat the transmission, defaults to `0`.
- `tick` - base "tick" length for `RAW_TICKS` & friends, defaults to `560`.
- `hmt` - header mark ticks, defaults to `16`.
- `hst` - header space ticks, defaults to `8`.
- `zmt` - zero bit mark ticks, defaults to `1`.
- `zst` - zero bit space ticks, defaluts to `1`
- `omt` - one bit mark ticks, defaults to `1`.
- `ost` - one bit space ticks, defaults to `3`.
- `fmt` - footer mark ticks, defaults to `1`.
- `fst` - footer space ticks, defaults to `0`.

Returns either 400 when sending failed, or 200 otherwise:

```json
{
  "protocol": "protocol that was used",
  "code": "data that was sent"
}
```
