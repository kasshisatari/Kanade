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
#include <pthread.h>
#include "sqlite3.h"
#include "define.h"
#include "history.h"
#include "user.h"

#define DBNAME "history.db"
#define SQLTABLE "CREATE TABLE [HISTORY] ( [HISTORYID] INTEGER NOT NULL UNIQUE, [USERID] INTEGER NOT NULL, [COMMENT] TEXT, [PATH] TEXT NOT NULL, [PLAYTIME] TEXT NOT NULL, PRIMARY KEY(HISTORYID));"
#define SQLINSERT "INSERT INTO HISTORY VALUES(?, ?, ?, ?, ?);"
#define SQLMAXID "SELECT MAX(HISTORYID) FROM HISTORY;"
#define SQLDELETE "DELETE FROM HISTORY WHERE HISTORYID = ?;"
#define SQLUPDATE "UPDATE HISTORY SET COMMENT = ? WHERE HISTORYID = ?;"
#define SQLHAS "SELECT COUNT(HISTORYID) FROM HISTORY WHERE PATH = ?;"
#define SQLCOUNT "SELECT COUNT(HISTORYID) FROM HISTORY;"
#define SQLLIST "SELECT HISTORYID, USERID, COMMENT, PATH, PLAYTIME FROM HISTORY ORDER BY PLAYTIME DESC LIMIT 30 OFFSET ?;"
#define SQLCSV "SELECT HISTORYID, USERID, COMMENT, PATH, PLAYTIME FROM HISTORY ORDER BY PLAYTIME ASC;"

static pthread_mutex_t lock;
static sqlite3 *db = NULL;
static sqlite3_stmt *insertStmt = NULL;
static sqlite3_stmt *maxIdStmt = NULL;
static sqlite3_stmt *deleteStmt = NULL;
static sqlite3_stmt *updateStmt = NULL;
static sqlite3_stmt *countStmt = NULL;
static sqlite3_stmt *hasStmt = NULL;
static sqlite3_stmt *listStmt = NULL;
static sqlite3_stmt *csvStmt = NULL;
static unsigned long maxHistoryId = 0;

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
GetHistoryList(
  unsigned long pageNo,
  unsigned long* historyIdList,
  unsigned short* userIdList,
  char* filePathList,
  char* commentList,
  char* playTimeList,
  unsigned char* recordsInPage,
  unsigned long* pageNum
)
{
  int ret = 0;
  unsigned long records = 0;
  unsigned long offset = (pageNo - 1) * PAGERECORDS;
  const unsigned char *p = NULL;
  sqlite3_step(countStmt);
  records = sqlite3_column_int(countStmt, 0);
  sqlite3_reset(countStmt);
  if (0 == records)
  {
    return records;
  }
  sqlite3_bind_int(listStmt, 1, offset);
  ret = sqlite3_step(listStmt);
  *recordsInPage = 0;
  while (SQLITE_ROW == ret)
  {
    historyIdList[*recordsInPage] = sqlite3_column_int(listStmt, 0);
    userIdList[*recordsInPage] = sqlite3_column_int(listStmt, 1);
    p = sqlite3_column_text(listStmt, 2);
    strcpy(&(commentList[*recordsInPage * COMMENT_LENGTH_MAX]), p);
    p = sqlite3_column_text(listStmt, 3);
    strcpy(&(filePathList[*recordsInPage * PATH_LENGTH_MAX]), p);
    p = sqlite3_column_text(listStmt, 4);
    strcpy(&(playTimeList[*recordsInPage * PLAYTIME_LENGTH_MAX]), p);
    *recordsInPage = *recordsInPage + 1;
    ret = sqlite3_step(listStmt);
  }
  sqlite3_reset(listStmt);
  sqlite3_clear_bindings(listStmt);
  *pageNum = GetPageNum(records);
  return records;
}

unsigned long /* History ID */
AddHistory(
  unsigned short userId,
  char* path,
  char* comment,
  char* playTime
)
{
  maxHistoryId++;
  sqlite3_bind_int(insertStmt, 1, maxHistoryId);
  sqlite3_bind_int(insertStmt, 2, userId);
  sqlite3_bind_text(insertStmt, 3, comment, strlen(comment), SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 4, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 5, playTime, strlen(playTime), SQLITE_TRANSIENT);
  sqlite3_step(insertStmt);
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
  return maxHistoryId;
}

// #define SQLCSV "SELECT HISTORYID, USERID, COMMENT, PATH, PLAYTIME FROM HISTORY ORDER BY PLAYTIME ASC;"
void
ExtractHistory(
  unsigned long historyId,
  char* fileName
)
{
  int ret = 0;
  unsigned char extract = 0;
  FILE *file = NULL;
  unsigned long histId;
  unsigned long userId;
  char userName[128];
  const char *p = NULL;
  const char *song = NULL;
  const char *path = NULL;
  file = fopen(fileName, "w");
  fputc(0xEF, file);
  fputc(0xBB, file);
  fputc(0xBF, file);
        fprintf(file,"time,user,comment,file,path\n");
  ret = sqlite3_step(csvStmt);
  while (SQLITE_ROW == ret)
  {
    if (0 == extract)
    {
      histId = sqlite3_column_int(csvStmt, 0);
      if (histId == historyId)
      {
        extract = 1;
      }
    }
    if (1 == extract)
    {
      // Time
      p = sqlite3_column_text(csvStmt, 4);
      fprintf(file, "%s,", p);
      // User
      userId = sqlite3_column_int(csvStmt, 1);
      GetUserName(userId, userName);
      fprintf(file, "%s,", userName);
      // Comment
      p = sqlite3_column_text(csvStmt, 2);
      fprintf(file, "%s,", p);
      // File
      p = sqlite3_column_text(csvStmt, 3);
      path = p;
      while (0 != *p)
      {
        if ('/' == *p)
        {
          song = p + 1;
        }
        p++;
      }
      fprintf(file, "%s,", song);
      // Path
      fprintf(file, "%s\n", path);
    }
    ret = sqlite3_step(csvStmt);
  }
  sqlite3_reset(csvStmt);
  fclose(file);
}

void
InitHistory(
  void
)
{
  pthread_mutex_init(&lock, 0);
}

void
OpenHistory(
  void
)
{
  int ret = 0;
  FILE *file = fopen(DBNAME, "r");
  if (NULL == file)
  {
    ResetHistory();
  }
  else
  {
    fclose(file);
  }
  sqlite3_open(DBNAME, &db);
  sqlite3_prepare_v2(db, SQLINSERT, -1, &insertStmt, NULL);
  sqlite3_prepare_v2(db, SQLMAXID, -1, &maxIdStmt, NULL);
  sqlite3_prepare_v2(db, SQLDELETE, -1, &deleteStmt, NULL);
  sqlite3_prepare_v2(db, SQLUPDATE, -1, &updateStmt, NULL);
  sqlite3_prepare_v2(db, SQLCOUNT, -1, &countStmt, NULL);
  sqlite3_prepare_v2(db, SQLLIST, -1, &listStmt, NULL);
  sqlite3_prepare_v2(db, SQLHAS, -1, &hasStmt, NULL);
  sqlite3_prepare_v2(db, SQLCSV, -1, &csvStmt, NULL);
  ret = sqlite3_step(maxIdStmt);
  if (SQLITE_ROW == ret)
  {
    maxHistoryId = sqlite3_column_int(maxIdStmt, 0);
  }
  else
  {
    maxHistoryId = 0;
  }
  sqlite3_reset(maxIdStmt);
  sqlite3_finalize(maxIdStmt);
}

void
CloseHistory(
  void
)
{
  sqlite3_finalize(insertStmt);
  sqlite3_finalize(deleteStmt);
  sqlite3_finalize(updateStmt);
  sqlite3_finalize(countStmt);
  sqlite3_finalize(listStmt);
  sqlite3_finalize(hasStmt);
  sqlite3_finalize(csvStmt);
  sqlite3_close(db);
}

void
LockHistory(
  void
)
{
  pthread_mutex_lock(&lock);
}

void
UnlockHistory(
  void
)
{
  pthread_mutex_unlock(&lock);
}

void
ResetHistory(
  void
)
{
  char *errMsg = NULL;
  CloseHistory();
  remove(DBNAME);
  sqlite3_open(DBNAME, &db);
  if (SQLITE_OK != sqlite3_exec(db, SQLTABLE, NULL, NULL, &errMsg))
  {
    sqlite3_free(errMsg);
    errMsg = NULL;
  }
  sqlite3_close(db);
}

void
DeleteHistory(
  unsigned long historyId
)
{
  sqlite3_bind_int(deleteStmt, 1, historyId);
  sqlite3_step(deleteStmt);
  sqlite3_reset(deleteStmt);
  sqlite3_clear_bindings(deleteStmt);
}

void
ModifyHistory(
  unsigned long historyId,
  char* comment
)
{
  sqlite3_bind_text(updateStmt, 1, comment, strlen(comment), SQLITE_TRANSIENT);
  sqlite3_bind_int(updateStmt, 2, historyId);
  sqlite3_step(updateStmt);
  sqlite3_reset(updateStmt);
  sqlite3_clear_bindings(updateStmt);
}

unsigned long /* play counts */
HasHistory(
  char* path
)
{
  unsigned long count = 0;
  sqlite3_bind_text(hasStmt, 1, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_step(hasStmt);
  count = (unsigned long)sqlite3_column_int64(hasStmt, 0);
  sqlite3_reset(hasStmt);
  sqlite3_clear_bindings(hasStmt);
  return count;
}
