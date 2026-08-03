# 摄像头标签识别动态 N 组监控 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让主板在摄像头标签识别 (CAMER_LABE_TYPE / 0x04) 模式下,按新规范接收动态 N 组 (N≤25) 检测数据,JSON 监控动态输出 N 组,并提供 `cam_count` PikaScript 接口。

**Architecture:** 摄像头专用数据回调 `setCamerAck` 将变长帧数据直通拷入扩容后的 `DEV_CAMER.data[256]`,帧 length 通过新增的 `SensorBase.data_len` 字段传递,`n_targets = data_len/10` 驱动 JSON 监控循环与 PikaScript 边界。其他设备、其他摄像头模式完全不受影响。

**Tech Stack:** 裸 C (STM32H723 HAL + FreeRTOS),Keil µVision 构建,PikaScript 绑定 (`pikaPackage.exe` 生成)。

**Spec:** `docs/superpowers/specs/2026-08-03-camer-label-dynamic-monitor-design.md`

## Global Constraints

- **无 CLI 构建、无测试套件**:所有构建在 Keil µVision (MDK-ARM/STM32H723.uvprojx) 手动进行。每步"验证"= Keil 构建 0 错误 + 代码审查 + 特定检查点。硬件验证清单在 Task 5。
- **工作区已有未提交 WIP**(上一轮会话遗留):`CLAUDE.md`、`python/_camer.pyi`、`python/pikascript-api/__pikaBinding.c`、`python/pikascript-api/_camer.h`、`python/pikascript-lib/camer/_camer.c`、`python/pikascript-lib/grayv2/_grayv2.c`。**提交时只 `git add` 本计划相关文件**,不要 `git add -A`。
- **编辑 .pyi 后必须重跑 `python/pikaPackage.exe`**(在 `python/` 目录)重新生成 `pikascript-api/` 绑定,再 Keil 重建。
- **现有 C 文件含 GBK 中文注释**:编辑时保持编码,勿转 UTF-8,防 Keil 乱码。
- **禁止标准库 malloc/free**:统一 `pvPortMalloc`/`vPortFree`(本项目未用新 malloc,遵循现状)。
- 帧协议:`_AGREEMENT` 格式 `5A | sID | oID | length(1B) | index | data[N×10] | crc | A5`,length ≤ 255,本计划载荷 ≤ 250B。

## File Structure

| 文件 | 职责 | 改动 |
|---|---|---|
| `Drivers/DataFile/dataStruct/dataStruct.h` | 通用设备基结构 `SensorBase` | + `uint16_t data_len`;声明 `setCamerAck` |
| `Drivers/DataFile/dataStruct/dataStruct.c` | 设备绑定 + 数据分发 | + `setCamerAck()`;摄像头改绑;`identify_and_bind` 摄像头分支 |
| `Drivers/DataFile/camer/camer.h` | 摄像头设备结构 | `data[40]`→`data[256]`,+ `uint8_t n_targets` |
| `Drivers/DataFile/camer/camer.c` | 摄像头设备实现 | `refsh_camer` 只更新 mode |
| `Drivers/DataFile/portAgree/portagree.c` | 帧解析 + 监控 JSON | 传 length → data_len;labe 模式动态循环 |
| `python/_camer.pyi` | Python API 声明 | + `cam_count` |
| `python/pikascript-lib/camer/_camer.c` | Python API 实现 | `cam_data` 边界改;+ `_camer_cam_count` |
| `python/pikascript-api/_camer.h`、`__pikaBinding.c` | **生成物,勿手改** | `pikaPackage.exe` 自动更新 |

---

### Task 1: 设备结构扩容 + 摄像头专用接收回调 (setCamerAck)

**Files:**
- Modify: `Drivers/DataFile/dataStruct/dataStruct.h:14-19`
- Modify: `Drivers/DataFile/dataStruct/dataStruct.c:115-124` (identify_and_bind 摄像头分支),`:130-147` (setAck 之后新增 setCamerAck)
- Modify: `Drivers/DataFile/camer/camer.h:22-27`
- Modify: `Drivers/DataFile/camer/camer.c:34-56` (refsh_camer)

**Interfaces:**
- Consumes: 现有 `SensorBase`(type/findIndex/devId/data[64]/name/setParam)、`DEV_CAMER`、`uartDevices[]`、`devControlQueue`
- Produces:
  - `SensorBase.data_len` (uint16_t) — 本帧数据长度,Task 2 写入,setCamerAck 消费
  - `DEV_CAMER.data[256]`、`DEV_CAMER.n_targets` (uint8_t) — Task 3/4 消费
  - `void setCamerAck(void* self, uint8_t *data)` — 摄像头专用回调,行为同 setAck 但直通 camer->data

- [ ] **Step 1: `dataStruct.h` — SensorBase 加 data_len 字段**

在 `dataStruct.h:14-19` 的 `SensorBase` 内,`devId` 与 `data[64]` 之间加入:

```c
 typedef struct {
   int  type,findIndex,devId;
	 uint16_t data_len;           /* 本帧数据长度(字节),摄像头等变长设备使用 */
	 uint8_t data[64];
   char name[16];
   void (*setParam)(void* self, uint8_t *data);
}SensorBase;
```

文件底部 `setAck` 声明旁加:

```c
void setCamerAck(void* self, uint8_t *data);
```

- [ ] **Step 2: `camer.h` — DEV_CAMER 扩容**

`camer.h:22-27` 改为:

```c
typedef struct
{
	SensorBase base;
	CAMER_MODE mode;
	uint8_t n_targets;   /* 本帧目标数量 N (0~25),由帧长度/10 得出 */
	uint8_t data[256];   /* 标签识别动态帧:N*10 字节,N<=25 */
}DEV_CAMER;
```

- [ ] **Step 3: `dataStruct.c` — 新增 setCamerAck**

在 `setAck()` 函数 (`dataStruct.c:130-147`) 之后新增:

```c
/* 摄像头专用数据回调:变长帧直通 DEV_CAMER.data,跳过 64B 通用中继 */
void setCamerAck(void* self, uint8_t *data)
{
   BaseType_t pxHigherPriorityTaskWoken;

	 SensorBase *base = (SensorBase*)self;
	 DEV_CAMER *camer = (DEV_CAMER*)self;

	 size_t len = base->data_len;
	 if (len > 256) len = 256;                 /* 与 DEV_CAMER.data 匹配,防溢出 */
	 memset(camer->data, 0, sizeof(camer->data));
	 if (len > 0)
		 memcpy(camer->data, data, len);
	 camer->n_targets = (uint8_t)(len / 10);   /* 每组 10 字节,余数丢弃 */
	 if (camer->n_targets > 25) camer->n_targets = 25;

	 UartDeviceContext_t *pDev = &uartDevices[base->devId];
	 xQueueSendFromISR(pDev->devControlQueue, &base, &pxHigherPriorityTaskWoken);
}
```

(`dataStruct.c` 已 `#include "camer.h"` 第 9 行,无需新增 include。)

- [ ] **Step 4: `identify_and_bind` 摄像头分支改绑 setCamerAck**

`dataStruct.c:115-124` 的 `DEV_ID_CAMER` 分支中 `manager->sensors->setParam = setAck;` 改为 `manager->sensors->setParam = setCamerAck;`(仅此一处,其他设备不动)。

- [ ] **Step 5: `camer.c` — refsh_camer 只更新 mode**

`camer.c:34-56` 的 `refsh_camer()` 改为:

```c
void refsh_camer(DEV_CAMER* mt, uint8_t index,uint8_t* data)
{
    (void)data;                              /* 数据已由 setCamerAck 直通写入 */
    mt->mode = (CAMER_MODE)index;
}
```

删除 switch 与 memcpy 逻辑。(参数保留以匹配 `matchineState.c:170` 调用点,该文件不动。)

- [ ] **Step 6: Keil 构建验证**

Keil µVision 打开 `MDK-ARM/STM32H723.uvprojx`,构建目标 `STM32H723`。
预期:0 error(可能有未用变量警告,可忽略)。
**注意**:`dev_camer[8]` 从 8×~144B 增至 8×~360B(+约 1.7KB),若 SRAM2 区 (0x38000000) 溢出,链接报 L6220E — **若发生,执行 Task 5 Step 3 的 AXI 兜底后再继续**。

- [ ] **Step 7: 提交**

```bash
git add Drivers/DataFile/dataStruct/dataStruct.h Drivers/DataFile/dataStruct/dataStruct.c Drivers/DataFile/camer/camer.h Drivers/DataFile/camer/camer.c
git commit -m "feat(camer): expand camera buffer to 256B and add setCamerAck direct path"
```

**临时行为说明**:本任务完成后、Task 2 前,`data_len` 恒为 0 → 摄像头监控显示 0 目标。Task 2 落地后恢复真实数据。这是任务间合法中间态,勿视为 bug。

---

### Task 2: 帧 length 传入 data_len

**Files:**
- Modify: `Drivers/DataFile/portAgree/portagree.c:431-444` (scan_agreement_data_port_dev 的 else 分支)

**Interfaces:**
- Consumes: `SensorBase.data_len` (Task 1)、`rxAGREEMENT.length` (uint16_t)
- Produces: `portDev[index].sensors->data_len` 被帧 length 填充 → setCamerAck 据此计算 N

- [ ] **Step 1: 在 port_data_parsing 调用前写入 data_len**

`portagree.c:431-444`,当前代码:

```c
	 if(dataAgreeAnalys(&rxAGREEMENT,pData,length)!=AGREE_MEN_OK)
	 {
		...
	 }
	 else
	 {
      port_data_parsing(index,rxAGREEMENT.sID,rxAGREEMENT.index,rxAGREEMENT.data);
	 }
```

改为:

```c
	 else
	 {
      if(portDev[index].sensors != NULL)
      {
          portDev[index].sensors->data_len = rxAGREEMENT.length;
      }
      port_data_parsing(index,rxAGREEMENT.sID,rxAGREEMENT.index,rxAGREEMENT.data);
	 }
```

- [ ] **Step 2: Keil 构建验证**

构建目标 `STM32H723`,预期 0 error。

- [ ] **Step 3: 提交**

```bash
git add Drivers/DataFile/portAgree/portagree.c
git commit -m "feat(camer): pass frame length to sensor data_len for dynamic N"
```

---

### Task 3: JSON 监控 — 标签识别模式动态 N 组

**Files:**
- Modify: `Drivers/DataFile/portAgree/portagree.c:818-847` (newAiMonitor 的 DEV_ID_CAMER switch)

**Interfaces:**
- Consumes: `DEV_CAMER.n_targets`、`DEV_CAMER.data[256]` (Task 1)
- Produces: 新 JSON 契约 — labe 模式 `"configs"` 数组动态长度,键名 `id`/`conf`;非 labe 模式保持 `id1..id4`/`pp` 固定 4 组

- [ ] **Step 1: 拆分 switch — labe 单独 case**

`portagree.c:818-847`,当前所有摄像头模式共享一个 case 块(固定 4 组循环)。改为:labe 单独 case(动态),其余模式保持原循环。

```c
								switch((uint8_t)dev_camer->mode)
								{
									case CAMER_LABE_TYPE:
									{
										/* 动态 N 组:按本帧目标数输出,不补齐 */
										int n = (int)dev_camer->n_targets;
										for(int det = 0; det < n; det++)
										{
											int base = det * 10;
											p = json_objOpen(p, NULL, &remLen);
											p = json_int(p, "id", dev_camer->data[base], &remLen);
											p = json_int(p, "x", dev_camer->data[base+1]<<8|dev_camer->data[base+2], &remLen);
											p = json_int(p, "y", dev_camer->data[base+3]<<8|dev_camer->data[base+4], &remLen);
											p = json_int(p, "w", dev_camer->data[base+5]<<8|dev_camer->data[base+6], &remLen);
											p = json_int(p, "h", dev_camer->data[base+7]<<8|dev_camer->data[base+8], &remLen);
											p = json_int(p, "conf", dev_camer->data[base+9], &remLen);
											p = json_objClose(p, &remLen);
										}
										break;
									}
									case CAMER_MENU_TYPE:
									case CAMER_MODE_TYPE:
									case CAMER_FACE_TYPE:
									case CAMER_OBJECT_TYPE:
									case CAMER_COLOR_TYPE:
									case CAMER_WAY_TYPE:
									case CAMER_GESTURE_TYPE:
									case CAMER_BODY_TYPE:
									case CAMER_OBJECT_BODY_TYPE:
									case CAMER_PHOTO_TYPE:
									{
										/* 原有固定 4 组,不动 */
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
```

- [ ] **Step 2: 代码审查确认**

确认:
- labe 分支无 `id_names` 数组、键名为 `"id"` 与 `"conf"`
- 非 labe 分支与改动前逐字节一致(`id1..id4`/`pp`,固定 4 次循环)
- `"configs"` 数组 open/close 在 switch 之外 (portagree.c:816/849) 未被改动 — N=0 时自然输出 `"configs": []`

- [ ] **Step 3: Keil 构建验证**

构建目标 `STM32H723`,预期 0 error。

- [ ] **Step 4: 提交**

```bash
git add Drivers/DataFile/portAgree/portagree.c
git commit -m "feat(camer): dynamic N configs in monitor JSON for label mode"
```

---

### Task 4: PikaScript 接口 — cam_count 新增 + cam_data 边界修改

> **⚠️ 分工约定**:用户手动运行 `pikaPackage.exe` 重新生成绑定 (Task 4 Step 4)。子代理**只改源码** (`_camer.pyi` + `_camer.c`),**不运行** pikaPackage、**不改** `pikascript-api/` 生成物。

**Files:**
- Modify: `python/_camer.pyi` (子代理)
- Modify: `python/pikascript-lib/camer/_camer.c:26-58` (子代理)
- Regenerate: `python/pikascript-api/_camer.h`、`__pikaBinding.c` (**用户**运行 pikaPackage.exe 生成)

**Interfaces:**
- Consumes: `DEV_CAMER.n_targets` (Task 1)、`read_camer()`、`getDevBase()`
- Produces: `pika_float _camer_cam_count(PikaObj *self, pika_float port)` — 返回当前帧 N;`_camer_cam_data` 边界语义更新

- [ ] **Step 1: 编辑 `_camer.pyi`**

`python/_camer.pyi` 追加 `cam_count` 声明(签名/参数名与现有风格一致):

```python
def cam_data(port:float, id:float, obj_id:float)->float:...

def cam_count(port:float)->float:...

def changer_camer_mode(port:float,mode:float):...

def send_hw_mode(port:float,mode:float):...
```

- [ ] **Step 2: 改 `_camer_cam_data` 边界检查**

`python/pikascript-lib/camer/_camer.c:26-58`,将:

```c
    // 2. ������Χ���
    int group = (int)id;
    int field = (int)obj_id;
    if (group < 0 || group >= 4 || field < 0 || field >= 6)
        return 0.0f;
```

改为:

```c
    // 2. ������Χ���
    // id �����Ƿ������� (0~N-1), ���������IDֵ; ���������� (0~25)
    int group = (int)id;
    int field = (int)obj_id;
    if (group < 0 || group > 25 || field < 0 || field >= 6)
        return 0.0f;
    if (group >= (int)camer->n_targets)   /* 此序号不存在 → 0 */
        return 0.0f;
```

其余 switch (field 0~5 的偏移计算) 不动。

- [ ] **Step 3: 新增 `_camer_cam_count`**

在 `_camer_cam_data` 函数之后追加:

```c
pika_float _camer_cam_count(PikaObj *self, pika_float port)
{
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return 0.0f;
    return (pika_float)camer->n_targets;
}
```

- [ ] **Step 4: [用户] 手动运行 pikaPackage.exe 重新生成绑定**

用户在 `python/` 目录手动运行:

```bash
cd python && ./pikaPackage.exe
```

预期:命令成功退出。
验证生成物 — `python/pikascript-api/_camer.h` 应出现新原型:

```c
pika_float _camer_cam_count(PikaObj *self, pika_float port);
```

`python/pikascript-api/__pikaBinding.c` 应出现 `_camer_cam_count` 注册行。
(若 pikaPackage 输出报错,先恢复 .pyi 并检查语法。)

**子代理在此停下,把 Step 1-3 的改动展示给用户,等用户跑完 pikaPackage.exe 并确认后再继续 Step 5。**

- [ ] **Step 5: [用户] Keil 构建验证**

用户构建目标 `STM32H723`,预期 0 error。确认 `_camer_cam_count` 符号被链接(无 unresolved 错误)。

- [ ] **Step 6: 提交**

```bash
git add python/_camer.pyi python/pikascript-lib/camer/_camer.c python/pikascript-api/_camer.h python/pikascript-api/__pikaBinding.c
git commit -m "feat(camer): add cam_count and dynamic bounds to cam_data PikaScript API"
```

(提交由用户在确认构建通过后执行;若用户已自行提交,子代理跳过。)

---

### Task 5: 最终验证 — SRAM2 内存 + 硬件行为清单

**Files:**
- Inspect: `MDK-ARM/STM32H723/STM32H723.map` (构建后新生成)
- Contingency: `Drivers/DataFile/camer/camer.c:8` (dev_camer 段属性,仅溢出时)

**Interfaces:**
- Consumes: 全部 Task 1-4 产物

- [ ] **Step 1: 检查 SRAM2 区余量**

重新构建后,检查 `MDK-ARM/STM32H723/STM32H723.map` 中 `RW_SRAM2` (0x38000000) 执行区域:

```bash
grep -A6 "Execution Region RW_SRAM2" MDK-ARM/STM32H723/STM32H723.map
```

预期:Size + 未用空间 ≤ 0x4000 (16KB),且 `dev_camer` 符号出现在该区,尺寸 = 8 × sizeof(DEV_CAMER) ≈ 2880B。
**注意**:该地址与 legacy malloc 池 `mem4base` (0x38000000, 14KB) 重叠是既有现象(池未使用),只需确认链接无 L6220E 且区域未超 16KB。

- [ ] **Step 2: 溢出兜底 — 移到 AXI RAM (仅当 Step 1 失败)**

若链接报 SRAM2 溢出或余量不足,改 `camer.c:8`:

```c
static DEV_CAMER dev_camer[8];
```

去掉 `__attribute__((section(".DMA_SRAM2_16KB"), aligned(4)))` → 落入默认 AXI RAM (0x24000000)。安全依据:`camer->data` 从不被 DMA 直接访问(只有 `dmaRxBuffer` 需要 DMA 区)。
改后重新构建,确认链接通过,回到 Step 1 复查。

- [ ] **Step 3: 硬件验证清单**

接真实摄像头,`0x04` (labe) 模式下逐项验证(主机串口/USB 读 JSON,Python 脚本调 cam_count/cam_data):

| # | 场景 | 预期 |
|---|---|---|
| 1 | 3 目标帧 | JSON `"configs"` 3 组,键名 `id`/`conf`;`cam_count`=3;`cam_data(0,0,0)`=第1个目标id,`cam_data(0,2,0)`=第3个目标id |
| 2 | 0 目标帧 | JSON `"configs": []`;`cam_count`=0;`cam_data` 任意序号=0 |
| 3 | 25 目标帧 | JSON `"configs"` 25 组,无截断 |
| 4 | 越界 | `cam_data(0,26,0)`=0;`cam_data(0,-1,0)`=0;`cam_data(0,4,0)` (N<4)=0 |
| 5 | 序号≠ID值 | 摄像头帧 id 为 12/13 等非连续值时,`cam_data(0,0,0)` 返回该位实际 id 值(如 12),`cam_data(0,2,0)` 返回第 3 位 id 值(如 13) |
| 6 | 非 labe 模式 | 如 object 模式:JSON 仍固定 4 组 `id1..id4`/`pp`;`cam_data` 行为不变 |

- [ ] **Step 4: 回归 — 其他设备**

验证 motor/gray/grayv2/color/touch/ultrasion 监控 JSON 与功能不受影响(本计划未触碰其数据通路,应无变化)。

- [ ] **Step 5: 提交**

无代码改动则不提交;若执行了 Step 2 兜底:

```bash
git add Drivers/DataFile/camer/camer.c
git commit -m "fix(camer): move dev_camer to AXI RAM due to SRAM2 overflow"
```

---

## Self-Review

**Spec 覆盖检查:**

| Spec 需求 | 任务 |
|---|---|
| SensorBase + data_len | Task 1 Step 1 |
| setCamerAck 直通 + 改绑 | Task 1 Step 3-4 |
| DEV_CAMER.data[256] + n_targets | Task 1 Step 2 |
| refsh_camer 只更新 mode | Task 1 Step 5 |
| 帧 length → data_len | Task 2 |
| JSON labe 动态 N + id/conf 键名 + 非 labe 不动 | Task 3 |
| cam_count 新增 + cam_data 边界 (0~25, 序号≥N→0) | Task 4 |
| SRAM2 余量验证 + AXI 兜底 | Task 5 Step 1-2 |
| 硬件验证 (3/0/25 帧、越界、非 labe、其他设备) | Task 5 Step 3-4 |

**占位符扫描:** 无 TBD/TODO;所有代码步骤含完整可粘贴代码。

**类型一致性:** `data_len` (uint16_t) 在 Task 1 定义、Task 2 写入、setCamerAck 消费;`n_targets` (uint8_t) Task 1 定义、Task 3/4 消费;`_camer_cam_count` 原型在 Task 4 Step 2 生成、Step 4 实现,签名一致 (`pika_float (PikaObj*, pika_float)`)。
