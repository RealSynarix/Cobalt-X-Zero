#include "webhid.h"
#include "config.h"
#include <string.h>
#define HEADER_SIZE 4
#define RESP_HEADER 5
#define MAX_PAYLOAD 59
enum { CMD_READ=1, CMD_WRITE=2, CMD_RESET=4, CMD_PING=5 };
enum { STATUS_OK=0, STATUS_BAD_CMD=1, STATUS_BAD_FIELD=2, STATUS_BAD_VALUE=5, STATUS_SAVE_FAILED=6 };
void webhid_init(void){config_init();}
int webhid_handle(uint8_t id,uint8_t *data,uint16_t len,uint8_t *resp,uint16_t *resp_len){
(void)id;
if(!data||!resp||!resp_len||len<HEADER_SIZE)return -1;
uint8_t cmd=data[0];uint8_t fid=data[1];uint16_t dlen=data[2]|(data[3]<<8);
if(dlen>MAX_PAYLOAD||dlen>len-HEADER_SIZE)return -1;
uint8_t payload[MAX_PAYLOAD]={0};uint8_t out[MAX_PAYLOAD]={0};uint16_t out_len=0;uint8_t status=STATUS_OK;
memcpy(payload,data+HEADER_SIZE,dlen);
switch(cmd){
case CMD_PING:out[0]=0xAA;out[1]=0x55;out_len=2;break;
case CMD_READ:if(config_get_field(fid,out,&out_len)!=0){status=STATUS_BAD_FIELD;out_len=0;}break;
case CMD_WRITE:if(config_set_field(fid,payload,dlen)!=0){status=STATUS_BAD_VALUE;}else if(config_save()!=0){status=STATUS_SAVE_FAILED;}break;
case CMD_RESET:config_reset_defaults();if(config_save()!=0)status=STATUS_SAVE_FAILED;break;
default:status=STATUS_BAD_CMD;
}
resp[0]=cmd;resp[1]=fid;resp[2]=out_len&0xFF;resp[3]=out_len>>8;resp[4]=status;
memcpy(resp+RESP_HEADER,out,out_len);*resp_len=RESP_HEADER+out_len;return 0;
}
