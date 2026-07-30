# Camera Monitor JSON Format Redesign

**Date:** 2026-07-30
**Status:** Approved design, ready for implementation
**Files changed:** `Drivers/DataFile/portAgree/portagree.c` (only)

## Motivation

The monitor JSON output for the AI camera device (`DEV_ID_CAMER`, 0xA7) currently uses a flat property layout inside the `"camer"` object. This format is inconsistent with the structured JSON conventions used elsewhere and makes it harder for downstream consumers (monitoring app, debug tools) to iterate over detection results.

## Current Format

```json
{
  "camer": {
    "mode": 4,
    "id1": 1, "x": 320, "y": 364, "w": 72, "h": 76, "pp": 100,
    "id2": 2, "x": 204, "y": 134, "w": 74, "h": 72, "pp": 100,
    "id3": 3, "x": 0, "y": 0, "w": 0, "h": 0, "pp": 0,
    "id4": 4, "x": 0, "y": 0, "w": 0, "h": 0, "pp": 0
  }
}
```

Problems:
- Detection objects are flat properties, not array elements — cannot iterate
- Key `"camer"` is inconsistent with the module name `"camera"` used elsewhere
- Repeated property names (`"x"`, `"y"`, etc.) make JSON parsing ambiguous

## Target Format

```json
{
  "camera": {
    "mode": 4,
    "configs": [
      {"id1": 1, "x": 320, "y": 364, "w": 72, "h": 76, "pp": 100},
      {"id2": 2, "x": 204, "y": 134, "w": 74, "h": 72, "pp": 100},
      {"id3": 3, "x": 0, "y": 0, "w": 0, "h": 0, "pp": 0},
      {"id4": 4, "x": 0, "y": 0, "w": 0, "h": 0, "pp": 0}
    ]
  }
}
```

Key improvements:
- `"camer"` → `"camera"` (consistent naming)
- Detection data wrapped in `"configs"` array — consumers can iterate
- Each array element is a self-contained config object
- ID field names (`id1`–`id4`) encode the detection index

## Data Layout (unchanged)

The camera device sends 40 bytes of detection data in `dev_camer->data[40]`:

| Bytes | Field | Object |
|-------|-------|--------|
| 0     | id1   | 0      |
| 1–2   | x1    | 0      |
| 3–4   | y1    | 0      |
| 5–6   | w1    | 0      |
| 7–8   | h1    | 0      |
| 9     | pp1   | 0      |
| 10–19 | ...   | 1      |
| 20–29 | ...   | 2      |
| 30–39 | ...   | 3      |

Each 10-byte slice: id (1B) + x (2B) + y (2B) + w (2B) + h (2B) + pp (1B).

## Implementation

**File:** `Drivers/DataFile/portAgree/portagree.c`, function `newAiMonitor()`, `DEV_ID_CAMER` case (lines 809–858).

**Changes:**
1. `json_objOpen(p, "camer", &remLen)` → `json_objOpen(p, "camera", &remLen)`
2. After writing `"mode"`, open a `"configs"` array: `json_arrOpen(p, "configs", &remLen)`
3. Replace the current flat property writes with a loop body that opens an anonymous object, writes all 6 fields, closes it — repeated 4 times
4. Close the array: `json_arrClose(p, &remLen)`

**No other files touched.** No data structures, no protocol parsing, no PikaScript bindings. The rest of the monitor JSON (other devices, flash, mem, battery, etc.) is unaffected.

## Review

- [x] Placeholder scan: no TBDs, TODOs, or vague requirements
- [x] Internal consistency: format matches the example, all data fields accounted for
- [x] Scope: single-file formatting change, appropriate for one implementation plan
- [x] Ambiguity: the exact JSON structure and data byte offsets are fully specified