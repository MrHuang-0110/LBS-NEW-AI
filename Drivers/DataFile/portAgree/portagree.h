#ifndef __PORTAGREE_H
#define __PORTAGREE_H
#include "sys.h"
#include "adc.h"
#include "dataStruct.h"
 
 

#define DEV_PORT_LINKE 		    0x09
#define DEV_PORT_INFO_DATA    0xED

 

 

typedef struct
{
  uint8_t  Head;
  uint8_t  sID;
  uint8_t  oID;
  uint16_t length;
  uint8_t  index;
  uint8_t  data[256];
  uint8_t  crc;
  uint8_t  tard;
}_AGREEMENT;

enum
{
  AGREE_MEN_ERROR,
  AGREE_MEN_OK,
};
 
 
#define FRAME_HEADER    0x5A
#define SRC_ID          0x97
#define DEST_ID         0x98
#define FRAME_FOOTER    0xA5
#define MAX_FRAME_SIZE  300  // 最大帧长度
#define MIN_FRAME_SIZE  8    // 最小帧长度 (头+源+目标+长度+类型+校验+尾)

typedef enum {
    STATE_IDLE,         // 空闲状态
    STATE_HEADER,       // 接收帧头
    STATE_SRC_ID,       // 接收源ID
    STATE_DEST_ID,      // 接收目标ID
    STATE_LENGTH,       // 接收长度
    STATE_TYPE,         // 接收类型
    STATE_DATA,         // 接收数据
    STATE_CHECKSUM,     // 接收校验和
    STATE_FOOTER        // 接收帧尾
} ParserState;

typedef struct {
    ParserState state;              // 当前解析状态
    uint8_t buffer[MAX_FRAME_SIZE]; // 帧缓冲区
    uint16_t index;                // 当前写入位置
    uint16_t expected_length;     // 预期数据长度
	  uint16_t data_bytes_received;
    uint8_t calc_checksum;       // 计算的校验和
    uint8_t frame_type;          // 帧类型
    bool frame_valid;            // 帧有效标志
} FrameParser;


uint8_t calculate_checksum(const uint8_t *data, size_t length);
uint8_t dataAgreeAnalys(_AGREEMENT *_agreement_,uint8_t *data,uint16_t length);
void FreeDevReFrechSource(uint8_t port);
void scan_agreement_data_port_dev(uint8_t index,uint8_t *pData,uint16_t length);
__PORT *getPortHandle(uint8_t index);
SensorBase *getDevBase(uint8_t index);
void frame_parser_init(FrameParser *parser);
bool frame_parser_process_byte(FrameParser *parser, uint8_t byte);
void handle_complete_frame(uint8_t *frame, uint16_t length, uint8_t type, uint16_t data_length);
void process_received_data(void);
void init_portdata_struct(void);
void ResetScanPort(void);
void FindProtDev(void);
void newAiMonitor(void);
#endif
