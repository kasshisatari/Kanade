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
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "define.h"
#include "file.h"

#define DBNAME ":memory:"
#define SQLTABLE "CREATE TABLE [FILE] ([FILEID] INTEGER NOT NULL UNIQUE,[PATH] TEXT NOT NULL UNIQUE,[NAME] TEXT NOT NULL,[SIZE] INTEGER NOT NULL,[MODIFYTIME] TEXT NOT NULL,PRIMARY KEY(FILEID));"
#define SQLINSERT "INSERT INTO FILE VALUES(?, ?, ?, ?, ?);"
#define SQLDETAIL "SELECT PATH, MODIFYTIME, SIZE FROM FILE WHERE FILEID = ?;"
#define SQLPATH "SELECT PATH FROM FILE WHERE FILEID = ?;"
#define SQLMAXID "SELECT MAX(FILEID) FROM FILE;"
#define SQLTRANSACTION "BEGIN TRANSACTION;"

static pthread_mutex_t lock;
static sqlite3 *db = NULL;
static sqlite3_stmt *insertStmt = NULL;
static sqlite3_stmt *detailStmt = NULL;
static sqlite3_stmt *maxIdStmt = NULL;
static sqlite3_stmt *pathStmt = NULL;
static unsigned long maxFileId = 0;

void
AddFile(
  char* path /* file path */
)
{
  int ret = 0;
  char timestamp[FILE_LENGTH_MAX];
  unsigned long size = 0;
  char *p = path;
  char *name = path;
  struct stat st = { 0 };
  if (0 != stat(path, &st))
  {
    strcpy(timestamp, "2019/01/01 00:00:00");
  }
  else
  {
    size = st.st_size;
    strftime(timestamp, sizeof(timestamp), "%Y/%m/%d %H:%M:%S", localtime(&(st.st_mtime)));
  }
  maxFileId++;
  sqlite3_bind_int(insertStmt, 1, maxFileId);
  sqlite3_bind_text(insertStmt, 2, path, strlen(path), SQLITE_TRANSIENT);
  while (0x00 != *p)
  {
    if ('/' == *p)
    {
      name = p + 1;
    }
    p++;
  }
  sqlite3_bind_text(insertStmt, 3, name, strlen(name), SQLITE_TRANSIENT);
  sqlite3_bind_int(insertStmt, 4, size);
  sqlite3_bind_text(insertStmt, 5, timestamp, strlen(timestamp), SQLITE_TRANSIENT);
  ret = sqlite3_step(insertStmt);
  if (SQLITE_CONSTRAINT == ret)
  {
    maxFileId--;
  }
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
}

static
char*
SkipSpace(char *str)
{
  while (0x00 != *str && 0x20 == *str)
  {
    /* < SPACE >*/
    str++;
  }
  return str;
}

static
char*
CopyKeyword(char *dst, char *src)
{
  *dst = '%';
  dst++;
  while (0x00 != *src && 0x20 != *src)
  {
    *dst = *src;
    ++dst;
    ++src;
  }
  *dst = '%';
  dst++;
  *dst = 0x00;
  return src;
}

static
char*
CopyString(char *dst, char *src)
{
  while (0x00 != *src)
  {
    *dst = *src;
    ++dst;
    ++src;
  }
  return dst;
}

static
unsigned char
MakeSQLForList(
  char* search,
  char* countSql,
  char* resultSql,
  char keyword[KEYWORD_MAX][FILE_LENGTH_MAX],
  unsigned char sort,
  unsigned long pageNo
)
{
  unsigned char hasKeyword = 0;
  char *p = countSql;
  char *q = resultSql;
  unsigned char keyIndex = 0;
  p = CopyString(p, "SELECT COUNT(FILEID) FROM FILE ");
  q = CopyString(q, "SELECT FILEID, PATH FROM FILE ");
  while (0x00 != *search)
  {
    search = SkipSpace(search);
    if (0x00 == *search)
    {
      break;
    }
    /* < Not SPACE > */
    if (0 == hasKeyword)
    {
      p = CopyString(p, "WHERE PATH LIKE ? ");
      q = CopyString(q, "WHERE PATH LIKE ? ");
      hasKeyword = 1;
    }
    else
    {
      p = CopyString(p, "AND PATH LIKE ? ");
      q = CopyString(q, "AND PATH LIKE ? ");
    }
    search = CopyKeyword((char*)&(keyword[keyIndex]), search);
    keyIndex++;
  }
  q = CopyString(q, "ORDER BY ");
  if (0 == sort)
  {
    q = CopyString(q, "NAME ASC LIMIT ");
  }
  else
  {
    q = CopyString(q, "MODIFYTIME DESC LIMIT ");
  }
  sprintf(q, "%d OFFSET %d", PAGERECORDS, PAGERECORDS * (pageNo - 1));
  return keyIndex;
}

static
unsigned short
CopyPath(
  char *dst,
  const unsigned char *src
)
{
  short delimiter = -1;
  unsigned short count = 0;
  while (0x00 != *src)
  {
    *dst = *src;
    if ('/' == *src)
    {
      delimiter = count;
    }
    dst++;
    src++;
    count++;
  }
  *dst = 0x00;
  return delimiter + 1;
}

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
GetFileList(
  char* keyword, /* search keyword */
  unsigned long pageNo, /* page No. */
  unsigned char sort, /* sort kind */
  unsigned long* fileIdList, /* file id list */
  char* filePathList, /* file path list */
  unsigned short *offsetList, /* file name offset list */
  unsigned char* recordsInPage, /* records in page */
  unsigned long* pageNumber
)
{
  int keyIndex = 0;
  char countSql[PATH_LENGTH_MAX] = { 0 };
  char resultSql[PATH_LENGTH_MAX] = { 0 };
  char keywords[KEYWORD_MAX][FILE_LENGTH_MAX];
  sqlite3_stmt *countStmt = NULL;
  sqlite3_stmt *resultStmt = NULL;
  int ret = 0;
  unsigned char recIndex = 0;
  unsigned long records = 0;
  *pageNumber = 0;
  keyIndex = MakeSQLForList(keyword, countSql, resultSql, keywords, sort, pageNo);
  sqlite3_prepare_v2(db, countSql, -1, &countStmt, NULL);
  sqlite3_prepare_v2(db, resultSql, -1, &resultStmt, NULL);
  for (int i = 0; i < keyIndex; ++i)
  {
    sqlite3_bind_text(countStmt, i + 1, keywords[i], strlen(keywords[i]), SQLITE_TRANSIENT);
    sqlite3_bind_text(resultStmt, i + 1, keywords[i], strlen(keywords[i]), SQLITE_TRANSIENT);
  }
  ret = sqlite3_step(countStmt);
  if (SQLITE_ROW == ret)
  {
    records = sqlite3_column_int(countStmt, 0);
  }
  if (0 == records)
  {
    *recordsInPage = 0;
    *pageNumber = 0;
  }
  else
  {
    *pageNumber = records / PAGERECORDS;
    if (0 != records % PAGERECORDS)
    {
      *pageNumber = *pageNumber + 1;
    }
    ret = sqlite3_step(resultStmt);
    while (SQLITE_ROW == ret)
    {
      fileIdList[recIndex] = sqlite3_column_int(resultStmt, 0);
      const unsigned char *p = sqlite3_column_text(resultStmt, 1);
      offsetList[recIndex] = CopyPath(&(filePathList[recIndex * PATH_LENGTH_MAX]), p);
      recIndex++;
      ret = sqlite3_step(resultStmt);
    }
  }
  *recordsInPage = recIndex;
  sqlite3_reset(countStmt);
  sqlite3_reset(resultStmt);
  sqlite3_clear_bindings(countStmt);
  sqlite3_clear_bindings(resultStmt);
  sqlite3_finalize(countStmt);
  sqlite3_finalize(resultStmt);
  return records;
}

void
GetFilePath(
  unsigned long fileId,
  char* path
)
{
  const unsigned char *p = NULL;
  int ret = 0;
  sqlite3_bind_int(pathStmt, 1, fileId);
  ret = sqlite3_step(pathStmt);
  if (SQLITE_ROW == ret)
  {
    p = sqlite3_column_text(pathStmt, 0);
    strcpy(path, (const char*)p);
  }
  else
  {
    *path = 0;
  }
  sqlite3_reset(pathStmt);
  sqlite3_clear_bindings(pathStmt);
}

long /* file size */
GetFileDetail(
  unsigned long fileId, /* file id */
  char* path, /* file path */
  char* modifyTime /* modify time */
)
{
  const unsigned char *p = NULL;
  int ret = 0;
  long size = -1;
  sqlite3_bind_int(detailStmt, 1, fileId);
  ret = sqlite3_step(detailStmt);
  if (SQLITE_ROW == ret)
  {
    p = sqlite3_column_text(detailStmt, 0);
    strcpy(path, (const char*)p);
    p = sqlite3_column_text(detailStmt, 1);
    strcpy(modifyTime, (const char*)p);
  size = sqlite3_column_int(detailStmt, 2);
  }
  else
  {
    sqlite3_reset(detailStmt);
    sqlite3_clear_bindings(detailStmt);
    return -1;
  }
  sqlite3_reset(detailStmt);
  sqlite3_clear_bindings(detailStmt);
  return size;
}

void
InitFile(
  void
)
{
  pthread_mutex_init(&lock, 0);
}

void
OpenFile(
  void
)
{
  char *errMsg = NULL;
  int ret = 0;
  system("rm -fr t/*");
  sqlite3_open(DBNAME, &db);
  if (SQLITE_OK != sqlite3_exec(db, SQLTABLE, NULL, NULL, &errMsg))
  {
    sqlite3_free(errMsg);
    errMsg = NULL;
  }
  sqlite3_prepare_v2(db, SQLINSERT, -1, &insertStmt, NULL);
  sqlite3_prepare_v2(db, SQLDETAIL, -1, &detailStmt, NULL);
  sqlite3_prepare_v2(db, SQLMAXID, -1, &maxIdStmt, NULL);
  sqlite3_prepare_v2(db, SQLPATH, -1, &pathStmt, NULL);
  ret = sqlite3_step(maxIdStmt);
  if (SQLITE_ROW == ret)
  {
    maxFileId = sqlite3_column_int(maxIdStmt, 0);
  }
  else
  {
    maxFileId = 0;
  }
  sqlite3_reset(maxIdStmt);
  sqlite3_finalize(maxIdStmt);
}

void
CloseFile(
  void
)
{
  sqlite3_finalize(insertStmt);
  sqlite3_finalize(detailStmt);
  sqlite3_finalize(pathStmt);
  sqlite3_close(db);
}

void
ResetFile(
  void
)
{
  CloseFile();
}

void
LockFile(
  void
)
{
  pthread_mutex_lock(&lock);
}

void
UnlockFile(
  void
)
{
  pthread_mutex_unlock(&lock);
}

unsigned long
GetFileCount(
  void
)
{
  return maxFileId;
}
