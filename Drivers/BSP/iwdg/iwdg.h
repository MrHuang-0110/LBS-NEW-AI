#ifndef __BSP_IWDG_H
#define __BSP_IWDG_H

void iwdg_init(uint32_t prer, uint16_t rlr);
void iwdg_feed(void);

#endif
