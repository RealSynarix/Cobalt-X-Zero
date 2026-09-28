#include "webhid.h"
#include "config.h"
#include <string.h>
void webhid_init(void){config_init();}
int webhid_handle(uint8_t id,uint8_t *data,uint16_t len,uint8_t *resp,uint16_t *resp_len){
if(!data||!resp||!resp_len||len<4)return -1;
uint8_t cmd=data[0]; uint8_t fid=data[1]; uint16_t dlen=data[2]|(data[3]<<8);
if(dlen>60)dlen=len-4; if(dlen>60)return -1;
uint8_t payload[60]={0}; if(dlen)memcpy(payload,data+4,dlen);
uint8_t out[60]={0}; uint16_t out_len=0; uint8_t status=0;
if(cmd==5){out[0]=0xAA;out[1]=0x55;out_len=2;}
else if(cmd==1){if(config_get_field(fid,out,&out_len)!=0)status=2;}
else if(cmd==2){int r=config_set_field(fid,payload,dlen); if(r!=0)status=5; else config_save();}
else if(cmd==4){config_reset_defaults();config_save();}
else status=1;
resp[0]=cmd;resp[1]=fid;resp[2]=out_len&0xFF;resp[3]=out_len>>8;resp[4]=status;
if(out_len)memcpy(resp+5,out,out_len);
*resp_len=5+out_len; return 0;
}
