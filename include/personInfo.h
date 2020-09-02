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

    PersonInfo(const std::string& name, const std::string& featureData, const std::string& featureSize)
    {
        this->name = name;
        this->feature.featureSize = atoi(featureSize.c_str());
        if(this->feature.featureSize > 0){
            this->feature.feature = (MByte*)malloc(this->feature.featureSize);
            memset(this->feature.feature, 0, this->feature.featureSize);
            memcpy(this->feature.feature, featureData.c_str(), this->feature.featureSize);
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
