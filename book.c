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
#include "book.h"

#define DBNAME ":memory:"
#define SQLTABLE "CREATE TABLE [BOOK] ([BOOKID] INTEGER NOT NULL UNIQUE,[USERID] INTEGER,[PATH] TEXT,[COMMENT] TEXT,[DURATION] INTEGER,[AUDIOTRACK] INTEGER,[POSITION] INTEGER NOT NULL UNIQUE,[PUBLIC] INTEGER NOT NULL,[VALID] INTEGER NOT NULL,PRIMARY KEY(BOOKID));"
#define SQLINSERT "INSERT INTO BOOK VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);"
#define SQLDELETE "DELETE FROM BOOK WHERE BOOKID = ?;"
#define SQLUPDATE "UPDATE BOOK SET PATH = ?, COMMENT = ?, DURATION = ?, AUDIOTRACK = ?, PUBLIC = ? WHERE BOOKID = ?;"
#define SQLUPCHECK "SELECT MIN(BOOK.POSITION), BOOKID, ORG.POSITION  FROM BOOK,  (SELECT POSITION FROM BOOK WHERE BOOKID = ?) as ORG WHERE ORG.POSITION < BOOK.POSITION;"
#define SQLDOWNCHECK "SELECT MAX(BOOK.POSITION), BOOKID, ORG.POSITION  FROM BOOK,  (SELECT POSITION FROM BOOK WHERE BOOKID = ?) as ORG WHERE ORG.POSITION > BOOK.POSITION;"
#define SQLMOVE "UPDATE BOOK SET POSITION = ? WHERE BOOKID = ?;"
#define SQLLIST "SELECT BOOKID, USERID, PATH, COMMENT, DURATION, PUBLIC, VALID FROM BOOK ORDER BY POSITION ASC"
#define SQLHAS "SELECT COUNT(BOOKID) FROM BOOK WHERE PATH = ?;"
#define SQLOPEN "SELECT MAX(BOOKID), MAX(POSITION), MIN(POSITION), COUNT(BOOKID) FROM BOOK;"
#define SQLCHANGE "SELECT CHANGES();"
#define SQLPATH "SELECT PATH FROM BOOK WHERE BOOKID = ?;"
#define SQLAUDIO "SELECT AUDIOTRACK FROM BOOK WHERE BOOKID = ?;"

static pthread_mutex_t lock;
static pthread_mutex_t latest;
static sqlite3 *db = NULL;
static sqlite3_stmt *insertStmt = NULL;
static sqlite3_stmt *deleteStmt = NULL;
static sqlite3_stmt *updateStmt = NULL;
static sqlite3_stmt *moveUpCheckStmt = NULL;
static sqlite3_stmt *moveDownCheckStmt = NULL;
static sqlite3_stmt *moveStmt = NULL;
static sqlite3_stmt *listStmt = NULL;
static sqlite3_stmt *hasStmt = NULL;
static sqlite3_stmt *openStmt = NULL;
static sqlite3_stmt *changeStmt = NULL;
static sqlite3_stmt *pathStmt = NULL;
static sqlite3_stmt *audioStmt = NULL;
static unsigned long maxBookId = 0;
static long minPos = 0;
static long maxPos = 0;
static unsigned short count = 0;
unsigned long latestBookId = 0;
char latestFile[PATH_LENGTH_MAX];
char latestComment[COMMENT_LENGTH_MAX];
unsigned short latestUserId = 0;
unsigned char latestVisible = 0;

unsigned char /* records */
GetBookList(
  unsigned long* bookIdList,
  unsigned short* userIdList,
  char* filePathList,
  char* commentList,
  unsigned long* durationList
)
{
  int ret = sqlite3_step(listStmt);
  int recIndex = 0;
  int valid = 0;
  int visible = 0;
  const unsigned char *p = NULL;
  while (SQLITE_ROW == ret)
  {
    bookIdList[recIndex] = sqlite3_column_int(listStmt, 0);
    valid = sqlite3_column_int(listStmt, 6);
    if (1 == valid)
    {
      userIdList[recIndex] = (unsigned short)sqlite3_column_int(listStmt, 1);
      visible = sqlite3_column_int(listStmt, 5);
      if (1 == visible)
      {
        p = sqlite3_column_text(listStmt, 2);
        strcpy((filePathList + PATH_LENGTH_MAX * recIndex), (const char*)p);
      }
      else
      {
        strcpy((filePathList + PATH_LENGTH_MAX * recIndex), "Secret");
      }
      p = sqlite3_column_text(listStmt, 3);
      strcpy((commentList + COMMENT_LENGTH_MAX * recIndex), (const char*)p);
      durationList[recIndex] = sqlite3_column_int(listStmt, 4);
    }
    else
    {
      userIdList[recIndex] = 0;
      strcpy((filePathList + PATH_LENGTH_MAX * recIndex), "Break");
      *(commentList + COMMENT_LENGTH_MAX * recIndex) = 0x00;
      durationList[recIndex] = 0;
    }
    recIndex++;
    ret = sqlite3_step(listStmt);
  }
  sqlite3_reset(listStmt);
  return (unsigned char)recIndex;
}

unsigned long /* Book ID */
AddBook(
  unsigned short userId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
)
{
  if (count >= PAGERECORDS)
  {
    return 0;
  }
  count++;
  maxBookId++;
  maxPos++;
  sqlite3_bind_int(insertStmt, 1, maxBookId);
  sqlite3_bind_int(insertStmt, 2, userId);
  sqlite3_bind_text(insertStmt, 3, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 4, comment, strlen(comment), SQLITE_TRANSIENT);
  sqlite3_bind_int(insertStmt, 5, duration);
  sqlite3_bind_int(insertStmt, 6, audioTrack);
  sqlite3_bind_int(insertStmt, 7, maxPos);
  sqlite3_bind_int(insertStmt, 8, visible);
  sqlite3_bind_int(insertStmt, 9, 1);
  sqlite3_step(insertStmt);
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);

  pthread_mutex_lock(&latest);
  latestBookId = maxBookId;
  latestUserId = userId;
  strcpy(latestComment, comment);
  strcpy(latestFile, path);
  latestVisible = visible;
  pthread_mutex_unlock(&latest);

  return maxBookId;
}

unsigned long /* Book ID */
InsertBook(
  unsigned short userId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
)
{
  if (count >= PAGERECORDS)
  {
    return 0;
  }
  count++;
  maxBookId++;
  minPos--;
  sqlite3_bind_int(insertStmt, 1, maxBookId);
  sqlite3_bind_int(insertStmt, 2, userId);
  sqlite3_bind_text(insertStmt, 3, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 4, comment, strlen(comment), SQLITE_TRANSIENT);
  sqlite3_bind_int(insertStmt, 5, duration);
  sqlite3_bind_int(insertStmt, 6, audioTrack);
  sqlite3_bind_int(insertStmt, 7, minPos);
  sqlite3_bind_int(insertStmt, 8, visible);
  sqlite3_bind_int(insertStmt, 9, 1);
  sqlite3_step(insertStmt);
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);

  pthread_mutex_lock(&latest);
  latestBookId = maxBookId;
  latestUserId = userId;
  strcpy(latestComment, comment);
  strcpy(latestFile, path);
  latestVisible = visible;
  pthread_mutex_unlock(&latest);

  return maxBookId;
}

unsigned long /* Book ID */
AddBreak(
  void
)
{
  if (count >= PAGERECORDS)
  {
    return 0;
  }
  count++;
  maxBookId++;
  maxPos++;
  sqlite3_bind_int(insertStmt, 1, maxBookId);
  sqlite3_bind_int(insertStmt, 2, 0);
  sqlite3_bind_text(insertStmt, 3, "", 0, SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 4, "", 0, SQLITE_TRANSIENT);
  sqlite3_bind_int(insertStmt, 5, 0);
  sqlite3_bind_int(insertStmt, 6, 0);
  sqlite3_bind_int(insertStmt, 7, maxPos);
  sqlite3_bind_int(insertStmt, 8, 1);
  sqlite3_bind_int(insertStmt, 9, 0);
  sqlite3_step(insertStmt);
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
  return maxBookId;
}

unsigned long /* Book ID */
InsertBreak(
  void
)
{
  if (count >= PAGERECORDS)
  {
    return 0;
  }
  count++;
  maxBookId++;
  minPos--;
  sqlite3_bind_int(insertStmt, 1, maxBookId);
  sqlite3_bind_int(insertStmt, 2, 0);
  sqlite3_bind_text(insertStmt, 3, "", 0, SQLITE_TRANSIENT);
  sqlite3_bind_text(insertStmt, 4, "", 0, SQLITE_TRANSIENT);
  sqlite3_bind_int(insertStmt, 5, 0);
  sqlite3_bind_int(insertStmt, 6, 0);
  sqlite3_bind_int(insertStmt, 7, minPos);
  sqlite3_bind_int(insertStmt, 8, 1);
  sqlite3_bind_int(insertStmt, 9, 0);
  sqlite3_step(insertStmt);
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
  return maxBookId;
}

unsigned long
DeleteBook(
  unsigned long bookId
)
{
  int ret = 0;
  unsigned char changed = 0;
  sqlite3_bind_int(deleteStmt, 1, bookId);
  sqlite3_step(deleteStmt);
  sqlite3_reset(deleteStmt);
  sqlite3_clear_bindings(deleteStmt);
  ret = sqlite3_step(openStmt);
  if (SQLITE_ROW == ret)
  {
    count = (unsigned char)sqlite3_column_int(openStmt, 3);
  }
  else
  {
    count = 0;
  }
  sqlite3_reset(openStmt);
  sqlite3_step(changeStmt);
  changed = (unsigned char)sqlite3_column_int(changeStmt, 0);
  sqlite3_reset(changeStmt);
  if (1 == changed)
  {
    return bookId;
  }
  else
  {
    return 0;
  }
}

unsigned long /* Book ID */
MoveBook(
  unsigned long bookId,
  unsigned char dir
)
{
  long srcPos = 0;
  long dstPos = 0;
  unsigned long dstId = 0;
  if (0 == dir)
  {
    // < Up >
    sqlite3_bind_int(moveUpCheckStmt, 1, bookId);
    sqlite3_step(moveUpCheckStmt);
    dstPos = sqlite3_column_int(moveUpCheckStmt, 0);
    dstId = sqlite3_column_int(moveUpCheckStmt, 1);
    srcPos = sqlite3_column_int(moveUpCheckStmt, 2);
    sqlite3_reset(moveUpCheckStmt);
    sqlite3_clear_bindings(moveUpCheckStmt);
    if (0 == dstId)
    {
      // < Not Found >
      return 0;
    }
  }
  else
  {
    // < Down >
    sqlite3_bind_int(moveDownCheckStmt, 1, bookId);
    sqlite3_step(moveDownCheckStmt);
    dstPos = sqlite3_column_int(moveDownCheckStmt, 0);
    dstId = sqlite3_column_int(moveDownCheckStmt, 1);
    srcPos = sqlite3_column_int(moveDownCheckStmt, 2);
    sqlite3_reset(moveDownCheckStmt);
    sqlite3_clear_bindings(moveDownCheckStmt);
    if (0 == dstId)
    {
      // < Not Found >
      return 0;
    }
  }
  sqlite3_bind_int(moveStmt, 1, 0);
  sqlite3_bind_int(moveStmt, 2, bookId);
  sqlite3_step(moveStmt);
  sqlite3_reset(moveStmt);
  sqlite3_clear_bindings(moveStmt);

  sqlite3_bind_int(moveStmt, 1, srcPos);
  sqlite3_bind_int(moveStmt, 2, dstId);
  sqlite3_step(moveStmt);
  sqlite3_reset(moveStmt);
  sqlite3_clear_bindings(moveStmt);

  sqlite3_bind_int(moveStmt, 1, dstPos);
  sqlite3_bind_int(moveStmt, 2, bookId);
  sqlite3_step(moveStmt);
  sqlite3_reset(moveStmt);
  sqlite3_clear_bindings(moveStmt);

  return bookId;
}

unsigned char /* Exist */
HasBook(
  char* path
)
{
  unsigned char ret = 0;
  sqlite3_bind_text(hasStmt, 1, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_step(hasStmt);
  ret = (unsigned char)sqlite3_column_int(hasStmt, 0);
  sqlite3_reset(hasStmt);
  sqlite3_clear_bindings(hasStmt);
  return ret;
}

unsigned long /* Book ID */
ModifyBook(
  unsigned long bookId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
)
{
  unsigned char changed = 0;
  sqlite3_bind_text(updateStmt, 1, path, strlen(path), SQLITE_TRANSIENT);
  sqlite3_bind_text(updateStmt, 2, comment, strlen(comment), SQLITE_TRANSIENT);
  sqlite3_bind_int(updateStmt, 3, duration);
  sqlite3_bind_int(updateStmt, 4, audioTrack);
  sqlite3_bind_int(updateStmt, 5, visible);
  sqlite3_bind_int(updateStmt, 6, bookId);
  sqlite3_step(updateStmt);
  sqlite3_reset(updateStmt);
  sqlite3_clear_bindings(updateStmt);
  sqlite3_step(changeStmt);
  changed = (unsigned char)sqlite3_column_int(changeStmt, 0);
  sqlite3_reset(changeStmt);
  if (1 == changed)
  {
    return bookId;
  }
  else
  {
    return 0;
  }
}

void
OpenBook(
	void
)
{
  char *errMsg = NULL;
  int ret = 0;
  pthread_mutex_init(&lock, 0);
  pthread_mutex_init(&latest, 0);
  sqlite3_open(DBNAME, &db);
  if (SQLITE_OK != sqlite3_exec(db, SQLTABLE, NULL, NULL, &errMsg))
  {
    sqlite3_free(errMsg);
    errMsg = NULL;
  }
  sqlite3_prepare_v2(db, SQLINSERT, -1, &insertStmt, NULL);
  sqlite3_prepare_v2(db, SQLDELETE, -1, &deleteStmt, NULL);
  sqlite3_prepare_v2(db, SQLUPDATE, -1, &updateStmt, NULL);
  sqlite3_prepare_v2(db, SQLMOVE, -1, &moveStmt, NULL);
  sqlite3_prepare_v2(db, SQLUPCHECK, -1, &moveUpCheckStmt, NULL);
  sqlite3_prepare_v2(db, SQLDOWNCHECK, -1, &moveDownCheckStmt, NULL);
  sqlite3_prepare_v2(db, SQLLIST, -1, &listStmt, NULL);
  sqlite3_prepare_v2(db, SQLHAS, -1, &hasStmt, NULL);
  sqlite3_prepare_v2(db, SQLOPEN, -1, &openStmt, NULL);
  sqlite3_prepare_v2(db, SQLCHANGE, -1, &changeStmt, NULL);
  sqlite3_prepare_v2(db, SQLPATH, -1, &pathStmt, NULL);
  sqlite3_prepare_v2(db, SQLAUDIO, -1, &audioStmt, NULL);
  ret = sqlite3_step(openStmt);
  if (SQLITE_ROW == ret)
  {
    maxBookId = (unsigned long)sqlite3_column_int(openStmt, 0);
    maxPos = sqlite3_column_int(openStmt, 1);
    if (maxPos < 0)
    {
      maxPos = 0;
    }
    minPos = sqlite3_column_int(openStmt, 2);
    if (minPos > 0)
    {
      minPos = 0;
    }
    count = (unsigned char)sqlite3_column_int(openStmt, 3);
  }
  else
  {
    maxBookId = 0;
    maxPos = 0;
    minPos = 0;
    count = 0;
  }
  sqlite3_reset(openStmt);
}

void
CloseBook(
	void
)
{
  sqlite3_finalize(insertStmt);
  sqlite3_finalize(deleteStmt);
  sqlite3_finalize(updateStmt);
  sqlite3_finalize(moveStmt);
  sqlite3_finalize(moveUpCheckStmt);
  sqlite3_finalize(moveDownCheckStmt);
  sqlite3_finalize(listStmt);
  sqlite3_finalize(hasStmt);
  sqlite3_finalize(openStmt);
  sqlite3_finalize(changeStmt);
  sqlite3_finalize(pathStmt);
  sqlite3_finalize(audioStmt);
  sqlite3_close(db);
  pthread_mutex_destroy(&lock);
}

void
ResetBook(
	void
)
{
  CloseBook();
}

void
LockBook(
	void
)
{
  pthread_mutex_lock(&lock);
}

void
UnlockBook(
	void
)
{
  pthread_mutex_unlock(&lock);
  pthread_mutex_unlock(&latest);
}

void
GetBookPath(
  unsigned long bookId,
  char* path
)
{
  const unsigned char *p = NULL;
  sqlite3_bind_int(pathStmt, 1, bookId);
  sqlite3_step(pathStmt);
  p = sqlite3_column_text(pathStmt, 0);
  strcpy(path, (const char*)p);
  sqlite3_reset(pathStmt);
  sqlite3_clear_bindings(pathStmt);
}

unsigned char
GetBookAudio(
  unsigned long bookId
)
{
  unsigned char audio = 0;
  sqlite3_bind_int(audioStmt, 1, bookId);
  sqlite3_step(audioStmt);
  audio = sqlite3_column_int(audioStmt, 0);
  sqlite3_reset(audioStmt);
  sqlite3_clear_bindings(audioStmt);
  return audio;
}

void
GetLatestBook(
  unsigned long *bookId,
  unsigned short *userId,
  char *file,
  char *comment,
  unsigned char *visible
)
{
  pthread_mutex_lock(&latest);
  *bookId = latestBookId;
  *userId = latestUserId;
  strcpy(comment, latestComment);
  strcpy(file, latestFile);
  *visible = latestVisible;
  pthread_mutex_unlock(&latest);
}
