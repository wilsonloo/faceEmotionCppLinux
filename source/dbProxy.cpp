#include "dbProxy.h"

#include <stdio.h>
#include <stdlib.h>

#include "sqlite3.h"

#include "utils.h"
#include "data_dump.hpp"

bool DBProxy::Init(const std::string& dbPath)
{
    m_dbPath = dbPath;

    // 打开数据库
    printf("Opening database %s...\n", m_dbPath.c_str());
    int ok = sqlite3_open(m_dbPath.c_str(), &m_db);
    if(ok != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        return false;
    }
    printf("Opening database %s... OK\n", m_dbPath.c_str());

    // 检测对应数据库，数据表是否存在
    if(!fixTableFace()){
        return false;
    }

    return true;
}

bool DBProxy::fixTableFace()
{
    int nrow=0;
    int ncolumn = 0;
    char *zErrMsg =NULL;
    char **azResult=NULL; //二维数组存放结果
    
    const char* checkFaceSql = "select * from sqlite_master where type = 'table' and name = 't_face'";
    sqlite3_get_table(m_db, checkFaceSql, &azResult, &nrow, &ncolumn, &zErrMsg);

    if(nrow == 0){
        // 数据表不存
        const char* createFaceSql = "CREATE TABLE t_face( \
                                     name varchar(100) primary key, \
                                     feature blob, \
                                     featureSize unsigned int(11) \
                                     )"; 
        int retCode = sqlite3_exec(m_db, createFaceSql, NULL, NULL, &zErrMsg);
        if(retCode != SQLITE_OK){
            fprintf(stderr, "create table t_face failed: %s\n", zErrMsg);
            return false;
        }else{
            printf("create table t_face ok\n"); 
        }
    }

    return true;
}

void DBProxy::SaveFaceFeature(const std::string& faceName, const char* feature, int featureSize)
{
    char *zErrMsg =NULL;

    // fem::utils::data_dump(stdout, feature, featureSize, std::string("SaveFaceFeature-").append(faceName).c_str());

    const char* sql = "replace into t_face(name, feature, featureSize)values(?, ?, ?)"; 
    sqlite3_stmt* stmt = NULL;
    sqlite3_prepare(m_db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, faceName.c_str(), faceName.size(), NULL);
    sqlite3_bind_blob(stmt, 2, feature, featureSize, NULL);
    sqlite3_bind_int(stmt, 3, featureSize);

    int retCode = sqlite3_step(stmt);
    if(retCode != SQLITE_DONE){
        fprintf(stderr, "failed to save face features:%d: %s\n", retCode, sqlite3_errmsg(m_db)); 
    }        

    sqlite3_finalize(stmt);
}

// 加载全部脸谱
bool DBProxy::LoadAllFaces(PersonMapType& personInfoMap)
{
    printf("\nLoading all faces...\n");

    sqlite3_stmt* stmt = NULL;
    sqlite3_prepare(m_db, "select name, feature, featureSize from t_face", -1, &stmt, NULL);
    
    while(sqlite3_step(stmt)  == SQLITE_ROW){
        const char* faceName = (const char*)(sqlite3_column_text(stmt, 0));
        const void* featureData = sqlite3_column_blob(stmt, 1);
        int featureSize = sqlite3_column_int(stmt, 2);

        PersonInfo* person = new PersonInfo(faceName, featureData, featureSize);
        personInfoMap.emplace(faceName, person); 

        printf("\tface: %s\n", faceName); 
        // fem::utils::data_dump(stdout, featureData, featureSize, std::string("LoadFace-").append(faceName).c_str());
    }

    sqlite3_finalize(stmt);
    
    printf("Loading all faces...Done\n");
    return true;
}
