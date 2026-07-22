#ifndef __COM_PACK_H__
#define __COM_PACK_H__

#include "ti_msp_dl_config.h"

#define COMPACK_CMD_MAX_NUM             32
#define COMPACK_FDBK_BUF_LEN            256
#define COMPACK_CMD_RETURN_STR_MAXLEN   256
#define COMPACK_FDBK_SEPARATOR          "\n"
#define COMPACK_CMD_IRRELEVANT_CHARS    " \r\n\"\t{}"
#define COMPACK_CMD_DATA_SEPARATOR      "=,;"
#define COMPACK_CMD_SEPARATOR           ",;"



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

typedef char* (*COMPACK_CmdFuncCallback)(char*);



void COMPACK_Init(void);
void COMPACK_Decode(char* str);

#endif
