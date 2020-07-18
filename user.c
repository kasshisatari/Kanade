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
#include "user.h"

/* DB File Name */
#define DBNAME "user.db"
/* Table Schema */
#define SQLTABLE "CREATE TABLE [USER] ( [USERID] INTEGER NOT NULL UNIQUE, [USERNAME] TEXT NOT NULL UNIQUE, PRIMARY KEY(USERID));"
/* Insert SQL Statement */
#define SQLINSERT "INSERT INTO USER VALUES(?, ?);"
/* Get USERID SQL Statement */
#define SQLUSERID "SELECT USERID FROM USER WHERE ? = USERNAME;"
/* Get USERNAME SQL Statement */
#define SQLUSERNAME "SELECT USERNAME FROM USER WHERE ? = USERID;"
/* Get Max USERID SQL Statement */
#define SQLMAXUSERID "SELECT MAX(USERID) FROM USER;"

/* SQLite3 DB */
static sqlite3 *db = NULL;
/* Insert SQL Statement */
static sqlite3_stmt *insertStmt = NULL;
/* Get USERID SQL Statement */
static sqlite3_stmt *userIdStmt = NULL;
/* Get USERNAME SQL Statement */
static sqlite3_stmt *nameStmt = NULL;
/* Get Max USERID SQL Statement */
static sqlite3_stmt *maxUserIdStmt = NULL;
/* Max USERID */
static unsigned short maxUserId = 0;
/* Mutex */
static pthread_mutex_t lock;

/* Brief: Create User Account */
/* Pre-Condition: Callled OpenUser and LockUser Functions */
unsigned short /* User-ID */
CreateUser(
  char* userName /* User-Name */
)
{
  /* [[[ 0. Var ]]] */
  /* Return Value for Sub-Functions */
  int ret = 0;

  /* [[[ 1. Insert User-Name ]]] */
  /* [[ 1.1. Update Max USERID ]] */
  maxUserId++;
  /* [[ 1.2. Inser User-Name to DB ]] */
  ret = sqlite3_bind_int(insertStmt, 1, maxUserId);
  ret = sqlite3_bind_text(insertStmt, 2, userName, (int)strlen(userName), SQLITE_TRANSIENT);
  ret = sqlite3_step(insertStmt);
  
  /* [[[ 2. Check Already Exist User-Name ]]] */
  if (SQLITE_CONSTRAINT == ret)
  {
    /* < Already Exist User-Name > */
    /* [[ 2.1. Get User-ID for Already Exist User-Name ]] */
    /* [ 2.1.1. Update Max USERID ] */
    maxUserId--;
    /* [ 2.1.2. Get User-ID for Already Exist User-Name from DB ] */
    sqlite3_bind_text(userIdStmt, 1, userName, (int)strlen(userName), SQLITE_TRANSIENT);
    sqlite3_step(userIdStmt);
    ret = sqlite3_column_int(userIdStmt, 0);
    /* [[ 2.2. Reset SQL Statements ]] */
    /* [ 2.2.1. Insert SQL Statement ]*/
    sqlite3_reset(insertStmt);
    sqlite3_clear_bindings(insertStmt);
    /* [ 2.2.2. Get USERID SQL Statement ] */
    sqlite3_reset(userIdStmt);
    sqlite3_clear_bindings(userIdStmt);
    /* [ 2.2.3. Return USERID ] */
    return (unsigned short)ret;
  }

  /* [[[ 3. Reset SQL Statements ]]] */
  /* [[ 3.1. Insert SQL Statement ]] */
  sqlite3_reset(insertStmt);
  sqlite3_clear_bindings(insertStmt);
  /* [[ 3.2. Return USER ID ]] */
  return maxUserId;
}

/* Brief: Get User-Name */
/* Pre-Condition: Callled OpenUser and LockUser Functions */
unsigned short /* Result: 0 is Error, 1 is OK */
GetUserName(
  unsigned short userId, /* User-ID */
  char* userName /* User-Name */
)
{
  /* [[[ 0. Var ]]] */
  /* Return Value for Sub-Functions */
  int ret = 0;
  /* User-Name String */
  const unsigned char *userNameStr = NULL;

  /* [[[ 1. Get User-Name ]]] */
  sqlite3_bind_int(nameStmt, 1, userId);
  ret = sqlite3_step(nameStmt);
  if (SQLITE_ROW == ret)
  {
    /* < Fount > */
    /* [[ 1.1. Copy User-Name String ]] */
    userNameStr = sqlite3_column_text(nameStmt, 0);
    strcpy(userName, (const char*)userNameStr);
  }
  else
  {
    /* < Not Found > */
    /* [[ 1.2. Reset SQL Statements ]] */
    /* [ 1.2.1. Get USERNAME SQL Statement ]*/
    sqlite3_reset(nameStmt);
    sqlite3_clear_bindings(nameStmt);
    /* [ 1.2.2. Return Error ] */
    return 0;
  }
  /* [[[ 2. Reset SQL Statements ]]] */
  /* [[ 2.1. Get USERNAME SQL Statement ]] */
  sqlite3_reset(nameStmt);
  sqlite3_clear_bindings(nameStmt);
  /* [[ 2.2. Return OK ]] */
  return 1;
}

/* Brief: Delete DB */
/* Pre-Condition: Callled OpenUser and LockUser Functions */
void
ResetUser(
  void
)
{
  /* [[[ 0. Var ]]] */
  /* Error Message fo CREATE TABLE SQL Statement  */
  char *errMsg = NULL;

  /* [[[ 1. Close DB ]]] */
  CloseUser();

  /* [[[ 2. Delete DB File ]]] */
  remove(DBNAME);

  /* [[[ 3. Initialize DB File ]]] */
  /* [[ 3.1. Open DB File ]] */
  sqlite3_open(DBNAME, &db);
  /* [[ 3.2. Create Table ]]*/
  if (SQLITE_OK != sqlite3_exec(db, SQLTABLE, NULL, NULL, &errMsg))
  {
    sqlite3_free(errMsg);
    errMsg = NULL;
  }

  /* [[[ 4. Close DB File ]]] */
  sqlite3_close(db);
}

/* Brief: Lock User DB */
/* Pre-Condition: Callled OpenUser Function */
void LockUser(
  void
)
{
  /* [[[ 1. Lock ]]] */
  pthread_mutex_lock(&lock);
}

/* Brief: Unlock User DB */
/* Pre-Condition: Callled OpenUser Function */
void UnlockUser(
  void
)
{
  /* [[[ 1. Unlock ]]] */
  pthread_mutex_unlock(&lock);
}

/* Brief: Open User DB */
void OpenUser(
  void
)
{
  /* [[[ 0. Var ]]] */
  int ret = 0;
  char *errMsg = NULL;

  /* [[[ 1. Initialize Mutex ]]] */
  pthread_mutex_init(&lock, 0);

  /* [[[ 2. Check DB File Exist ]]] */
  FILE *file = fopen(DBNAME, "r");
  if (NULL == file)
  {
    /* < Not Found > */
    /* [[ 2.1. Create DB File ]] */
    ResetUser();
  }
  else
  {
    /* < Found > */
    fclose(file);
  }

  /* [[[ 3. Open DB File ]]] */
  sqlite3_open(DBNAME, &db);

  /* [[[ 4. Prepare SQL Statements ]]] */
  sqlite3_prepare_v2(db, SQLINSERT, -1, &insertStmt, NULL);
  sqlite3_prepare_v2(db, SQLUSERID, -1, &userIdStmt, NULL);
  sqlite3_prepare_v2(db, SQLUSERNAME, -1, &nameStmt, NULL);
  sqlite3_prepare_v2(db, SQLMAXUSERID, -1, &maxUserIdStmt, NULL);

  /* [[[ 5. Get Max User-ID ]]] */
  /* [[ 5.1. Check Any User Accounts ]] */
  ret = sqlite3_step(maxUserIdStmt);
  if (SQLITE_ROW == ret)
  {
    /* < Found > */
    /* [[ 5.1. Get Max User-ID from DB ]] */
    maxUserId = (unsigned short)sqlite3_column_int(maxUserIdStmt, 0);
  }
  else
  {
    /* < Not Found > */
    /* [[ 5.2. Update Max User-ID ]]*/
    maxUserId = 0;
  }

  /* [[[ 6. Finalize SQL Statements ]]] */
  /* [[ 6.1. Get Max USERID SQL Statement ]] */
  sqlite3_reset(maxUserIdStmt);
  sqlite3_finalize(maxUserIdStmt);
}

/* Brief: Close User DB */
/* Pre-Condition: Callled OpenUser Function */
void CloseUser(
  void
)
{
  /* [[[ 1. Finalize SQL Statements ]]] */
  /* [[ 1.1. Insert SQL Statement ]] */
  sqlite3_finalize(insertStmt);
  /* [[ 1.2. Get USERID SQL Statement ]] */
  sqlite3_finalize(userIdStmt);
  /* [[ 1.3. Get USERNAME SQL Statement ]] */
  sqlite3_finalize(nameStmt);

  /* [[[ 2. Close DB ]]] */
  sqlite3_close(db);

  /* [[[ 3. Destroy Mutex ]]] */
  pthread_mutex_destroy(&lock);
}
