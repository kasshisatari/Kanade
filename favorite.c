/* Copyright 2020 oscillo

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software
and associated documentation files(the "Software"),
to deal in the Software without restriction,
including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense,
and / or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so,
subject to the following conditions :

The above copyright notice and this permission
notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY
OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "define.h"
#include "favorite.h"

#define DBNAME "favorite.db"
#define SQLTABLE "CREATE TABLE [FAVORITE] ([FAVORITEID] INTEGER,[USERID] INTEGER NOT NULL,[PATH] TEXT NOT NULL,UNIQUE([USERID], [PATH]),PRIMARY KEY(FAVORITEID));"
#define SQLINSERT "INSERT INTO FAVORITE VALUES(?, ?, ?);"
#define SQLDELETE "DELETE FROM FAVORITE WHERE FAVORITEID = ?;"
#define SQLDETAIL "SELECT PATH FROM FAVORITE WHERE FAVORITEID = ?;"
#define SQLLIST "SELECT FAVORITEID, PATH FROM FAVORITE WHERE USERID = ? ORDER BY FAVORITEID ASC LIMIT 30 OFFSET ?;"
#define SQLOPEN "SELECT MAX(FAVORITEID) FROM FAVORITE;"
#define SQLCOUNT "SELECT COUNT(FAVORITEID) FROM FAVORITE WHERE USERID = ?;"

static sqlite3 *db = NULL;
static sqlite3_stmt *insertStmt = NULL;
static sqlite3_stmt *deleteStmt = NULL;
static sqlite3_stmt *listStmt = NULL;
static sqlite3_stmt *openStmt = NULL;
static sqlite3_stmt *countStmt = NULL;
static sqlite3_stmt *detailStmt = NULL;
static unsigned long maxFavoriteId = 0;

static
unsigned long
GetPageNum(
  unsigned long records
)
{
  if (records % PAGERECORDS)
  {
    return records / PAGERECORDS + 1;
  }
  else
  {
    return records / PAGERECORDS;
  }
}

unsigned long /* records */
GetFavoriteList(
  unsigned long pageNo,
  unsigned short userId,
  unsigned long* favoriteIdList,
  char* pathList,
  unsigned char* recordsInPage,
  unsigned long* pageNum
)
{
  int ret = 0;
  unsigned long records = 0;
  unsigned long offset = pageNo * PAGERECORDS;
  const unsigned char *p = NULL;
  sqlite3_bind_int(countStmt, 1, userId);
  sqlite3_step(countStmt);
  records = sqlite3_column_int(countStmt, 0);
  sqlite3_clear_bindings(countStmt);
  sqlite3_reset(countStmt);
  if (0 == records)
  {
    return records;
  }
  sqlite3_bind_int(listStmt, 1, userId);
  sqlite3_bind_int(listStmt, 2, offset);
  ret = sqlite3_step(listStmt);
  *recordsInPage = 0;
  while (SQLITE_ROW == ret)
  {
    favoriteIdList[*recordsInPage] = sqlite3_column_int(listStmt, 0);
    p = sqlite3_column_text(listStmt, 1);
    strcpy(&(pathList[*recordsInPage * PATH_LENGTH_MAX]), p);
    *recordsInPage = *recordsInPage + 1;
    ret = sqlite3_step(listStmt);
  }
  sqlite3_reset(listStmt);
  sqlite3_clear_bindings(listStmt);
  *pageNum = GetPageNum(records);
  return records;
}

void
GetFavoriteDetail(
  unsigned long favoriteId,
  char* path
)
{
  const unsigned char *p = NULL;
  int ret = 0;
  sqlite3_bind_int(detailStmt, 1, favoriteId);
  ret = sqlite3_step(detailStmt);
  if (SQLITE_ROW == ret)
  {
    p = sqlite3_column_text(detailStmt, 0);
    strcpy(path, (const char*)p);
  }
  else
  {
    sqlite3_reset(detailStmt);
    sqlite3_clear_bindings(detailStmt);
    *path = 0;
  }
  sqlite3_reset(detailStmt);
  sqlite3_clear_bindings(detailStmt);
}

unsigned long
AddFavorite(
  unsigned short userId,
  char* path
)
{
  int ret = 0;
  maxFavoriteId++;
  sqlite3_bind_int(insertStmt, 1, maxFavoriteId);
  sqlite3_bind_int(insertStmt, 2, userId);
  sqlite3_bind_text(insertStmt, 3, path, strlen(path), SQLITE_TRANSIENT);
  ret = sqlite3_step(insertStmt);
  if (SQLITE_CONSTRAINT == ret)
  {
    maxFavoriteId--;
    sqlite3_reset(insertStmt);
    sqlite3_clear_bindings(insertStmt);
    return 0;
  }
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
  return maxFavoriteId;
}

void
DeleteFavorite(
  unsigned long favoriteId
)
{
  sqlite3_bind_int(deleteStmt, 1, favoriteId);
  sqlite3_step(deleteStmt);
  sqlite3_reset(deleteStmt);
  sqlite3_clear_bindings(deleteStmt);
}

void
OpenFavorite(
  void
)
{
  int ret = 0;
  FILE *file = fopen(DBNAME, "r");
  if (NULL == file)
  {
    ResetFavorite();
  }
  else
  {
    fclose(file);
  }
  sqlite3_open(DBNAME, &db);
  sqlite3_prepare_v2(db, SQLINSERT, -1, &insertStmt, NULL);
  sqlite3_prepare_v2(db, SQLDELETE, -1, &deleteStmt, NULL);
  sqlite3_prepare_v2(db, SQLLIST, -1, &listStmt, NULL);
  sqlite3_prepare_v2(db, SQLOPEN, -1, &openStmt, NULL);
  sqlite3_prepare_v2(db, SQLDETAIL, -1, &detailStmt, NULL);
  sqlite3_prepare_v2(db, SQLCOUNT, -1, &countStmt, NULL);
  ret = sqlite3_step(openStmt);
  if (SQLITE_ROW == ret)
  {
    maxFavoriteId = (unsigned long)sqlite3_column_int(openStmt, 0);
  }
  else
  {
    maxFavoriteId = 0;
  }
  sqlite3_reset(openStmt);
}

void
CloseFavorite(
  void
)
{
  sqlite3_finalize(insertStmt);
  sqlite3_finalize(deleteStmt);
  sqlite3_finalize(detailStmt);
  sqlite3_finalize(listStmt);
  sqlite3_finalize(openStmt);
  sqlite3_finalize(countStmt);
  sqlite3_close(db);
}

void
LockFavorite(
  void
)
{
}

void
UnlockFavorite(
  void
)
{
}

void
ResetFavorite(
  void
)
{
  char *errMsg = NULL;
  CloseFavorite();
  remove(DBNAME);
  sqlite3_open(DBNAME, &db);
  if (SQLITE_OK != sqlite3_exec(db, SQLTABLE, NULL, NULL, &errMsg))
  {
    sqlite3_free(errMsg);
    errMsg = NULL;
  }
  sqlite3_close(db);
}
