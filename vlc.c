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

#include <unistd.h>
#include <vlc/vlc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
static long long* playTime;
static int* volume = 0;
static int sock = 0;

static
unsigned char /* Reply */
SendCmd(
  unsigned char cmd
)
{
  unsigned char buf;
  int res = 0;
  int recvSize = 0;
  if (0 == sock)
  {
    return 0;
  }
  res = write(sock, &cmd, 1);
  if (res < 0)
  {
    close(sock);
    sock = 0;
    return 0;
  }
  recvSize = read(sock, &buf, 1);
  if (-1 == recvSize)
  {
    close(sock);
    sock = 0;
  }
  return buf;
}

long long
VLCGetPosition(
  void
)
{
  unsigned char c = 'A';
  long long playTime;
  int res = 0;
  int recvSize = 0;
  if (0 == sock)
  {
    return 0;
  }
  res = write(sock, &c, 1);
  if (res < 0)
  {
    close(sock);
    sock = 0;
    return 0;
  }
  recvSize = read(sock, &playTime, sizeof(long long));
  if (-1 == recvSize)
  {
    close(sock);
    sock = 0;
    return 0;
  }
  return playTime;
}

void
VLCFadeoutVolume(
  void
)
{
  SendCmd('B');
}

void
VLCStopVideo(
  void
)
{
  SendCmd('C');
}

void
VLCPauseVideo(
  void
)
{
  SendCmd('D');
}

void
VLCRWVideo(
  void
)
{
  SendCmd('E');
}

void
VLCFFVideo(
  void
)
{
  SendCmd('F');
}

void
VLCDownVolume(
  void
)
{
  *volume -= 5;
  if (*volume < 0)
  {
    *volume = 0;
    return;
  }
  SendCmd('G');
}

void
VLCUpVolume(
  void
)
{
  *volume += 5;
  if (100 < *volume)
  {
    *volume = 100;
    return;
  }
  SendCmd('H');
}

void
VLCChangeAudioTrack(
  void
)
{
  SendCmd('I');
}

void
VLCInitPlayer(
  unsigned char audioOutput,
  int* vol,
  long long* time
)
{
  volume = vol;
  playTime = time;
}

void
VLCOpenVideo(
  char *path,
  unsigned char audioTrack
)
{
  unsigned char cmdLine[1024];
  sprintf(cmdLine, "./vlcwrapper \"%s\" ", path);
  sprintf(&cmdLine[strlen(cmdLine)], "%d ", audioTrack);
  sprintf(&cmdLine[strlen(cmdLine)], "%d &", *volume);
  system(cmdLine);
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr_un sa = {0};
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, "/tmp/vlcwrapper");
  while (0 != connect(sock, (struct sockaddr*)&sa, sizeof(struct sockaddr_un)));
}

unsigned char
VLCIsPlayingVideo(
  void
)
{ 
  unsigned char buf = SendCmd('J');
  if ('0' == buf)
  {
    return 0;
  }
  else if ('1' == buf)
  {
    return 1;
  }
}
