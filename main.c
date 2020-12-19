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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <memory.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/input.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <gtk/gtk.h>
#include "define.h"
#include "user.h"
#include "file.h"
#include "book.h"
#include "history.h"
#include "favorite.h"
#include "player.h"

#define NOT_FOUND_MESSAGE "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n"
#define OK_HEADER "HTTP/1.1 200 OK\r\n"
#define PARTIAL_HEADER "HTTP/1.1 206 OK\r\n"
#define CSV_HEADER "Content-Type: application/octet-stream\r\n"
#define HTML_HEADER "Content-Type: text/html; charset=UTF-8\r\n"
#define CSS_HEADER "Content-Type: text/css; charset=UTF-8\r\n"
#define JS_HEADER "Content-Type: text/javascript; charset=UTF-8\r\n"
#define AVI_HEADER "Content-Type: video/avi\r\n"
#define MP4_HEADER "Content-Type: video/mp4\r\n"
#define MKV_HEADER "Content-Type: video/x-matroska\r\n"
#define FLV_HEADER "Content-Type: video/x-flv\r\n"
#define CONTENT_RANGE_HEADER "Content-Range: bytes %d-"

unsigned char playerState = 0;
long long playTime;
unsigned long playDuration;
unsigned char audioOutput = 0;
unsigned char playMode = 0;
GtkWidget *userLabel = NULL;
GtkWidget *songLabel = NULL;
GtkWidget *commentLabel = NULL;
GtkWidget *pauseLabel = NULL;

void
CopyFileName(
  unsigned char* dst,
  unsigned char* src,
  unsigned char* mediaType
)
{
  unsigned char *ext = NULL;
  unsigned long fileId = 0;
  unsigned char *head = dst;
  while(' ' != *src)
  {
    if ('.' == *src)
    {
      ext = src + 1;
    }
    *dst = *src;
    fileId = fileId * 10 - '0' + *src;
    src++;
    dst++;
  }
  if (NULL == ext)
  {
    LockFile();
    GetFilePath(fileId, head);
    UnlockFile();
    src = head;
    while (0 != *src)
    {
      if ('.' == *src)
      {
        ext = src + 1;
      }
      src++;
    }
    if ('m' == ext[0])
    {
      if ('p' == ext[1])
      {
        *mediaType = 4; // MP4
      }
      else if ('k' == ext[1])
      {
        *mediaType = 5; // MKV
      }
    }
    else if ('a' == ext[0])
    {
      *mediaType = 3; // AVI
    }
    else if ('f' == ext[0])
    {
      *mediaType = 6; // FLV
    }
    return;
  }
  else if ('c' == ext[0] &&
      's' == ext[1] &&
      's' == ext[2])
  {
    *mediaType = 1; // CSS
  }
  else if('j' == ext[0] &&
          's' == ext[1])
  {
    *mediaType = 2; // JS
  }
  *dst = 0;
}

void
SendText(
  int conn,
  unsigned char *str,
  int size
)
{
  int sendSize = -1;
  while(-1 == sendSize)
  {
    sendSize = send(conn, str, size, 0);
    if (EPIPE == errno || ECONNRESET == errno)
    {
      close(conn);
      pthread_exit(NULL);
    }
    else if (2 == errno)
    {
    }
    else if (0 != errno)
    {
      fprintf(stderr, "SendText send error %d.\n", errno);
      fprintf(stderr, "%s\n", str);
    }
  }
  int offset = 0;
  while (sendSize != size)
  {
    size -= sendSize;
    offset += sendSize;
    sendSize = -1;
    while(-1 == sendSize)
    {
      sendSize = send(conn, &(str[offset]), size, 0);
      if (EPIPE == errno || ECONNRESET == errno)
      {
        close(conn);
        pthread_exit(NULL);
      }
      else if (2 == errno)
      {
      }
      else if (0 != errno)
      {
        fprintf(stderr, "SendText send error %d.\n", errno);
      }
    }
  }
}

unsigned long
CheckHeader(
  unsigned char* in,
  ssize_t size
)
{
  int status = 0;
  unsigned long count = 0;
  for(int i=0;i<size;++i)
  {
    if (0 == status && '\r' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (1 == status && '\n' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (2 == status && '\r' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (3 == status && '\n' == *in)
    {
      status++;
      in++;
      count++;
      return count;
    }
    status = 0;
    in++;
    count++;
  }
  return 0;
}

unsigned char*
SkipMessage(
  unsigned char* in,
  ssize_t rest
)
{
  int status = 0;
  int count = 0;
  unsigned long long maxRest = rest;
  for(int i=0;i<maxRest;++i)
  {
    if (0 == status && '\r' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (1 == status && '\n' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (2 == status && '\r' == *in)
    {
      status++;
      in++;
      count++;
      continue;
    }
    if (3 == status && '\n' == *in)
    {
      status++;
      in++;
      count++;
      return in;
    }
    status = 0;
    in++;
    count++;
  }
  return in;
}

unsigned char*
ParseHttpRequest(
  unsigned char* in,
  unsigned char* fileName,
  unsigned char* mediaType,
  unsigned long long *begin,
  unsigned long long *end,
  ssize_t rest,
  unsigned char *isRange
)
{
  char *range= in;
  ssize_t rangeRest = rest;
  *begin = 0;
  *end = 0;
  *isRange = 0;
  if (0 == rest)
  {
    return NULL;
  }
  if (' ' == in[5])
  {
    strcpy(fileName,"index.html");
    *mediaType = 0; // HTML
  }
  else
  {
    CopyFileName(fileName, &(in[5]), mediaType);
  }
  rest = rest - 5;
  while (rangeRest > 0)
  {
    if ('R' != *range)
    {
      ++range;
      rangeRest--;
      if (rangeRest <= 0)
      {
        goto end;
      }
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('a' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('n' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('g' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('e' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if (':' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if (' ' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('b' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('y' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('t' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('e' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('s' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    if ('=' != *range)
    {
      continue;
    }
    ++range;
    rangeRest--;
    if (rangeRest <= 0)
    {
      goto end;
    }
    *isRange = 1;
    break;
  }
  // range = strstr(in, "Range: bytes=");
  if (1 == *isRange)
  {
    while ('0' <= *range && *range <= '9')
    {
      *begin = *begin * 10 +  *range - '0';
      range++;
      rangeRest--;
      if (rangeRest <= 0)
      {
        goto end;
      }
    }
    if ('\r' == *range)
    {
      goto end;
    }
    if ('-' == *range)
    {
      range++;
      rangeRest--;
      if (rangeRest <= 0)
      {
        goto end;
      }
    }
    else 
    {
      while ('-' != *range && '\r' != *range)
      {
        range++;
        rangeRest--;
        if (rangeRest <= 0)
        {
          goto end;
        }
      }
      if ('\r' == *range)
      {
        goto end;
      }
      if ('-' == *range)
      {
        range++;
        rangeRest--;
        if (rangeRest <= 0)
        {
          goto end;
        }
      }
    }
    while (*range < '0' || '9' < *range)
    {
      if ('\r' == *range)
      {
        goto end;
      }
      range++;
      rangeRest--;
      if (rangeRest <= 0)
      {
        goto end;
      }
    }
    while ('0' <= *range && *range <= '9')
    {
      *end = *end * 10 +  *range - '0';
      range++;
      rangeRest--;
      if (rangeRest <= 0)
      {
        goto end;
      }
    }
  }
end:
  return SkipMessage(&(in[5]), rest);
}

long long
GetContentSize(
  unsigned char *in,
  ssize_t size
)
{
  int status = 0;
  unsigned long long contentSize = 0;
  if (size > 0 && 'G' == *in)
  {
    return 0;
  }
  if (0 == size)
  {
    return -1;
  }
  for(;;)
  {
    if (('C' == *in && 0 == status) ||
        ('o' == *in && 1 == status) ||
        ('n' == *in && 2 == status) ||
        ('t' == *in && 3 == status) ||
        ('e' == *in && 4 == status) ||
        ('n' == *in && 5 == status) ||
        ('t' == *in && 6 == status) ||
        ('-' == *in && 7 == status) ||
        ('L' == *in && 8 == status) ||
        ('e' == *in && 9 == status))
    {
      status++;
    }
    else if (10 != status)
    {
      status = 0;
    }
    if (10 == status)
    {
      in += 5;
      while(':' != *in)
      {
        in++;
        size--;
      }
      in++;
      size--;
      while(*in < '0' || '9' < *in)
      {
        in++;
        size--;
      }
      while('\r' != *in && 0 < size)
      {
        contentSize = contentSize * 10 + *in - '0';
        in++;
        size--;
      }
      if ('\r' != *in)
      {
        return -1;
      }
      break;
    }
    in++;
    size--;
    if (0 == size)
    {
      return -1;
    }
  }
  return contentSize;
}

void
ResponseConnect(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[256];
  unsigned char fileBuf[256];
  char *result = NULL;
  char pass[256];
  char payload[256];
  char command[256];
  char dateStr[32];
  char *p = dateStr;
  in++;
  size--;
  if (size > 0)
  {
    while (size > 0)
    {
      *p = *in;
      ++p;
      ++in;
      --size;
    }
    *p = 0;
    sprintf(command,"sudo date --set=\"%s\" > /dev/null", dateStr);
    system(command);
  }
  FILE *file = NULL;
  file = fopen("ip.txt", "r");
  fgets(fileBuf, 256, file);
  fclose(file);
  fileBuf[strlen(fileBuf)-1] = 0;
  sprintf(payload, "{\"ip\":\"%s\",", fileBuf);
  sprintf(&(payload[strlen(payload)]), "\"port\":\"50000\",");
  file = fopen("AccessPointSSID.txt", "r");
  fgets(fileBuf, 256, file);
  fileBuf[strlen(fileBuf)-1] = 0;
  fclose(file);
  file = fopen("AccessPointPass.txt", "r");
  result = fgets(pass, 256, file);
  if (NULL == result)
  {
    *pass = 0;
  }
  else
  {
    pass[strlen(pass)-1] = 0;
  }
  fclose(file);
  sprintf(&(payload[strlen(payload)]), "\"ssid\":\"%s\",", fileBuf);
  sprintf(&(payload[strlen(payload)]), "\"pass\":\"%s\"}", pass);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header,"Content-Length:%d\r\n",strlen(payload));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, payload, strlen(payload));
}

void
ResponseLogin(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char userName[USER_LENGTH_MAX];
  unsigned char header[128];
  unsigned char payload[8];
  unsigned char *dst = userName;
  unsigned short userId = 0;
  size--;
  in++;
  for(int i=0;i<size;++i)
  {
    *dst = *in;
    dst++;
    in++;
  }
  *dst = 0;
  LockUser();
  userId = CreateUser(userName);
  UnlockUser();
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(payload, "%d", userId);
  sprintf(header, "Content-Length:%d\r\n", strlen(payload));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, payload, strlen(payload));
}

void
WalkDir(
  char *path,
  unsigned short length
)
{
  DIR *dir = NULL;
  struct dirent *dp = NULL;
  unsigned short len = 0;

  dir = opendir(path);
  if (NULL == dir)
  {
    return;
  }
  path[length] = '/';
  path[length + 1] = 0;
  for(dp = readdir(dir); NULL != dp; dp = readdir(dir))
  {
    if (4 == dp->d_type)
    {
      if ((!strcmp("..", dp->d_name) && 2 == strlen(dp->d_name)) ||
          (!strcmp(".", dp->d_name)  && 1 == strlen(dp->d_name)) ||
          (!strcmp("$RECYCLE.BIN", dp->d_name)  && 12 == strlen(dp->d_name)))
      {
        continue;
      }
      strcpy(&(path[length + 1]), dp->d_name);
      WalkDir(path, strlen(path));
    }
    else if (8 == dp->d_type)
    {
      len = strlen(dp->d_name);
      if ('4' != dp->d_name[len - 1] || 
          'p' != dp->d_name[len - 2] ||
          'm' != dp->d_name[len - 3] ||
          '.' != dp->d_name[len - 4])
      {
        if ('i' != dp->d_name[len - 1] ||
            'v' != dp->d_name[len - 2] ||
            'a' != dp->d_name[len - 3])
        {
          if ('v' != dp->d_name[len - 1] ||
              'k' != dp->d_name[len - 2] ||
              'm' != dp->d_name[len - 3])
          {
            continue;
          }
        }
      }
      strcpy(&(path[length + 1]), dp->d_name);
      LockFile();
      AddFile(path);
      UnlockFile();
    }
  }
  closedir(dir);
}

void
UpdateSongService(
  void
)
{
  char dir[PATH_LENGTH_MAX];
  strcpy(dir, "/media/pi");
  LockFile();
  ResetFile();
  OpenFile();
  UnlockFile();
  WalkDir(dir, strlen("/media/pi"));
}

void
ResponseUpdateSong(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char header[64];
  char *sendBuf = "true";
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header,"Content-Length:%d\r\n",strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
  UpdateSongService();
}

void
ResponseSearch(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char path[PAGERECORDS][PATH_LENGTH_MAX];
  unsigned long fileIdList[PAGERECORDS];
  unsigned char recordsInPage = 0;
  long pageNum = 0;
  unsigned long records = 0;
  unsigned short offsetList[PAGERECORDS];
  unsigned char sort = 0;
  unsigned long page = 0;
  unsigned char keyword[1024];
  unsigned char *dst = keyword;
  unsigned char buf[1024 * 16];
  unsigned char header[128];
  unsigned long playCount = 0;

  int index = 0;
  in++;
  size--;
  if ('1' == *in)
  {
    sort = 1;
  }
  in++;
  size--;
  while(',' != *in)
  {
    page = page * 10 + *in - '0';
    in++;
    size--;
  }
  in++;
  size--;
  for(int i=0; i<size; ++i)
  {
    *dst = *in;
    in++;
    dst++;
  }
  *dst = 0;
  LockFile();
  records = GetFileList(
    keyword, 
    page,
    sort, 
    fileIdList,
    (char*)path, 
    offsetList, 
    &recordsInPage, 
    &pageNum);
  UnlockFile();
  index = sprintf(buf, "{\"page\":%d,\"record\":%d,\"files\":[",pageNum, records);
  int flag = 0;
  LockHistory();
  for(int i=0;i<recordsInPage;++i)
  {
    if (0 == flag)
    {
      flag = 1;
    }
    else
    {
      index += sprintf(&(buf[index]),",");
    }
    index += sprintf(&(buf[index]), "{\"fileId\":%d,", fileIdList[i]);
    playCount = HasHistory(path[i]);
    if (0 == playCount)
    {
      index += sprintf(&(buf[index]), "\"played\":false,");
    }
    else
    {
      index += sprintf(&(buf[index]), "\"played\":true,");
    }
    index += sprintf(&(buf[index]), "\"name\":\"%s\"}", &(path[i][offsetList[i]]));
  }
  UnlockHistory();
  index += sprintf(&(buf[index]), "]}\n");

  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header,"Content-Length:%d\r\n",index);
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, buf, index);
}

void
ResponseFileDetail(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long fileId = 0;
  unsigned char path[PATH_LENGTH_MAX];
  FILE *fp = NULL;
  AVFormatContext* formatContext = NULL;
  AVDictionaryEntry *dicEntry = NULL;
  in++;
  size--;
  unsigned char audioTrackCount = 0;
  unsigned char audioTrackName[16][128];
  unsigned char modifyTime[32];
  unsigned char sendBuf[1024 * 4];
  unsigned char header[128];
  long fileSize = 0;
  unsigned long length = 0;
  unsigned long playCount = 0;

  strcpy(sendBuf, "");

  while(size > 0)
  {
    fileId = fileId * 10 + *in - '0';
    in++;
    size--;
  }
  LockFile();
  fileSize = GetFileDetail(fileId, path, modifyTime);
  UnlockFile();
  if (-1 == fileSize)
  {
    // < Not Found >
    goto error;
  }
  sprintf(sendBuf, "{\"path\":\"%s\",\"size\":%d,\"date\":\"%s\",",
    path, fileSize, modifyTime);
  if (0 != avformat_open_input(&formatContext, path, NULL, NULL))
  {
    // < Can not open  >
    goto error;
  }
  if (0 > avformat_find_stream_info(formatContext, NULL))
  {
    // < Can not read stream >
    goto error;
  }
  LockHistory();
  playCount = HasHistory(path);
  UnlockHistory();
  sprintf(&(sendBuf[strlen(sendBuf)]), "\"count\":%d,", playCount);
  for(int i=0; i<(int)formatContext->nb_streams; ++i)
  {
    if (AVMEDIA_TYPE_VIDEO == formatContext->streams[i]->codecpar->codec_type)
    {
      // < Video >
      sprintf(&(sendBuf[strlen(sendBuf)]), "\"duration\":%d,",
        formatContext->duration/1000000);
      sprintf(&(sendBuf[strlen(sendBuf)]), "\"videoCodec\":\"%s\",",
        avcodec_get_name(formatContext->streams[i]->codecpar->codec_id));
      sprintf(&(sendBuf[strlen(sendBuf)]), "\"resolution\":\"%d x %d\",\"audioTrack\":[",
        formatContext->streams[i]->codecpar->width,
        formatContext->streams[i]->codecpar->height);
    }
    else if (AVMEDIA_TYPE_AUDIO == formatContext->streams[i]->codecpar->codec_type)
    {
      // < Audio >
      dicEntry = NULL;
      do
      {
        dicEntry = av_dict_get(formatContext->streams[i]->metadata, "", dicEntry, AV_DICT_IGNORE_SUFFIX);
	if (NULL == dicEntry)
        {
          break;
        }
        if (!strcmp(dicEntry->key, "handler_name"))
        {
          break;
        }
      }while(NULL != dicEntry);
      if (NULL != dicEntry)
      {
        strcpy(audioTrackName[audioTrackCount], dicEntry->value);
      }
      else
      {
        strcpy(audioTrackName[audioTrackCount], "None");
      }
      audioTrackCount++;
    }
  }
  avformat_close_input(&formatContext);
  sprintf(&(sendBuf[strlen(sendBuf)]), "\"%s\"", audioTrackName[0]);
  for(int i=1; i<audioTrackCount; ++i)
  {
    sprintf(&(sendBuf[strlen(sendBuf)]), ",\"%s\"", audioTrackName[i]);
  }
  strcpy(&(sendBuf[strlen(sendBuf)]), "]}");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
  return;
error:
  strcpy(sendBuf, "{\"path\":\"\"}");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
  return;
}

void
ResponsePlayList(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long bookIdList[PAGERECORDS];
  unsigned short userIdList[PAGERECORDS];
  unsigned char pathList[PAGERECORDS][PATH_LENGTH_MAX];
  unsigned char commentList[PAGERECORDS][COMMENT_LENGTH_MAX];
  unsigned long durationList[PAGERECORDS];
  unsigned long record = 0;
  unsigned char sendBuf[1024 * 64] = { 0 };
  unsigned char header[32] = { 0 };
  unsigned char userName[USER_LENGTH_MAX] = { 0 };
  unsigned char flag = 0;

  LockBook();
  record = GetBookList(bookIdList, userIdList, (char*)pathList, (char*)commentList, durationList);
  UnlockBook();
  strcpy(sendBuf, "{\"restCurrentTime\":");
  LockPlayer();
  if (0 == playerState ||
      3 == playerState ||
      4 == playerState)
  {
    sprintf(&(sendBuf[strlen(sendBuf)]), "0,");
  }
  else
  {
    sprintf(&(sendBuf[strlen(sendBuf)]), "%d,", playDuration - (playTime/1000));
  }
  UnlockPlayer();

  sprintf(&(sendBuf[strlen(sendBuf)]), "\"playList\":[");

  LockUser();
  for (int i=0; i<record; ++i)
  {
    if (0 == flag)
    {
      flag = 1;
    }
    else
    {
      sprintf(&(sendBuf[strlen(sendBuf)]), ",");
    }

    sprintf(&(sendBuf[strlen(sendBuf)]), "{\"bookId\":%d,", bookIdList[i]);
    if (0 == userIdList[i])
    {
      *userName = 0;
    }
    else
    {
      GetUserName(userIdList[i], userName);
    }
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"user\":\"%s\",", userName);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"path\":\"%s\",", pathList[i]);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"comment\":\"%s\",", commentList[i]);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"duration\":%d}", durationList[i]);
  }
  UnlockUser();
  sprintf(&(sendBuf[strlen(sendBuf)]), "]}");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
GuiPlayList(
)
{
  unsigned long bookIdList[PAGERECORDS];
  unsigned short userIdList[PAGERECORDS];
  unsigned char pathList[PAGERECORDS][PATH_LENGTH_MAX];
  unsigned char commentList[PAGERECORDS][COMMENT_LENGTH_MAX];
  unsigned long durationList[PAGERECORDS];
  unsigned long record = 0;
  unsigned char sendBuf[1024 * 64] = { 0 };
  unsigned char header[32] = { 0 };
  unsigned char userName[USER_LENGTH_MAX] = { 0 };
  unsigned char userNameListLabel[PAGERECORDS * USER_LENGTH_MAX] = { 0 };
  unsigned char pathListLabel[PAGERECORDS * PATH_LENGTH_MAX] = { 0 };
  unsigned char commentListLabel[PAGERECORDS * COMMENT_LENGTH_MAX] = { 0 };
  unsigned char *songName = NULL;
  unsigned char *songCursor = NULL;
  unsigned char pauseStr[128] = { 0 };

  LockBook();
  record = GetBookList(bookIdList, userIdList, (char*)pathList, (char*)commentList, durationList);
  UnlockBook();

  sprintf(userNameListLabel, "<span bgcolor=\"#6495ED\" size=\"xx-large\">");
  sprintf(pathListLabel, "<span bgcolor=\"#FAF0E6\" size=\"xx-large\">");
  sprintf(commentListLabel, "<span bgcolor=\"#FFE4E1\" size=\"xx-large\">");
  LockUser();
  for (int i=0; i<record; ++i)
  {
    if (0 == userIdList[i])
    {
      *userName = 0;
    }
    else
    {
      GetUserName(userIdList[i], userName);
    }
    sprintf(&(userNameListLabel[strlen(userNameListLabel)]), "%s\n", userName);
    songCursor = pathList[i];
    songName = songCursor;
    while (0 != *songCursor)
    {
      if ('/' == *songCursor)
      {
        songName = songCursor + 1;
      }
      songCursor++;
    }
    sprintf(&(pathListLabel[strlen(pathListLabel)]), "%s\n", songName);
    sprintf(&(commentListLabel[strlen(commentListLabel)]), "%s\n", commentList[i]);
  }
  UnlockUser();
  sprintf(&(userNameListLabel[strlen(userNameListLabel)]), "</span>");
  sprintf(&(pathListLabel[strlen(pathListLabel)]), "</span>");
  sprintf(&(commentListLabel[strlen(commentListLabel)]), "</span>");
  if ((2 == playerState) || (3 == playerState) || (5 == playerState))
  {
    sprintf(pauseStr, "<span bgcolor=\"#FFFFFF\" size=\"xx-large\">Pause</span>");
  }
  else
  {
    *pauseStr = 0;
  }
  gtk_label_set_markup(GTK_LABEL(songLabel), pathListLabel);
  gtk_label_set_markup(GTK_LABEL(userLabel), userNameListLabel);
  gtk_label_set_markup(GTK_LABEL(commentLabel), commentListLabel);
  gtk_label_set_markup(GTK_LABEL(pauseLabel), pauseStr);
}

void
ResponseAddBook(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char sendBuf[32] = { 0 };
  unsigned char header[32] = { 0 };
  unsigned short userId = 0;
  unsigned long fileId = 0;
  unsigned char audio = 0;
  long pos = 0;
  unsigned char pause = 0;
  unsigned char secret = 0;
  unsigned char comment[COMMENT_LENGTH_MAX] = { 0 };
  unsigned long bookId = 0;
  unsigned char *p = comment;
  unsigned char path[PATH_LENGTH_MAX] = { 0 };
  unsigned long duration = 0;
  AVFormatContext* formatContext = NULL;
  unsigned char posSign = 0;

  in++;
  size--;
  while(',' != *in)
  {
    userId = userId * 10 + *in - '0';
    in++;
    size--;
  }
  in++;
  size--;
  while(',' != *in)
  {
    fileId = fileId * 10 + *in - '0';
    in++;
    size--;
  }
  in++;
  size--;
  audio = *in - '0';
  in++;
  size--;
  posSign = *in;
  in++;
  size--;
  while(',' != *in)
  {
    pos = *in - '0';
    in++;
    size--;
  }
  if ('-' == posSign)
  {
    pos = -1 * pos;
  }
  in++;
  size--;
  pause = *in - '0';
  in++;
  size--;
  secret = *in - '0';
  in++;
  size--;
  
  while(size>0)
  {
    *p = *in;
    in++;
    p++;
    size--;
  }
  if (0 != fileId)
  {
    LockFile();
    GetFilePath(fileId, path);
    UnlockFile();

    if (0 != avformat_open_input(&formatContext, path, NULL, NULL))
    {
      // < Can not open  >
      goto error;
    }
    if (0 > avformat_find_stream_info(formatContext, NULL))
    {
      // < Can not read stream >
      goto error;
    }
    duration = formatContext->duration/1000000;
    avformat_close_input(&formatContext);
  }
  if (-2 == pos)
  {
    if (0 != fileId)
    {
      LockBook();
      bookId = AddBook(userId, path, comment, duration, audio, secret);
      UnlockBook();
    }
    if (1 == pause)
    {
      LockBook();
      bookId = AddBreak();
      UnlockBook();
    }
  }
  else if (-1 == pos)
  {
    if (1 == pause)
    {
      LockBook();
      bookId = InsertBreak();
      UnlockBook();
    }
    if (0 != fileId)
    {
      LockBook();
      bookId = InsertBook(userId, path, comment, duration, audio, secret);
      UnlockBook();
    }
  }
  sprintf(sendBuf, "%d", bookId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
  return;
error:
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:1\r\n");
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, "0", 1);
  return;
}

void
ResponseDeleteBook(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[32];
  unsigned char sendBuf[32];
  unsigned long bookId = 0;
  in++;
  size--;
  while(size > 0)
  {
    bookId = bookId * 10 + *in - '0';
    in++;
    size--;
  }
  LockBook();
  DeleteBook(bookId);
  UnlockBook();
  sprintf(sendBuf, "%d", bookId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseMoveBook(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long bookId = 0;
  unsigned char dir = 1;
  unsigned char sendBuf[32];
  unsigned char header[32];
  in++;
  size--;
  dir ^= *in - '0';
  in++;
  size--;
  while (size > 0)
  {
    bookId = bookId * 10 + *in - '0';
    size--;
    in++;
  }
  LockBook();
  MoveBook(bookId, dir);
  UnlockBook();
  sprintf(sendBuf, "%d", bookId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseLatestBook(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long latestBookId = 0;
  char latestUser[USER_LENGTH_MAX];
  char latestFile[PATH_LENGTH_MAX];
  char latestComment[COMMENT_LENGTH_MAX];
  unsigned short latestUserId = 0;
  unsigned char latestVisible = 0;
  unsigned char sendBuf[1024 * 16];
  unsigned char header[32];
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  GetLatestBook(
    &latestBookId,
    &latestUserId,
    latestFile,
    latestComment,
    &latestVisible);
  LockUser();
  if (0 == GetUserName(latestUserId, latestUser))
  {
    latestUser[0] = '\0';
  }
  UnlockUser();
  if (0 == latestBookId)
  {
    sprintf(sendBuf, "{\"bookId\":0}");
  }
  else
  {
    sprintf(sendBuf, "{\"bookId\":%d,", latestBookId);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"user\":\"%s\",", latestUser);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"file\":\"%s\",", latestFile);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"comment\":\"%s\",", latestComment);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"userId\":%d,", latestUserId);
    if (0 == latestVisible)
    {
      sprintf(&(sendBuf[strlen(sendBuf)]), "\"visible\":false}");
    }
    else
    {
      sprintf(&(sendBuf[strlen(sendBuf)]), "\"visible\":true}");
    }
  }
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseHistoryList(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long pageNo = 0;
  unsigned char sendBuf[1024 * 64];
  unsigned char header[32];
  unsigned long historyIdList[PAGERECORDS];
  unsigned short userIdList[PAGERECORDS];
  char filePathList[PAGERECORDS][PATH_LENGTH_MAX];
  char commentList[PAGERECORDS][COMMENT_LENGTH_MAX];
  char playTimeList[PAGERECORDS][20];
  char userName[USER_LENGTH_MAX];
  unsigned long records = 0;
  unsigned char recordInPages = 0;
  unsigned long pageNum = 0;
  unsigned char flag = 0;

  in++;
  size--;
  while (size > 0)
  {
    pageNo = pageNo * 10 + *in - '0';
    size--;
    in++;
  }
  LockHistory();
  records = GetHistoryList(pageNo, historyIdList, userIdList, (char*)filePathList, (char*)commentList, (char*)playTimeList, &recordInPages, &pageNum);
  UnlockHistory();
  sprintf(sendBuf, "{\"page\":%d,", pageNum);
  sprintf(&(sendBuf[strlen(sendBuf)]), "\"record\":%d,", records);
  sprintf(&(sendBuf[strlen(sendBuf)]), "\"histories\":[");
  LockUser();
  for (unsigned char index = 0; index < recordInPages; ++ index)
  {
    GetUserName(userIdList[index], userName);
    if (1 == flag)
    {
      sprintf(&(sendBuf[strlen(sendBuf)]), ",");
    }
    else
    {
      flag = 1;
    }
    sprintf(&(sendBuf[strlen(sendBuf)]), "{\"historyId\":%d,", historyIdList[index]);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"user\":\"%s\",", userName);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"path\":\"%s\",", filePathList[index]);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"comment\":\"%s\",", commentList[index]);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"time\":\"%s\"}", playTimeList[index]);
    
  }
  UnlockUser();
  sprintf(&(sendBuf[strlen(sendBuf)]), "]}");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseHistoryCSV(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char fileName[32];
  char header[128];
  char fileBuf[1024*1024];
  struct stat st;
  FILE *file = NULL;
  int fileSize = 0;
  unsigned long historyId = 0;
  in++;
  size--;
  while (size > 0)
  {
    historyId = *in - '0' + historyId;
    in++;
    size--;
  }
  sprintf(fileName, "%d.csv", conn);
  LockUser();
  LockHistory();
  ExtractHistory(historyId, fileName);
  UnlockHistory();
  UnlockUser();
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, CSV_HEADER, strlen(CSV_HEADER));
  sprintf(header, "Content-Disposition: attachment; filename=history.csv\r\n");
  SendText(conn, header, strlen(header));
  stat(fileName, &st);
  sprintf(header, "Content-Length:%d\r\n", st.st_size);
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  file = fopen(fileName, "rb");
  while (fileSize = fread(fileBuf, sizeof(char), sizeof(fileBuf), file))
  {
    SendText(conn, fileBuf, fileSize);
  }
  fclose(file);
  remove(fileName);
}

void
ResponseDeleteHistory(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long historyId = 0;
  unsigned char header[32];
  unsigned char sendBuf[32];

  in++;
  size--;
  while (size > 0)
  {
    historyId = historyId * 10 + *in - '0';
    in++;
    size--;
  }
  LockHistory();
  DeleteHistory(historyId);
  UnlockHistory();
  sprintf(sendBuf, "%d", historyId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(OK_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseModifyHistory(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned long historyId = 0;
  unsigned char comment[COMMENT_LENGTH_MAX];
  unsigned char header[32];
  unsigned char sendBuf[32];
  unsigned char *dst = comment;

  in++;
  size--;
  while (size > 0 && ',' != *in)
  {
    historyId = historyId * 10 + *in - '0';
    in++;
    size--;
  }
  in++;
  size--;
  while (size > 0)
  {
    *dst = *in;
    dst++;
    in++;
    size--;
  }
  *dst = 0;
  LockHistory();
  ModifyHistory(historyId, comment);
  UnlockHistory();
  sprintf(sendBuf, "%d", historyId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(OK_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ShutdownService(
  void
)
{
  system("sudo shutdown -h now");
}

void
RestartService(
  void
)
{
  system("sudo shutdown -r now");
}

void
ResponseSystemDown(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[32];
  unsigned char sendBuf[32];
  unsigned char off = 0;
  in++;
  size--;
  off = *in - '0';
  strcpy(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
  if (0 == off)
  {
    ShutdownService();
  }
  else
  {
    RestartService();
  }
}

void
ResponseResetHistory(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[32];
  unsigned char sendBuf[32];
  LockHistory();
  ResetHistory();
  OpenHistory();
  UnlockHistory();
  strcpy(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseAddFavorite(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned short userId = 0;
  unsigned long favoriteId = 0;
  unsigned long fileId = 0;
  unsigned char filePath[PATH_LENGTH_MAX] = { 0 };
  unsigned char header[32];
  unsigned char sendBuf[32];
  in++;
  size--;
  while (',' != *in)
  {
    fileId = fileId * 10 + *in - '0';
    in++;
    size--;
  }
  in++;
  size--;
  while (size > 0)
  {
    userId = userId * 10 + *in - '0';
    size--;
    in++;
  }
  LockFile();
  GetFilePath(fileId, filePath);
  UnlockFile();
  favoriteId = AddFavorite(userId, filePath);
  sprintf(sendBuf, "%d", favoriteId);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseSetAudioConfig(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[32];
  unsigned char sendBuf[1024];
  unsigned char command[256];
  char *result = NULL;
  FILE *file = NULL;
  in++;
  size--;
  sprintf(command, "sh confAudio.sh %c", *in);
  system(command);
  sprintf(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseGetAudioConfig(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  unsigned char header[32];
  unsigned char sendBuf[1024];
  unsigned char line[256];
  unsigned char firstLine = 1;
  unsigned char headChar = '-';
  char *result = NULL;
  FILE *file = NULL;
  *line = '0';
  in++;
  size--;
  system("sh getAudio.sh > getAudio.txt");
  system("sh listAudio.sh > listAudio.txt");
  file = fopen("getAudio.txt", "r");
  fgets(line, 256, file);
  line[strlen(line) - 1] = 0;
  sprintf(sendBuf,"{\"no\":%s,\"list\":[",line);
  fclose(file);
  file = fopen("listAudio.txt", "r");
  fgets(line, 256, file);
  while (*line != headChar)
  {
    if (1 == firstLine)
    {
      firstLine = 0;
    }
    else
    {
      sprintf(&(sendBuf[strlen(sendBuf)]), ",");
    }
    line[strlen(line) - 1] = 0;
    sprintf(&(sendBuf[strlen(sendBuf)]), "{\"no\":%c,", *line);
    sprintf(&(sendBuf[strlen(sendBuf)]), "\"name\":\"%s\"}", &(line[1]));
    headChar = *line;
    fgets(line, 256, file);
  }
  sprintf(&(sendBuf[strlen(sendBuf)]), "]}");
  fclose(file);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void*
StopVideoWithFade(
  void* p
)
{
  int i = 0;
  int volume = 0;

  FadeoutVolume();  
  LockPlayer();
  if (4 == playerState)
  {
    playerState = 0;
  }
  else if (5 == playerState)
  {
    playerState = 3;
  }
  StopVideo();
  UnlockPlayer();
}

void
StopVideoService(
  void
)
{
  pthread_t thread;
  LockPlayer();
  if (1 == playerState)
  {
    playerState = 4;
  }
  else if (2 == playerState)
  {
    playerState = 5;
  }
  pthread_create(&thread, NULL, StopVideoWithFade, NULL);
  pthread_detach(thread);
  UnlockPlayer();
}

void
ResponseStopVideo(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[32];
  char header[32];
  StopVideoService();
  strcpy(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
PauseVideoService(
  void
)
{
  PauseVideo();
  LockPlayer();
  if (playerState == 1)
  {
    playerState = 2;
  }
  else if (playerState == 2)
  {
    playerState = 1;
  }
  else if (playerState == 3)
  {
    playerState = 0;
  }
  else if (playerState == 0)
  {
    playerState = 3;
  }
  UnlockPlayer();
}

void
ResponsePauseVideo(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[32];
  char header[32];
  PauseVideoService();
  strcpy(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseSeekVideo(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[32];
  char header[32];
  in++;
  size--;
  if ('0' == *in)
  {
    RWVideo();
  }
  else if ('1' == *in)
  {
    FFVideo();
  }

  sprintf(sendBuf, "%lld", playTime / 1000);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseSetVol(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[32];
  char header[32];
  in++;
  size--;
  if ('0' == *in)
  {
    DownVolume();
  }
  else if ('1' == *in)
  {
    UpVolume();
  }
  sprintf(sendBuf, "%d", GetVolume());
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseChangeAudio(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[32];
  char header[32];
  ChangeAudioTrack();
  strcpy(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponsePlayInfo(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[1024];
  char header[32];
  int count = 0;
  int current = 0;
  LockPlayer();
  if (0 == playerState)
  {
    sprintf(sendBuf,"{\"state\":0,\"vol\":%d}",GetVolume());
  }
  else
  {
    sprintf(sendBuf,"{\"user\":\"%s\",", GetSinger());
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"path\":\"%s\",",GetPlayingPath());
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"pos\":%d,",playTime / 1000);
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"duration\":%d,",playDuration);
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"state\":%d,", playerState);
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"comment\":\"%s\",", GetSingerComment());
    sprintf(&(sendBuf[strlen(sendBuf)]),"\"vol\":%d}",GetVolume());
  }
  UnlockPlayer();
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseGetAPConfig(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[128];
  char pass[128];
  char header[32];
  char *result = NULL;
  system("sh AccessPointPass.sh > AccessPointPass.txt");
  FILE *file = fopen("AccessPointPass.txt", "r");
  result = fgets(pass, 64, file);
  fclose(file);
  if (NULL == result)
  {
    sprintf(sendBuf, "{\"enable\":false}");
  }
  else
  {
    pass[strlen(pass) - 1] = 0;
    sprintf(sendBuf, "{\"enable\":true, \"phrase\":\"%s\"}", pass);
  }
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseUpdateAPConfig(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[128];
  char pass[128];
  char *dst = pass;
  char command[256];
  char header[32];
  char *result = NULL;
  in++;
  size--;
  if ('0' == *in)
  {
    system("sh disableAccessPointPass.sh");
  }
  else
  {
    in++;
    size--;
    while (size > 0)
    {
      *dst = *in;
      in++;
      size--;
      dst++;
    }
    *dst = 0;
    sprintf(command, "sh enableAccessPointPass.sh %s", pass);
    system(command);
  }
  sprintf(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseGetPlayMode(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[128];
  char header[32];
  in++;
  size--;
  sprintf(sendBuf, "{\"mode\":%d}", playMode);
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ResponseSetPlayMode(
  int conn,
  unsigned char *in,
  unsigned long long size
)
{
  char sendBuf[128];
  char header[32];
  in++;
  size--;
  if ('0' == *in)
  {
    playMode = 0;
  }
  else if ('1' == *in)
  {
    playMode = 1;
  }
  sprintf(sendBuf, "true");
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(header, "Content-Length:%d\r\n", strlen(sendBuf));
  SendText(conn, header, strlen(header));
  SendText(conn, "\r\n", 2);
  SendText(conn, sendBuf, strlen(sendBuf));
}

void
ChangeAudioConfig(
  void
  )
{
  unsigned char config = 0;
  unsigned char configMax = 0;
  unsigned char configTemp = 0;
  unsigned char headChar = '-';
  unsigned char command[256];
  unsigned char line[256];
  FILE *file = NULL;

  system("sh getAudio.sh > getAudio.txt");
  file = fopen("getAudio.txt", "r");
  fgets(line, 256, file);
  config = *line - '0';
  fclose(file);

  system("sh listAudio.sh > listAudio.txt");
  file = fopen("listAudio.txt", "r");
  fgets(line, 256, file);
  while (*line != headChar)
  {
    configTemp = *line - '0';
    if (configMax < configTemp)
    {
      configMax = configTemp;
    }
    headChar = *line;
    fgets(line, 256, file);
  }
  
  config++;
  if (config > configMax)
  {
    config = 0;
  }

  sprintf(command, "sh confAudio.sh %d", config);
  system(command);
}

void
RecvHttpRequest(
  ssize_t *recvSize,
  int conn,
  char *buf,
  ssize_t *totalSize,
  ssize_t *headerSize,
  long long *contentSize,
  ssize_t *packetSize,
  ssize_t *nextSize
)
{
    for(;;)
    {
      *recvSize = recv(
        conn, 
        buf + *totalSize,
        1024 - *totalSize,
        0
      );
      if (-1 == *recvSize)
      {
        fprintf(stderr,"recv err\n");
        close(conn);
        pthread_exit(NULL);
      }
      *totalSize += *recvSize;
      if (-1 == *recvSize)
      {
        if (11 == errno) // Try again
        {
          continue;
        }
        break;
      }
      else if (0 == *recvSize)
      {
        break;
      }
      *headerSize = CheckHeader(buf, *totalSize);
      if (0 == *headerSize)
      {
        continue;
      }
      *contentSize = GetContentSize(buf, *totalSize);
      if (-1 == *contentSize)
      {
        continue;
      }
      *packetSize = *headerSize + *contentSize;
      if (*totalSize < *packetSize)
      {
        continue;
      }
      *nextSize = *totalSize - *packetSize;
      break;
    }
}

void
SendHtmlHeaders(
  int conn,
  off_t fileSize,
  unsigned long begin,
  unsigned long end,
  unsigned char isRange
)
{
  char buf[1024];
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  sprintf(buf,"Content-Length:%d\r\n", fileSize);
  SendText(conn, buf, strlen(buf));
  SendText(conn, "\r\n", 2);
}

void
SendCssHeaders(
  int conn,
  off_t fileSize,
  unsigned long begin,
  unsigned long end,
  unsigned char isRange
)
{
  char buf[1024];
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, CSS_HEADER, strlen(CSS_HEADER));
  sprintf(buf,"Content-Length:%d\r\n", fileSize);
  SendText(conn, buf, strlen(buf));
  SendText(conn, "\r\n", 2);
}

void
SendJsHeaders(
  int conn,
  off_t fileSize,
  unsigned long begin,
  unsigned long end,
  unsigned char isRange
)
{
  char buf[1024];
  SendText(conn, OK_HEADER, strlen(OK_HEADER));
  SendText(conn, JS_HEADER, strlen(JS_HEADER));
  sprintf(buf,"Content-Length:%d\r\n", fileSize);
  SendText(conn, buf, strlen(buf));
  SendText(conn, "\r\n", 2);
}

unsigned char
GenerateThumbnail(
  char *fileName
)
{
  unsigned long fileId = 0;
  char filePath[PATH_LENGTH_MAX];
  char command[PATH_LENGTH_MAX * 2];
  struct stat st = {0};
  if ('t' != *fileName)
  {
    return 0;
  }
  fileName++;
  if ('/' != *fileName)
  {
    return 0;
  }
  fileName++;
  while ('.' != *fileName)
  {
    fileId = fileId * 10 + *fileName - '0';
    fileName++;
  }
  LockFile();
  GetFilePath(fileId, filePath);
  UnlockFile();
  sprintf(command, "ffmpeg -ss 10 -t 1 -r 1 -i \"%s\" -f image2 -s 120x90 ", filePath);
  sprintf(&command[strlen(command)], "t/%d.png 2> /dev/null", fileId);
  system(command);
  sprintf(command, "t/%d.png", fileId);
  if (0 != stat(command, &st))
  {
    return 0;
  }
  return 1;
}

void
ProcessHttpGet(
  int conn,
  unsigned char *buf,
  ssize_t packetSize
)
{
  unsigned char fileName[PATH_LENGTH_MAX];
  unsigned char mediaType = 0;
  unsigned char fileBuf[1500];
  unsigned long long begin = 0;
  unsigned long long end = 0;
  unsigned char isRange = 0;
  unsigned long long reqSize = 0;
  ParseHttpRequest(buf, fileName, &mediaType, &begin, &end, packetSize, &isRange);
  struct stat st;
  if (0 != stat(fileName, &st))
  {
    if (0 == GenerateThumbnail(fileName))
    {
      SendText(conn, NOT_FOUND_MESSAGE, strlen(NOT_FOUND_MESSAGE));
      return;
    }
    else
    {
      stat(fileName, &st);
      SendText(conn, OK_HEADER, strlen(OK_HEADER));
      SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
    }
  }
  if (0 == mediaType)
  {
    SendText(conn, OK_HEADER, strlen(OK_HEADER));
    SendText(conn, HTML_HEADER, strlen(HTML_HEADER));
  }
  else if (1 == mediaType)
  {
    SendText(conn, OK_HEADER, strlen(OK_HEADER));
    SendText(conn, CSS_HEADER, strlen(CSS_HEADER));
  }
  else if (2 == mediaType)
  {
    SendText(conn, OK_HEADER, strlen(OK_HEADER));
    SendText(conn, JS_HEADER, strlen(JS_HEADER));
  }
  else if (3 == mediaType)
  {
    if (0 == isRange)
    {
      SendText(conn, OK_HEADER, strlen(OK_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    else
    {
      SendText(conn, PARTIAL_HEADER, strlen(PARTIAL_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    SendText(conn, AVI_HEADER, strlen(AVI_HEADER));
    if (0 == end)
    {
      end = st.st_size - 1;
    }
    if (1 == isRange)
    {
      sprintf(fileBuf, CONTENT_RANGE_HEADER ,begin);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d/", end);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d\r\n", st.st_size);
      SendText(conn, fileBuf, strlen(fileBuf));
    }
  }
  else if (4 == mediaType)
  {
    if (0 == isRange)
    {
      SendText(conn, OK_HEADER, strlen(OK_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    else
    {
      SendText(conn, PARTIAL_HEADER, strlen(PARTIAL_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    SendText(conn, MP4_HEADER, strlen(MP4_HEADER));
    if (0 == end)
    {
      end = st.st_size - 1;
    }
    if (1 == isRange)
    {
      sprintf(fileBuf, CONTENT_RANGE_HEADER ,begin);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d/", end);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d\r\n", st.st_size);
      SendText(conn, fileBuf, strlen(fileBuf));
    }
  }
  else if (5 == mediaType)
  {
    if (0 == isRange)
    {
      SendText(conn, OK_HEADER, strlen(OK_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    else
    {
      SendText(conn, PARTIAL_HEADER, strlen(PARTIAL_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    SendText(conn, MKV_HEADER, strlen(MKV_HEADER));
    if (0 == end)
    {
      end = st.st_size - 1;
    }
    if (1 == isRange)
    {
      sprintf(fileBuf, CONTENT_RANGE_HEADER ,begin);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d/", end);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d\r\n", st.st_size);
      SendText(conn, fileBuf, strlen(fileBuf));
    }
  }
  else if (6 == mediaType)
  {
    if (0 == isRange)
    {
      SendText(conn, OK_HEADER, strlen(OK_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    else
    {
      SendText(conn, PARTIAL_HEADER, strlen(PARTIAL_HEADER));
      SendText(conn, "Accept-Ranges: bytes\r\n", strlen("Accept-Ranges: bytes\r\n"));
    }
    SendText(conn, FLV_HEADER, strlen(FLV_HEADER));
    if (0 == end)
    {
      end = st.st_size - 1;
    }
    if (1 == isRange)
    {
      sprintf(fileBuf, CONTENT_RANGE_HEADER ,begin);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d/", end);
      sprintf(&(fileBuf[strlen(fileBuf)]), "%d\r\n", st.st_size);
      SendText(conn, fileBuf, strlen(fileBuf));
    }
  }
  FILE* file = fopen(fileName,"rb");
  int fileSize = 0;
  unsigned long limitSize = 0;
  fseek(file, begin, SEEK_SET);
  if (0 == end)
  {
    end = st.st_size;
  }
  reqSize = end - begin + 1;
  if (1 == isRange)
  {
    sprintf(fileBuf,"Content-Length:%d\r\n", reqSize);
  }
  else
  {
    sprintf(fileBuf,"Content-Length:%d\r\n", st.st_size);
  }
  SendText(conn, fileBuf, strlen(fileBuf));
  SendText(conn, "\r\n", 2);
  while(fileSize = fread(fileBuf,sizeof(char),sizeof(fileBuf),file))
  {
    if (reqSize > fileSize)
    {
      SendText(conn, fileBuf, fileSize);
    }
    else
    {
      SendText(conn, fileBuf, reqSize);
      break;
    }
    reqSize -= fileSize;
  }
  fclose(file);
}

void*
HttpThread(
  void *p
)
{
  ssize_t recvSize = 0;
  ssize_t totalSize = 0;
  ssize_t headerSize = 0;
  ssize_t nextSize = 0;
  ssize_t packetSize = 0;
  unsigned char buf[1024];
  int conn = (int)p;
  int messageType = 0;
  unsigned char *srcP = NULL;
  unsigned char *dstP = NULL;
  long long contentSize = 0;
  unsigned char *message = buf;
  int flag = 1;

  if (-1 == setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)))
  {
    fprintf(stderr, "Nagle Error \n");
    close(conn);
    pthread_exit(NULL);
  }

  for(;;)
  {
    memcpy(buf, buf + packetSize, nextSize);
    totalSize = nextSize;
    RecvHttpRequest(
      &recvSize, conn, buf, &totalSize, &headerSize,
      &contentSize, &packetSize, &nextSize);
    if (0 == recvSize)
    {
      close(conn);
      pthread_exit(NULL);
    }

    if ('G' == *buf)
    {
      ProcessHttpGet(conn, buf, packetSize);
    }
    else
    {
      message = SkipMessage(buf, packetSize);
      if ('1' == *message)
      {
        ResponseConnect(conn, message, contentSize);
      }
      else if ('0' == *message)
      {
        ResponseLogin(conn, message, contentSize);
      }
      else if ('A' == *message)
      {
        ResponseUpdateSong(conn, message, contentSize);
      }
      else if ('2' == *message)
      {
        ResponseSearch(conn, message, contentSize);
      }
      else if('3' == *message)
      {
        ResponseFileDetail(conn, message, contentSize);
      }
      else if ('4' == *message)
      {
        ResponsePlayList(conn, message, contentSize);
      }
      else if('5' == *message)
      {
        ResponseAddBook(conn, message, contentSize);
      }
      else if('7' == *message)
      {
        ResponseDeleteBook(conn, message, contentSize);
      }
      else if('8' == *message)
      {
        ResponseMoveBook(conn, message, contentSize);
      }
      else if('B' == *message)
      {
        ResponseLatestBook(conn, message, contentSize);
      }
      else if('C' == *message)
      {
        ResponseHistoryList(conn, message, contentSize);
      }
      else if('D' == *message)
      {
        ResponseHistoryCSV(conn, message, contentSize);
      }
      else if('F' == *message)
      {
        ResponseDeleteHistory(conn, message, contentSize);
      }
      else if('G' == *message)
      {
        ResponseModifyHistory(conn, message, contentSize);
      }
      else if('L' == *message)
      {
        ResponseSystemDown(conn, message, contentSize);
      }
      else if('E' == *message)
      {
        ResponseResetHistory(conn, message, contentSize);
      }
      else if('J' == *message)
      {
        ResponseAddFavorite(conn, message, contentSize);
      }
      else if('N' == *message)
      {
        ResponseSetAudioConfig(conn, message, contentSize);
      }
      else if('O' == *message)
      {
        ResponseGetAudioConfig(conn, message, contentSize);
      }
      else if('R' == *message)
      {
        ResponseStopVideo(conn, message, contentSize);
      }
      else if('S' == *message)
      {
        ResponsePauseVideo(conn, message, contentSize);
      }
      else if('V' == *message)
      {
        ResponseSeekVideo(conn, message, contentSize);
      }
      else if('T' == *message)
      {
        ResponseSetVol(conn, message, contentSize);
      }
      else if('U' == *message)
      {
        ResponseChangeAudio(conn, message, contentSize);
      }
      else if('X' == *message)
      {
        ResponsePlayInfo(conn, message, contentSize);
      }
      else if('Y' == *message)
      {
        ResponseGetAPConfig(conn, message, contentSize);
      }
      else if('Z' == *message)
      {
        ResponseUpdateAPConfig(conn, message, contentSize);
      }
      else if('a' == *message)
      {
        ResponseGetPlayMode(conn, message, contentSize);
      }
      else if('b' == *message)
      {
        ResponseSetPlayMode(conn, message, contentSize);
      }
      else
      {
        fprintf(stderr, "%s\n%s\n%d\n%c", buf, message, contentSize, *message);
      }
    }
  }
}

void*
KeyPadThread(
  void *p
)
{
  struct input_event event;
  unsigned char flag = 0;
  int in = -1;
  for(;;)
  {
    if (-1 == in)
    {
      in = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
      fcntl(in, F_SETFL, 0);
      continue;
    }
    if (read(in, &event, sizeof(event)) != sizeof(event))
    {
      close(in);
      in = -1;
      continue;
    }
    switch(event.type)
    {
      case EV_KEY:
        if (0 == event.value)
        {
          /* < Release > */
          switch(event.code)
          {
            case KEY_KPENTER:
              flag = 0;
              break;
            default:
              break;
          }
        }
        else if (1 == event.value)
        {
          /* < Press > */
          switch(event.code)
          {
            case KEY_KP0:
              if (1 == flag)
              {
                StopVideoService();
              }
              break;
            case KEY_KP2:
              DownVolume();
              break;
            case KEY_KP4:
              RWVideo();
              break;
            case KEY_KP5:
              PauseVideoService();
              break;
            case KEY_KP6:
              FFVideo();
              break;
            case KEY_KP8:
              UpVolume();
              break;
            case KEY_KP9:
              if (1 == flag)
              {
                ChangeAudioConfig();
              }
              break;
            case KEY_KPMINUS:
              if (1 == flag)
              {
                RestartService();
              }
              break;
            case KEY_KPPLUS:
              if (1 == flag)
              {
                UpdateSongService();
              }
              break;
            case KEY_KPSLASH:
              if (1 == flag)
              {
                playMode ^= 1;
              }
              break;
            case KEY_KPASTERISK:
              if (1 == flag)
              {
                ShutdownService();
              }
              break;
            case KEY_KPENTER:
              flag = 1;
              break;
            case KEY_KPDOT:
              if (1 == flag)
              {
                ChangeAudioTrack();
              }
              break;
            default:
              break;
          }
        }
        else if (2 == event.value)
        {
          /* < Repeat > */
        }
        break;
      default:
        break;
    }
  }
}

void*
PlayThread(
  void *p
)
{
  unsigned long bookIdList[PAGERECORDS];
  unsigned short userIdList[PAGERECORDS];
  char pathList[PAGERECORDS][PATH_LENGTH_MAX];
  char path[PATH_LENGTH_MAX];
  char commentList[PAGERECORDS][COMMENT_LENGTH_MAX];
  unsigned long durationList[PAGERECORDS];
  unsigned char record = 0;
  unsigned char currentTime[32];
  unsigned char audioTrack = 0;
  unsigned char option[32];
  unsigned long fileCount = 0;
  AVFormatContext *formatContext = NULL;
  unsigned long fileId = 0;
  playerState = 0;
  InitPlayer(audioOutput, &playTime);
  for(;;)
  {
    LockBook();
    record = GetBookList(bookIdList, userIdList, (char*)pathList, (char*)commentList, durationList);
    UnlockBook();
    LockPlayer();
    if (0 != record && 0 == playerState)
    {
      if (!strcmp(pathList[0], "Break"))
      {
        playerState = 3;
        LockBook();
        DeleteBook(bookIdList[0]);
        UnlockBook();
        UnlockPlayer();
        continue;
      }
      sleep(3);
      LockBook();
      GetBookPath(bookIdList[0], path);
      audioTrack = GetBookAudio(bookIdList[0]);
      UnlockBook();
      OpenVideo(path, audioTrack);
      SetPlayingPath(pathList[0]);
      strcpy(GetSingerComment(), commentList[0]);
      playDuration = durationList[0];
      LockUser();
      GetUserName(userIdList[0], GetSinger());
      UnlockUser();
      time_t t = time(NULL);
      strftime(currentTime, sizeof(currentTime), "%Y/%m/%d %H:%M:%S", localtime(&t));
      LockHistory();
      AddHistory(userIdList[0], path, commentList[0], currentTime);
      UnlockHistory();
      LockBook();
      DeleteBook(bookIdList[0]);
      UnlockBook();
      playerState = 1;
    }
    else if (0 == record && 0 == playerState && 1 == playMode)
    {
      /* < Shuffle Mode > */
      fileCount = GetFileCount();
      if (0 != fileCount)
      {
        fileId = rand() % fileCount;
        GetFilePath(fileId, path);
        OpenVideo(path, 0);
        SetPlayingPath(path);
        avformat_open_input(&formatContext, path, NULL, NULL);
        avformat_find_stream_info(formatContext, NULL);
        for (int i=0; i<(int)formatContext->nb_streams; ++i)
        {
          if (AVMEDIA_TYPE_VIDEO == formatContext->streams[i]->codecpar->codec_type)
          {
            playDuration = formatContext->duration/1000000;
            break;
          }
        }
        avformat_close_input(&formatContext);
        playerState = 1;
      }
    }
    else
    {
      if (0 == IsPlayingVideo())
      {
        if ((1 == playerState) || (2 == playerState) || (4 == playerState) || (5 == playerState))
        {
          playerState = 0;
        }
	StopVideo();
      }
      else
      {
        playTime = GetPosition();
      }
    }
    UnlockPlayer();
    usleep(500000);
  }
}

gboolean
timer_handler(
  void
)
{
  GuiPlayList();
}

void
Exit(
  void
)
{
  exit(0);
}


void*
GuiThread(
  void *p
)
{
  GtkWidget *window = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *topBox = NULL;
  GtkWidget *listTextBox = NULL;
  GtkWidget *hTextBox = NULL;
  GtkWidget *hImageBox = NULL;
  GtkWidget *wifiLabel = NULL;
  GtkWidget *urlLabel = NULL;
  GtkWidget *wifiImage = NULL;
  GtkWidget *urlImage = NULL;
  GtkWidget *quitButton = NULL;
  GtkCssProvider *provider = NULL;
  GtkStyleContext *context = NULL;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  quitButton = gtk_button_new_with_label("X");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  topBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  listTextBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  hTextBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  hImageBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  songLabel = gtk_label_new(NULL);
  userLabel = gtk_label_new(NULL);
  pauseLabel = gtk_label_new(NULL);
  commentLabel = gtk_label_new(NULL);
  provider = gtk_css_provider_new();
  context = gtk_widget_get_style_context(window);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  gtk_widget_set_name(window, "window");
  gtk_css_provider_load_from_data(provider,
    "window {\n"
    " background-color: white;\n"
    "}\n",
    -1, NULL);
  g_object_unref(provider);

  char fileBuf[256];
  char wifiLabelStr[256];
  char urlLabelStr[256];
  char pass[256];
  FILE *file = NULL;
  file = fopen("ip.txt", "r");
  fgets(fileBuf, 256, file);
  fclose(file);
  fileBuf[strlen(fileBuf)-1] = 0;
  sprintf(urlLabelStr, "<span bgcolor=\"#FFFFFF\" size=\"xx-large\">Second, go to the following URL.\nhttp://%s:50000/</span>", fileBuf);
  file = fopen("AccessPointSSID.txt", "r");
  fgets(fileBuf, 256, file);
  fileBuf[strlen(fileBuf)-1] = 0;
  fclose(file);
  file = fopen("AccessPointPass.txt", "r");
  char *result = fgets(pass, 256, file);
  if (NULL == result)
  {
    *pass = 0;
  }
  else
  {
    pass[strlen(pass)-1] = 0;
  }
  fclose(file);
  sprintf(wifiLabelStr, "<span bgcolor=\"#FFFFFF\" size=\"xx-large\">First, connect the following Wi-Fi.\nSSID:%s\n", fileBuf);
  sprintf(&wifiLabelStr[strlen(wifiLabelStr)], "PASS:%s</span>", pass);

  wifiLabel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(wifiLabel), wifiLabelStr);
  urlLabel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(urlLabel), urlLabelStr);
  wifiImage = gtk_image_new_from_file("ssid.png");
  urlImage = gtk_image_new_from_file("url.png");
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_box_pack_start(GTK_BOX(vbox), topBox, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), listTextBox, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hTextBox, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hImageBox, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(topBox), pauseLabel, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(topBox), quitButton, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(listTextBox), userLabel, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(listTextBox), songLabel, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(listTextBox), commentLabel, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(hTextBox), wifiLabel, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(hTextBox), urlLabel, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(hImageBox), wifiImage, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(hImageBox), urlImage, 0, 0, 0);
  gtk_widget_realize(window);
  gtk_window_fullscreen((GtkWindow*)window);
  g_signal_connect(window, "destroy", gtk_main_quit, NULL);
  g_signal_connect_swapped(GTK_BUTTON(quitButton), "clicked", Exit, window);
  gtk_widget_show_all(window);
  g_timeout_add(100, (GSourceFunc)timer_handler, window);
  gtk_main();
}


int main(
  int argc,
  char **argv
)
{
  struct addrinfo hints = { 0 };
  struct addrinfo *res = NULL;
  int sock = 0;
  struct sockaddr_storage addr = { 0 };
  socklen_t addrlen = sizeof(addr);
  int conn = 0;
  int yes = 1;
  unsigned char *p;
  unsigned long contentLength;
  FILE *file = NULL;
  char dir[PATH_LENGTH_MAX];
  char pass[128];
  char command[256];
  char *result = NULL;

  signal(SIGPIPE, SIG_IGN);

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, "50000", &hints, &res);

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (-1 == sock)
  {
    fprintf(stderr, "Error socket()\n");
    return 1;
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

  if (-1 == bind(sock, res->ai_addr, res->ai_addrlen))
  {
    fprintf(stderr, "Error bind()\n");
    return 2;
  }
 
  if (-1 == listen(sock, 256))
  {
    fprintf(stderr, "Error listen()\n");
    return 3;
  } 

  freeaddrinfo(res);

  srand((unsigned int)time(NULL));

  InitFile();
  InitHistory();
  OpenUser();
  OpenFile();
  strcpy(dir, "/media/pi");
  WalkDir(dir, strlen("/media/pi"));
  OpenBook();
  OpenFavorite();
  OpenHistory();

  pthread_t playThread;
  pthread_create(&playThread, NULL, PlayThread, NULL);
  pthread_detach(playThread);

  pthread_t keypadThread;
  pthread_create(&keypadThread, NULL, KeyPadThread, NULL);
  pthread_detach(keypadThread);

  system("sh ip.sh > ip.txt");
  system("sh AccessPointSSID.sh > AccessPointSSID.txt");
  system("sh url.sh > url.txt");
  system("sh AccessPointPass.sh > AccessPointPass.txt");
  file = fopen("AccessPointPass.txt", "r");
  result = fgets(pass, 64, file);
  fclose(file);
  if (NULL == result)
  {
    system("sh QRCodeAccessPointNoPass.sh > QRCodeAccessPointNoPass.txt");
    system("qrencode -r QRCodeAccessPointNoPass.txt -d 400 -l H -s 7 -o ssid.png");
  }
  else
  {
    pass[strlen(pass) - 1] = 0;
    sprintf(command, "sh QRCodeAccessPointWithPass.sh %s > QRCodeAccessPointWithPass.txt", pass);
    system(command);
    system("qrencode -r QRCodeAccessPointWithPass.txt -d 400 -l H -s 7 -o ssid.png");
  }
  system("qrencode -r url.txt -d 400 -l H -s 7 -o url.png");

  pthread_t guiThread;
  gtk_init(&argc, &argv);
  pthread_create(&guiThread, NULL, GuiThread, NULL);
  pthread_detach(guiThread);

  //system("chromium-browser --noerrdialogs --kiosk --incognito --no-default-browser-check --no-sandbox --test-type http://localhost:50000/view.html 2> /dev/null  > /dev/null &");

  for(;;)
  {
    conn = accept(sock, (struct sockaddr*)&addr, &addrlen);
    pthread_t thread;
    pthread_create(&thread, NULL, HttpThread, (void*)conn);
    pthread_detach(thread);
  }
}
