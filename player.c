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

#include "define.h"
#include "vlc.h"
#include <pthread.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

pthread_mutex_t playerLock;
int vol = 0;
char playPath[PATH_LENGTH_MAX];
char playUser[USER_LENGTH_MAX];
char playComment[COMMENT_LENGTH_MAX];
int player = 1;

char*
GetSingerComment(
  void
)
{
  return playComment;
}

char*
GetSinger(
  void
)
{
  return playUser;
}

void
SetPlayingPath(
  char* path
)
{
  strcpy(playPath, path);
}

char*
GetPlayingPath(
  void
)
{
  return playPath;
}

int
GetVolume(
  void
)
{
  return vol;
}

long long
GetPosition(
  void
)
{
  return VLCGetPosition();
}

void
FadeoutVolume(
  void
)
{
  VLCFadeoutVolume();
}

void
StopVideo(
  void
)
{
  VLCStopVideo();
}

void
PauseVideo(
  void
)
{
  VLCPauseVideo();
}

void
FFVideo(
  void
)
{
  VLCFFVideo();
}

void
RWVideo(
  void
)
{
  VLCRWVideo();
}

void
DownVolume(
  void
)
{
  VLCDownVolume();
}

void
UpVolume(
  void
)
{
  VLCUpVolume();
}

void
ChangeAudioTrack(
  void
)
{
  VLCChangeAudioTrack();
}

void
InitPlayer(
  unsigned char audioOutput,
  long long* time
)
{
  pthread_mutex_init(&playerLock, 0);
  VLCInitPlayer(audioOutput, &vol, time);
}

void
LockPlayer(
  void
)
{
  pthread_mutex_lock(&playerLock);
}

void
UnlockPlayer(
  void
)
{
  pthread_mutex_unlock(&playerLock);
}

void
OpenVideo(
  char *path,
  unsigned char audioTrack
)
{
  unsigned char audioTrackCount = 0;
  AVFormatContext* formatContext = NULL;
  int codecid = 0;
  int width = 0;
  int height = 0;
  if (0 != avformat_open_input(&formatContext, path, NULL, NULL))
  {
    // < Can not open  >
    return;
  }
  if (0 > avformat_find_stream_info(formatContext, NULL))
  {
    // < Can not read stream >
    return;
  }

  for(int i=0; i<(int)formatContext->nb_streams; ++i)
  {
    if (AVMEDIA_TYPE_VIDEO == formatContext->streams[i]->codecpar->codec_type)
    {
      // < Video >
      codecid = formatContext->streams[i]->codecpar->codec_id;
      width = formatContext->streams[i]->codecpar->width;
      height = formatContext->streams[i]->codecpar->height;
    }
    else if (AVMEDIA_TYPE_AUDIO == formatContext->streams[i]->codecpar->codec_type)
    {
      // < Audio >
      audioTrackCount++;
    }
  }
  avformat_close_input(&formatContext);
  player = 1;
  VLCOpenVideo(path, audioTrack);
}

unsigned char
IsPlayingVideo(
  void
)
{ 
  return VLCIsPlayingVideo();
}
