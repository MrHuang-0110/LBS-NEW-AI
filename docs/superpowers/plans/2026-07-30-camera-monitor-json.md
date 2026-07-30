# Camera Monitor JSON Format Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Change the camera device JSON output in the monitor from flat properties to a `configs` array format.

**Architecture:** Single-function change in `newAiMonitor()` — replace the flat `idN/x/y/w/h/pp` property writes with a loop that builds an array of anonymous objects inside the `"camera"` object.

**Tech Stack:** C (STM32H723 firmware), json-maker library (Drivers/DataFile/cjson/json-maker.h)

**Global Constraints:**
- Only modify `Drivers/DataFile/portAgree/portagree.c`
- No data structures, protocol parsing, or PikaScript bindings are touched
- Byte offsets for detection data (`data[0]`–`data[39]`) remain unchanged
- Only the `DEV_ID_CAMER` branch inside `newAiMonitor()` is modified

---

### Task 1: Rewrite camera JSON serialization in `newAiMonitor()`

**Files:**
- Modify: `Drivers/DataFile/portAgree/portagree.c:809-858`

**Interfaces:**
- Consumes: `dev_camer->mode` (CAMER_MODE enum), `dev_camer->data[0..39]` (uint8_t[40])
- Produces: JSON string in `json_buffer` with the new camera format

**Old code (lines 809-858):**
```c
case DEV_ID_CAMER:
{
    DEV_CAMER *dev_camer = read_camer((SensorBase *)portDev[i].sensors);
    p = json_objOpen(p,"camer",&remLen);
     p = json_int(p, "mode",dev_camer->mode, &remLen);
     switch((uint8_t)dev_camer->mode)
     {
        case CAMER_MENU_TYPE:
        case CAMER_MODE_TYPE:
        ...
        case CAMER_PHOTO_TYPE:
        p = json_int(p, "id1",dev_camer->data[0], &remLen);
        p = json_int(p, "x",dev_camer->data[1]<<8|dev_camer->data[2], &remLen);
        p = json_int(p, "y",dev_camer->data[3]<<8|dev_camer->data[4], &remLen);
        p = json_int(p, "w",dev_camer->data[5]<<8|dev_camer->data[6], &remLen);
        p = json_int(p, "h",dev_camer->data[7]<<8|dev_camer->data[8], &remLen);
        p = json_int(p, "pp",dev_camer->data[9], &remLen);
        // ... repeated for id2, id3, id4
        break;
     }
     p = json_objClose(p, &remLen);
     break;
}
```

- [ ] **Step 1: Replace the flat properties with array-based format**

Replace the `DEV_ID_CAMER` case block (lines 809-858) with:

```c
case DEV_ID_CAMER:
{
    DEV_CAMER *dev_camer = read_camer((SensorBase *)portDev[i].sensors);
    p = json_objOpen(p, "camera", &remLen);
    p = json_int(p, "mode", dev_camer->mode, &remLen);
    
    // Open configs array
    p = json_arrOpen(p, "configs", &remLen);
    
    switch((uint8_t)dev_camer->mode)
    {
        case CAMER_MENU_TYPE:
        case CAMER_MODE_TYPE:
        case CAMER_FACE_TYPE:
        case CAMER_LABE_TYPE:
        case CAMER_OBJECT_TYPE:
        case CAMER_COLOR_TYPE:
        case CAMER_WAY_TYPE:
        case CAMER_GESTURE_TYPE:
        case CAMER_BODY_TYPE:
        case CAMER_OBJECT_BODY_TYPE:
        case CAMER_PHOTO_TYPE:
        {
            const char *id_names[4] = {"id1", "id2", "id3", "id4"};
            for(int det = 0; det < 4; det++)
            {
                int base = det * 10;
                p = json_objOpen(p, NULL, &remLen);
                p = json_int(p, id_names[det], dev_camer->data[base], &remLen);
                p = json_int(p, "x", dev_camer->data[base+1]<<8|dev_camer->data[base+2], &remLen);
                p = json_int(p, "y", dev_camer->data[base+3]<<8|dev_camer->data[base+4], &remLen);
                p = json_int(p, "w", dev_camer->data[base+5]<<8|dev_camer->data[base+6], &remLen);
                p = json_int(p, "h", dev_camer->data[base+7]<<8|dev_camer->data[base+8], &remLen);
                p = json_int(p, "pp", dev_camer->data[base+9], &remLen);
                p = json_objClose(p, &remLen);
            }
            break;
        }
    }
    
    p = json_arrClose(p, &remLen);
    p = json_objClose(p, &remLen);
    break;
}
```

- [ ] **Step 2: Verify the code compiles**

Open `MDK-ARM/STM32H723.uvprojx` in Keil µVision and build target `STM32H723`. Confirm no errors or warnings.

- [ ] **Step 3: Run pikaPackage.exe if needed**

Only if any `.pyi` files were changed — they were not in this task, so this step can be skipped.

- [ ] **Step 4: Commit**

```bash
git add Drivers/DataFile/portAgree/portagree.c
git commit -m "refactor(monitor): change camera JSON format to configs array

- Rename key 'camer' → 'camera'
- Wrap detection data in 'configs' array of anonymous objects
- Each object contains idN/x/y/w/h/pp fields
- Use loop over 4 detection objects instead of flat property writes

Co-Authored-By: Claude <noreply@anthropic.com>"
```