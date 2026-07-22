#include "COM_PACK.h"
#include "COM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "PID.h"
extern char* PUL_Set_str(char* cmd);
extern char* PUL_Stop_str(char* cmd);
extern PID_Struct MAIN_speedPID;
extern char* MAIN_PidToString(char* cmd);
extern char* MAIN_SetPidTarget(char* cmd);

COMPACK_CmdStruct COMPACK_cmdStructArr[COMPACK_CMD_MAX_NUM] = {
    { "PUL",        COMPACK_CMD_FUNC,       PUL_Set_str },
    { "PUL_Stop",   COMPACK_CMD_FUNC,       PUL_Stop_str },
    { "PID_kp",     COMPACK_CMD_FLOAT,      &(MAIN_speedPID.kp) },
    { "PID_ki",     COMPACK_CMD_FLOAT,      &(MAIN_speedPID.ki) },
    { "PID_kd",     COMPACK_CMD_FLOAT,      &(MAIN_speedPID.kd) },
    { "PID",        COMPACK_CMD_FUNC,       MAIN_PidToString },
    { "PID_target", COMPACK_CMD_FUNC,       MAIN_SetPidTarget },
};



void COMPACK_Init(void) {
}

uint16_t COMPACK_cmdMatchCur[COMPACK_CMD_MAX_NUM];
uint8_t COMPACK_cmdNotMatch[COMPACK_CMD_MAX_NUM];
char* COMPACK_cmdIrrelevantChars = COMPACK_CMD_IRRELEVANT_CHARS;

char COMPACK_cmdFeedbackStr[COMPACK_FDBK_BUF_LEN];

uint8_t COMPACK_CheckCharInString(char ch, char* str) {
    uint16_t i = 0;
    while (str[i] != '\0') {
        if (str[i] == ch) {
            return 1;
        }
        i++;
    }
    return 0;
}

char* COMPACK_ExecuteCmd(uint16_t cmdIndex, char* str) {
    uint16_t i = 0;
    for (i = 0; 
            COMPACK_CheckCharInString(str[i], COMPACK_CMD_SEPARATOR) == 0 && str[i] != '\0';
            i++);
    char temp = str[i];
    str[i] = '\0';
    
    // Start cmd execution
    COMPACK_CmdStruct* curCmdStruct = COMPACK_cmdStructArr + cmdIndex;
    if (curCmdStruct->pData == NULL) {
        goto RET;
    }
    
    static char cmdReturnStr[COMPACK_CMD_RETURN_STR_MAXLEN];
    cmdReturnStr[0] = '\0';
    switch (curCmdStruct->cmdType) {
        case COMPACK_CMD_INT: {
            *((int*)(curCmdStruct->pData)) = atoi(str);
            strcpy(cmdReturnStr, str);
            break;
        }
        case COMPACK_CMD_FLOAT: {
            *((float*)(curCmdStruct->pData)) = (float)atof(str);
            strcpy(cmdReturnStr, str);
            break;
        }
        case COMPACK_CMD_DOUBLE: {
            *((double*)(curCmdStruct->pData)) = atof(str);
            strcpy(cmdReturnStr, str);
            break;
        }
        case COMPACK_CMD_STRING: {
            strcpy((char*)(curCmdStruct->pData), str);
            strcpy(cmdReturnStr, str);
            break;
        }
        case COMPACK_CMD_FUNC: {
            char* strRet = 
                    ((COMPACK_CmdFuncCallback)curCmdStruct->pData)(str);
            strcpy(cmdReturnStr, strRet);
            break;
        }
        default: break;
    }
    
    RET:
    str[i] = temp;
    return cmdReturnStr;
}

void COMPACK_Decode(char* str) {
    // Remove irrelevant chars
    uint16_t i = 0, j = 0;
    for (; str[j] != '\0'; j++) {
        // Check irrelevant
        if (COMPACK_CheckCharInString(str[j], COMPACK_cmdIrrelevantChars)) {
            continue;
        }
        
        // Duplicate char
        str[i] = str[j];
        i++;
    }
    
    if (str[i - 1] != ',') { // add ',' at end if not exist.
        str[i] = ',';
        str[i + 1] = '\0';
    } else {
        str[i] = '\0';
    }

    // COM_TransmitBlocking((uint8_t*)str, strlen(str));

    // Start cmd decoding
    memset(COMPACK_cmdMatchCur, 0, sizeof(COMPACK_cmdMatchCur));
    memset(COMPACK_cmdNotMatch, 0, sizeof(COMPACK_cmdNotMatch));
    memset(COMPACK_cmdFeedbackStr, 0, sizeof(COMPACK_cmdFeedbackStr));
    for (i = 0; str[i] != '\0'; ) {
        
        // Scan cmd
        uint16_t curCmdIndex = 0;
        while (COMPACK_cmdStructArr[curCmdIndex].name != NULL) {
            if (COMPACK_cmdNotMatch[curCmdIndex]) {
                curCmdIndex++;
                continue;
            }

            char* curCmdName = COMPACK_cmdStructArr[curCmdIndex].name;
            uint16_t* curCmdMatchCur = COMPACK_cmdMatchCur + curCmdIndex;
            
            if (COMPACK_CheckCharInString(str[i], COMPACK_CMD_DATA_SEPARATOR)) {
                // Cmd end
                if (curCmdName[*curCmdMatchCur] == '\0') {
                    // Cmd confirm match
                    char* strParam = NULL;
                    if (str[i] == '=') {
                        strParam = str + i + 1; // has a param
                    } else {
                        strParam = str + i;     // doesn't have a param
                    }
                    char* strRet =
                            COMPACK_ExecuteCmd(curCmdIndex, strParam);
                    // Cmd parsed, fdbk
                    strcat(COMPACK_cmdFeedbackStr, curCmdName);
                    strcat(COMPACK_cmdFeedbackStr, ": ");
                    strcat(COMPACK_cmdFeedbackStr, strRet);
                    strcat(COMPACK_cmdFeedbackStr, COMPACK_FDBK_SEPARATOR);
                }
            } else if (curCmdName[*curCmdMatchCur] == str[i]) {
                // Cmd single char match
                (*curCmdMatchCur)++;
            } else {
                // Cmd single char not match
                *curCmdMatchCur = 0;
                COMPACK_cmdNotMatch[curCmdIndex] = 1;
            }
            
            curCmdIndex++;
        }

        // Cmd end
        if (COMPACK_CheckCharInString(str[i], COMPACK_CMD_DATA_SEPARATOR)) {
            // Reset
            memset(COMPACK_cmdMatchCur, 0, sizeof(COMPACK_cmdMatchCur));
            memset(COMPACK_cmdNotMatch, 0, sizeof(COMPACK_cmdNotMatch));

            // Next cmd
            while (COMPACK_CheckCharInString(str[i], COMPACK_CMD_SEPARATOR) == 0 && str[i] != '\0') {
                i++;
            }
            if (str[i] == '\0') {
                // Pack end
                continue;
            }
            // Cmd end
            i++;
            continue;
        }

        i++;
    }
    if (strlen(COMPACK_cmdFeedbackStr) > 0) {
        COM_Transmit((uint8_t*)COMPACK_cmdFeedbackStr, strlen(COMPACK_cmdFeedbackStr));
    }
}
