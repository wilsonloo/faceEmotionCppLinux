#ifndef _FACE_EMOTION_PERSON_INFO_H_
#define _FACE_EMOTION_PERSON_INFO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <memory>
#include <string.h>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

struct PersonInfo
{
    std::string     name;
    ASF_FaceFeature feature;

    PersonInfo(const char* name, const void* featureData, int featureSize)
    {
        this->name.assign(name);

        memset(&feature, 0, sizeof(feature));

        this->feature.featureSize = featureSize;
        if(this->feature.featureSize > 0){
            this->feature.feature = (MByte*)malloc(featureSize);
            memset(this->feature.feature, 0, featureSize);
            memcpy(this->feature.feature, featureData, featureSize);
        }else{
            this->feature.feature = NULL;
        }
    }

    virtual ~PersonInfo()
    {
        if(this->feature.feature != NULL){
            free(this->feature.feature);
            this->feature.feature = NULL;
        }
    }
};

typedef std::map<std::string, PersonInfo*> PersonMapType;

#endif
