#ifndef __COM_PACK_H__
#define __COM_PACK_H__

#include "ti_msp_dl_config.h"

#define COMPACK_CMD_MAX_NUM             32
#define COMPACK_CMD_IRRELEVANT_CHARS    " \r\n\"\t{}\\"



typedef enum COMPACK_CmdType_t {
    COMPACK_CMD_INT,
    COMPACK_CMD_FLOAT,
    COMPACK_CMD_DOUBLE,
    COMPACK_CMD_STRING,
    COMPACK_CMD_FUNC
}COMPACK_CmdType;

typedef struct COMPACK_CmdStruct_t {
    char* name;
    COMPACK_CmdType cmdType;
    void* pData;
}COMPACK_CmdStruct;

typedef void (*COMPACK_CmdFuncCallback)(char*);



void COMPACK_Init(void);
void COMPACK_Decode(char* str);

#endif
