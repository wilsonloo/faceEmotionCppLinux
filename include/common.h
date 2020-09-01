#ifndef _FACE_EMOTION_COMMON_H_
#define _FACE_EMOTION_COMMON_H_

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <string.h>

namespace fem
{
    //时间戳转换为日期格式
    static void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
    {
        time_t tTimeStamp = atoll(timeStamp);
        struct tm* pTm = gmtime(&tTimeStamp);
        strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
    }
}

#endif
