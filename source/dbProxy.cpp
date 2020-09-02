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

// 加载全部脸谱
bool DBProxy::LoadAllFaces(PersonMapType& personInfoMap)
{
    printf("\nLoading all falses...\n");
    
    int nrow=0;
    int ncolumn = 0;
    char *zErrMsg =NULL;
    char **azResult=NULL; //二维数组存放结果

    const std::string sql = "select name, feature, featureSize from t_face";
    int retCode = sqlite3_get_table(m_db, sql.c_str(), &azResult, &nrow, &ncolumn, &zErrMsg);
    if(retCode != SQLITE_OK){
        fprintf(stderr, "failed to load all faces:%s\n", zErrMsg);
        return false;
    }

    std::vector<std::string> rowValues(ncolumn);
    for(int k = 0; k < nrow; k++){
        rowValues.clear();

        int cellIndex = (k+1)*ncolumn;
        const std::string& faceName(azResult[cellIndex+0]);
        const std::string& featureData(azResult[cellIndex+1]);
        const std::string& featureSize(azResult[cellIndex+2]);
        PersonInfo* person = new PersonInfo(faceName, featureData, featureSize);
        personInfoMap.emplace(faceName, person); 

        printf("\tface: %s\n", faceName.c_str()); 
    }

    printf("Loading all falses...Done\n");
    return true;
}
