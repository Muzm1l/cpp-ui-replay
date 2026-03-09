# Parser Development Status

This implementation now follows the requested parser development flow in the current C++ codebase.

## 1) Locked output schema
- Own: 6 fields (`time,course,speed,depth,x,y`)
- Target: 16 fields (`time,bearing,range,course,speed,depth,f1..f8,x,y`)
- Tube: 10 fields (`time,bearing,range,course,speed,depth,sonar,wake,x,y`)
- Event: 4 fields (`time,sender,type,name`)
- Sonar: 5 fields (`time,weapon,beamX,beamY,intensity`)

Schema/offset/range config is centralized in [parser_format_config.h](parser_format_config.h).

## 2) Line reader
- Splits each line into timestamp + payload by first comma.
- Validates non-empty payload, even hex length, minimum header length, and valid hex chars.
- Logs malformed lines into `badLineLogs`.

## 3) Message classifier
- Routes to family: own/target/torpedo/tube/event/sonar/unknown.
- Uses type byte + source identity for type `0x0d` streams.

## 4) Binary decode helpers
- Central little-endian helpers: `u32`, `i32`, `f32`, `f64`, null-terminated ASCII.
- Central read-advance methods (`readU32`, `readF32`, etc.) to avoid duplicated offset logic.

## 5) Per-message handlers
- Each handler decodes normalized fields and emits row structs.
- Uses schema config offsets and required byte counts.

## 6) Assembler/state layer
- Partial payload merge by key (`family:source:trackId`) using nearest-in-stream line order.
- Handles split logical rows before decode.

## 7) Emitters
- Outputs `Own.csv`, `Target1.csv`, `Tube1.csv`, `Event.txt`, `Sonar.txt`.
- Also keeps `Torpedo.csv` for backward compatibility.

## 8) Verification
- Validates ranges for course/speed/depth/range.
- Checks monotonic time per output family.

## 9) Diagnostics
- Writes `summary.json` with:
  - message counts
  - unknown packet count
  - dropped rows
  - bad line count
  - row counts
  - min/max by field

## 10) Fixtures and regression
- Fixture lines: `tests/parser_fixture.dat` (24 lines)
- Expected baseline notes: `tests/expected_counts.txt`
- Regression script: `tests/run_regression.sh`

## 11) End-to-end validation
Run:

```bash
./tests/run_regression.sh
```

Then load generated files in the app from `tests/out`.
