#include "_camer.h"
#include "camer.h"
#include "uart.h"
#include "portagree.h"
#include "PikaPlatform.h"   /* pika_platform_sleep_ms: vTaskDelay + GIL release */

void _camer_changer_camer_mode(PikaObj *self, pika_float port, pika_float mode)
{
  DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
  if(camer== NULL)return;

  uint8_t u8_data = (uint8_t)mode;

		MultiUart_SendFrame((UartDevice_t)port,
												&u8_data,
												1,
												DEV_ID_CAMER,
												0xFF,
												10,250);

}
void _camer_changer_waite_camer_mode(PikaObj *self, pika_float port, pika_float mode)
{
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return;

    /* Send switch cmd frame: 5A 97 A7 01 FF <mode> chk A5 */
    uint8_t u8_data = (uint8_t)mode;
    MultiUart_SendFrame((UartDevice_t)port,
                        &u8_data,
                        1,
                        DEV_ID_CAMER,
                        0xFF,
                        10, 250);

    /* Dead-wait until mode effective: camera replies with data frame of the
       new mode type and refsh_camer updates camer->mode. Poll with
       pika_platform_sleep_ms (releases GIL); 2s timeout guard against hang.
       Note: 0x14/0x15 are switch-cmd modes only; camera data frames for
       both label sub-modes still use type 0x04 (label recognition). */
    uint32_t expect = (uint32_t)mode;
    if (expect == 0x14 || expect == 0x15)
        expect = CAMER_LABE_TYPE;           /* 0x04 */
   // uint32_t timeout_ms = 2000;
    while ((uint32_t)camer->mode != expect)
    {
        pika_platform_sleep_ms(10);
			    
       // timeout_ms -= (timeout_ms >= 10) ? 10 : timeout_ms;
    }
}

/* NUL-terminated copy buffer for returned names (name frame has no NUL).
   PikaScript is single-threaded and method_returnStr copies immediately. */
static char s_camer_name_buf[256];

static char *camer_get_name(const DEV_CAMER *camer, uint8_t id)
{
    uint8_t nlen = 0;
    const char *nm = camer_find_name(camer, id, &nlen);
    if (nm == NULL || nlen == 0) return "";
    if (nlen >= sizeof(s_camer_name_buf)) nlen = sizeof(s_camer_name_buf) - 1;
    memcpy(s_camer_name_buf, nm, nlen);
    s_camer_name_buf[nlen] = '\0';
    return s_camer_name_buf;
}

pika_float _camer_cam_dis_obj_id(PikaObj *self, pika_float port, pika_float obj, pika_float data)
{
    /* obj = screen order index 0~N-1 (for un-learned targets, id=0),
       data = field: 0=id, 1=x, 2=y, 3=w, 4=h, 5=conf */
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return 0.0f;

    int group = (int)obj;
    int field = (int)data;
    if (group < 0 || group >= (int)camer->n_targets || field < 0 || field >= 6)
        return 0.0f;

    uint8_t *d = camer->data;
    int off = group * 10;                    /* 10 bytes per target */
    switch (field) {
        case 0: return (pika_float)d[off];                            /* slot id (0 if un-learned) */
        case 1: return (pika_float)((d[off+1] << 8) | d[off+2]);      /* x BE */
        case 2: return (pika_float)((d[off+3] << 8) | d[off+4]);      /* y BE */
        case 3: return (pika_float)((d[off+5] << 8) | d[off+6]);      /* w BE */
        case 4: return (pika_float)((d[off+7] << 8) | d[off+8]);      /* h BE */
        case 5: return (pika_float)(d[off+9] & 0x7F);                 /* conf 0~100, learned bit masked */
    }
    return 0.0f;
}

pika_float _camer_cam_dis_id(PikaObj *self, pika_float port, pika_float obj, pika_float data)
{
    /* obj = learned registration slot id 1~25 (for learned targets).
       Scan current frame targets for id == slot, return its field. */
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return 0.0f;

    int slot = (int)obj;
    int field = (int)data;
    if (slot < 1 || slot > 25 || field < 0 || field >= 6)
        return 0.0f;

    for (int i = 0; i < (int)camer->n_targets; i++)
    {
        int off = i * 10;
        if (camer->data[off] != (uint8_t)slot)
            continue;
        switch (field) {
            case 0: return (pika_float)camer->data[off];
            case 1: return (pika_float)((camer->data[off+1] << 8) | camer->data[off+2]);
            case 2: return (pika_float)((camer->data[off+3] << 8) | camer->data[off+4]);
            case 3: return (pika_float)((camer->data[off+5] << 8) | camer->data[off+6]);
            case 4: return (pika_float)((camer->data[off+7] << 8) | camer->data[off+8]);
            case 5: return (pika_float)(camer->data[off+9] & 0x7F);
        }
    }
    return 0.0f;   /* slot not present in current frame */
}

char* _camer_cam_dis_obj_name(PikaObj *self, pika_float port, pika_float obj)
{
    /* obj = screen order index 0~N-1; lookup name by that target's id. */
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return "";

    int group = (int)obj;
    if (group < 0 || group >= (int)camer->n_targets) return "";

    return camer_get_name(camer, camer->data[group * 10]);
}

char* _camer_cam_dis_name(PikaObj *self, pika_float port, pika_float obj)
{
    /* obj = learned registration slot id 1~25; lookup name directly by id. */
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return "";

    int slot = (int)obj;
    if (slot < 1 || slot > 25) return "";

    return camer_get_name(camer, (uint8_t)slot);
}

pika_float _camer_cam_count(PikaObj *self, pika_float port)
{
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return 0.0f;
    return (pika_float)camer->n_targets;
}
