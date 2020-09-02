#include "dbProxy.h"

#include <stdio.h>
#include <stdlib.h>

#include "sqlite3.h"

#include "utils.h"

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
    printf("Opening database %s...OK\n", m_dbPath.c_str());

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
                                     feature varchar(1024), \
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

    std::string sql = fem::utils::string_format("replace into t_face(name, feature, featureSize)values('%s', '%s', '%d')", 
        faceName.c_str(), feature, featureSize);
    int retCode = sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &zErrMsg);
    if(retCode != SQLITE_OK){
        fprintf(stderr, "failed to save face features:%s\n", zErrMsg); 
    }        
}

