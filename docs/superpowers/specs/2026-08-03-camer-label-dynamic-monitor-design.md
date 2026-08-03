# Camera 标签识别动态监控格式设计

**日期:** 2026-08-03
**状态:** 待实现
**范围:** 摄像头标签识别 (CAMER_LABE_TYPE / 0x04) 模式的动态目标数支持

## 动机

摄像头标签识别 (0x04) 模式当前按**固定 4 组 (40 字节)** 解析和上传,与设备端新规范不符。新规范要求:每帧动态 N 组 (N≤25),不固定、不补齐。主板接收、JSON 监控、PikaScript 接口三处都需要适配。

本设计**只改标签识别 (0x04) 模式**;其余摄像头模式 (face/object/color/way/gesture/body/photo 等) 保持现状 (固定 4 组)。

## 目标格式

### 摄像头 → 主板 (帧协议,外部规范,主板适配)

```
载荷 = N × 10 字节,N = 本帧目标数 (N ≤ 25),每帧动态
每组 10 字节: id(1B) + x(2B BE) + y(2B BE) + w(2B BE) + h(2B BE) + conf(1B)
每帧按屏幕从左到右排序:x 最小为目标 1,依此类推
id 语义由板端当前激活功能决定:
  - AprilTag: id = 实际码值 (0~586;>255 固定输出 255)
  - 二维码:   id = 排序序号 (第1个=1, 第2个=2, ...)
主机须按帧 length 字段解析 N 组,不得假设固定 4 组/40 字节
```

### 主板 → 主机 (JSON 监控)

标签识别模式动态输出 N 组,其余模式不变:

```json
{
  "camera": {
    "mode": 4,
    "configs": [
      {"id": 12, "x": 320, "y": 364, "w": 72, "h": 76, "conf": 100},
      {"id": 3,  "x": 204, "y": 134, "w": 74, "h": 72, "conf": 98}
    ]
  }
}
```

- `"configs"` 数组长度 = 本帧 N,不补齐;N=0 时 `"configs": []`
- 键名:`id1/id2...` → `id`(数组位置即序号);`pp` → `conf`
- **不加 `"n"` 字段**(数组自带长度,主机按长度解析)
- 仅 `CAMER_LABE_TYPE` 模式这样输出;其他模式保持固定 4 组原格式

## 数据流与现状

```
摄像头 --UART帧--> 主板
  5A | sID | oID | length(1B) | index(0x04) | data[N×10] | crc | A5

[portagree.c]  dataAgreeAnalys() → length 存入 _AGREEMENT.length (局部变量)
[dataStruct.c] setAck() → SensorBase.data[64] 固定 memcpy 64B (通用中继,所有设备共用)
[matchineState.c] vDevControlTask → refsh_camer()
[camer.c]      refsh_camer() → DEV_CAMER.data[40] 固定拷 40B
[portagree.c]  newAiMonitor() → "configs" 数组,固定循环 4 次 (id1~id4, x,y,w,h,pp)
[python/_camer.c] cam_data(port,id,obj_id) → group 硬编码 0~3
```

## 关键约束 (已核实)

| 环节 | 现状 | 对新格式 (载荷 ≤ 250B) |
|---|---|---|
| 帧 length 字段 | 1B (max 255) | ✓ 250B 放得下 |
| DMA RX 缓冲 | 300B | ✓ 257B 整帧放得下 |
| 帧解析缓冲 MAX_FRAME_SIZE | 300B | ✓ |
| _AGREEMENT.data | 256B | ✓ 250B 放得下 |
| SensorBase.data | **64B** | ✗ 需绕开 (见下方 setCamerAck) |
| DEV_CAMER.data | **40B** | ✗ 需扩容到 ≥250B |
| JSON 缓冲 | 10KB 静态 | ✓ 25 组约 1.5KB |
| 端口超时 | 50 ticks (≈50ms) | ✓ 一帧最多 ~8ms @115200 |

## 设计

### 1. 接收链路:摄像头专用数据回调 (setCamerAck)

`SensorBase.data[64]` 是 8 种设备共用的通用中继,不能为摄像头一家扩到 256B (会让全部设备 × 8 端口膨胀约 12KB,挤爆 SRAM2 的 16KB 区)。代码库已有 `setParam` 函数指针分发机制 (`identify_and_bind` 给每个设备绑回调),给摄像头绑一个专用回调直通到 `DEV_CAMER.data[256]`,跳过 64B 中继。**其他设备完全不受影响。**

| 文件 | 改动 |
|---|---|
| `Drivers/DataFile/dataStruct/dataStruct.h` | `SensorBase` 加 `uint16_t data_len` (2B, 8 设备 × 8 端口共 +128B,可忽略) |
| `Drivers/DataFile/dataStruct/dataStruct.c` | 新增 `setCamerAck()`:按 `data_len` (上限 256) 拷入 `camer->data`,计算 `n_targets = data_len / 10`,入队;`identify_and_bind` 中 `DEV_ID_CAMER` 改用 `setCamerAck` |
| `Drivers/DataFile/portAgree/portagree.c` | `scan_agreement_data_port_dev` 在 `dataAgreeAnalys` 成功后、`port_data_parsing` 前:`portDev[index].sensors->data_len = rxAGREEMENT.length` |
| `Drivers/DataFile/camer/camer.h` | `DEV_CAMER.data[40]` → `data[256]` (25 组 × 10 = 250,留 6B 余量);新增 `uint8_t n_targets` |
| `Drivers/DataFile/camer/camer.c` | `refsh_camer` 只更新 mode (数据拷贝已由 setCamerAck 完成);其余不动 |

ISR 写 `camer->data` / 任务读,单一写者 + 队列同步,与现有模式一致。

### 2. JSON 监控

`newAiMonitor()` 的 `DEV_ID_CAMER` case,仅 `CAMER_LABE_TYPE`:

```c
// 现: for(int det = 0; det < 4; det++)  →  改为:
int n = dev_camer->n_targets;
for(int det = 0; det < n; det++) {
    p = json_objOpen(p, NULL, &remLen);
    p = json_int(p, "id",  dev_camer->data[base], &remLen);
    p = json_int(p, "x",   data[base+1]<<8|data[base+2], &remLen);
    p = json_int(p, "y",   data[base+3]<<8|data[base+4], &remLen);
    p = json_int(p, "w",   data[base+5]<<8|data[base+6], &remLen);
    p = json_int(p, "h",   data[base+7]<<8|data[base+8], &remLen);
    p = json_int(p, "conf", data[base+9], &remLen);
    p = json_objClose(p, &remLen);
}
```

- N=0 → 循环不执行,输出 `"configs": []`
- `id1/id2...` → `id`,`pp` → `conf`(仅 labe 模式)
- 非 labe 模式:原固定 4 组不动

### 3. PikaScript 接口

**签名不变**(参数名不动):

```python
def cam_data(port:float, id:float, obj_id:float)->float:...
def cam_count(port:float)->float:...    # 新增:当前帧目标数 N
def changer_camer_mode(port:float,mode:float):...
def send_hw_mode(port:float,mode:float):...
```

**`cam_data(port, id, obj_id)` 语义**(锁死,消除歧义):

- `id` 参数 = **数据序号 (0-based, 0=第1个/屏幕最左)**,**不是目标 ID 值**
  - `obj_id` 参数 = 字段:0=id值, 1=x, 2=y, 3=w, 4=h, 5=conf
- 返回规则:`id < 0 || id > 25` → 0;`id >= 当前帧 N`(此序号不存在)→ 0;否则返回该组字段
- 示例(帧:ID12 在第 1 位,ID13 在第 3 位,共 4 个目标):
  - `cam_data(0, 0, 0)` → 12(序号 0 的 id 值)
  - `cam_data(0, 2, 0)` → 13(序号 2 的 id 值)
  - `cam_data(0, 2, 5)` → 98(序号 2 的 conf)
  - `cam_data(0, 4, 0)` → 0(序号 4 ≥ N=4,不存在)
  - `cam_data(0, 26, 0)` → 0(>25)

**`_camer_cam_data` 实现改动**:边界检查 `group >= 4` → `group >= camer->n_targets`,加 `group > 25` 上限;`obj_id` 仍 0~5。

**`_camer_cam_count` 实现**:`return camer->n_targets`。

| 文件 | 改动 |
|---|---|
| `python/_camer.pyi` | 加 `cam_count` 声明 |
| `python/pikascript-lib/camer/_camer.c` | 改 `_camer_cam_data` 边界;新增 `_camer_cam_count` |
| — | 重跑 `python/pikaPackage.exe` 重新生成绑定 + Keil 重建 |

## 内存注意点

- `dev_camer[8]` 在 `.DMA_SRAM2_16KB` (0x38000000 区域, 16KB),data 40→256 每端口 +216B,共 +1728B
- 当前 map 文件是旧构建 (不含 dev_camer 符号),**SRAM2 余量需重新编译后从新 map 确认**
- 兜底:若 SRAM2 放不下,把 `dev_camer` 移到 AXI RAM (0x24000000, 512KB)— 安全,因 `camer->data` 从不被 DMA 直接访问 (只有 `dmaRxBuffer` 需要 DMA 区)

## 文件清单

| 文件 | 改动 |
|---|---|
| `Drivers/DataFile/dataStruct/dataStruct.h` | `data_len` 字段 |
| `Drivers/DataFile/dataStruct/dataStruct.c` | `setCamerAck` + `identify_and_bind` 摄像头改绑 |
| `Drivers/DataFile/portAgree/portagree.c` | 传 length;监控动态循环 (labe) |
| `Drivers/DataFile/camer/camer.h` | `data[256]` + `n_targets` |
| `Drivers/DataFile/camer/camer.c` | `refsh_camer` 简化 |
| `python/_camer.pyi` | `cam_count` 声明 |
| `python/pikascript-lib/camer/_camer.c` | 边界改 + `cam_count` 实现 |
| — | `pikaPackage.exe` + Keil 重建 |

## 验证

1. Keil 构建,查新 map 的 SRAM2 余量 (dev_camer 扩容后)
2. 接真实摄像头 0x04 模式:
   - 3 目标帧:JSON 输出 3 组;`cam_count`=3;`cam_data(0,0,0)`/`(0,2,0)` 取对
   - 0 目标帧:JSON 输出 `"configs": []`;`cam_count`=0;`cam_data` 全 0
   - 25 目标帧:JSON 输出 25 组
   - 越界:`cam_data(0,26,0)`=0,`(0,-1,0)`=0,`(0,4,0)` (N<4)=0
   - 非 labe 模式 (如 object):仍固定 4 组,`cam_data` 语义不变
3. 其余设备 (motor/gray/color/...) 监控与功能不受影响

## Review

- [ ] 待实现后复核
