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
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <vlc/vlc.h>

/* Receive Buffer Max Size */
#define RECV_MAX 16
/* Fadeout Volume Step */
#define FADE_STEP 5
/* Fadeout Count */
#define FADE_COUNT 15
/* Fadeout Wait */
#define FADE_WAIT 200000
/* Reply Normal */
#define REPLY_NORMAL '1'
/* Reply Stop */
#define REPLY_STOP '0'
/* Return OK */
#define RET_OK 0
/* Return STOP */
#define RET_STOP 1
/* Volume Step */
#define VOL_STEP 5
/* Seek Step */
#define SEEK_STEP 5000

typedef unsigned char (*Func)(int);

unsigned char GetPosition(int);
unsigned char FadeoutVolume(int);
unsigned char StopVideo(int);
unsigned char PauseVideo(int);
unsigned char RWVideo(int);
unsigned char FFVideo(int);
unsigned char DownVolume(int);
unsigned char UpVolume(int);
unsigned char ChangeAudioTrack(int);
unsigned char IsPlayingVideo(int);

/* Path */
static char* path = NULL;
/* Audio Track */
static unsigned char audioTrack = 0;
/* Volume */
static int volume = 0;
/* Receive Buffer */
static unsigned char buffer[RECV_MAX] = {0};
/* Request Command Function Table */
static Func funcTable[] = {
  GetPosition, 
  FadeoutVolume, 
  StopVideo, 
  PauseVideo, 
  RWVideo, 
  FFVideo, 
  DownVolume, 
  UpVolume, 
  ChangeAudioTrack, 
  IsPlayingVideo
};
/* Instance */
static libvlc_instance_t* instance = NULL;
/* Player */
static libvlc_media_player_t* player = NULL;
/* Media */
static libvlc_media_t* media = NULL;

unsigned char /* Status */
GetPosition(
  int fd /* Descripter */
)
{
  /* [[[ 1. Get Position ]]] */
  long long playTime = libvlc_media_player_get_time(player);

  /* [[[ 2. Reply ]]] */
  write(fd, &playTime, sizeof(long long));
  return RET_OK;
}

unsigned char /* Status */
FadeoutVolume(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Fade Count */
  int count = 0;
  /* Current Volume */
  int vol = 0;
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Fadeout Volume ]]] */
  for (count=0; count<FADE_COUNT; ++count)
  {
    /* [[ 1.1. Get Volume ]] */
    vol = libvlc_audio_get_volume(player);
    /* [[ 1.2. Calcurate Volume ]] */
    vol -= FADE_STEP;
    /* [[ 1.3. Set Volume ]] */
    libvlc_audio_set_volume(player, vol);
    /* [[ 1.4. Wait ]] */
    usleep(FADE_WAIT);
  }

  /* [[[ 2. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
StopVideo(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_STOP;

  /* [[[ 1. Stop Video ]]] */
  libvlc_media_player_stop(player);

  /* [[[ 2. Reply ]]] */
  write(fd, &c, 1);
  return RET_STOP;
}

unsigned char /* Status */
PauseVideo(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Pause Video ]]] */
  libvlc_media_player_pause(player);

  /* [[[ 2. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
RWVideo(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Get Position ]]] */
  long long playTime = libvlc_media_player_get_time(player);

  /* [[[ 2. Calcurate Position ]]] */
  playTime -= SEEK_STEP;
  if (playTime < 0)
  {
    playTime = 0;
  }

  /* [[[ 3. Seek ]]] */
  libvlc_media_player_set_time(player, playTime);

  /* [[[ 4. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
FFVideo(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Get Position ]]] */
  long long playTime = libvlc_media_player_get_time(player);

  /* [[[ 2. Calcurate Position ]]] */
  playTime += SEEK_STEP;

  /* [[[ 3. Seek ]]] */
  libvlc_media_player_set_time(player, playTime);

  /* [[[ 4. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
DownVolume(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Calcurate Volume ]]] */
  volume -= VOL_STEP;

  /* [[[ 2. Set Volume ]]] */
  libvlc_audio_set_volume(player, volume);

  /* [[[ 3. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
UpVolume(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Calcureate Volume ]]] */
  volume += VOL_STEP;

  /* [[[ 2. Set Volume ]]] */
  libvlc_audio_set_volume(player, volume);

  /* [[[ 3. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
ChangeAudioTrack(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;
  /* Max Audio Track */
  int count = 0;
  /* Current Audio Track */
  int current = 0;

  /* [[[ 1. Get Max Audio Track ]]] */
  count = libvlc_audio_get_track_count(player);

  if (1 != count)
  {
    /* < Multiple Audio Track > */
    /* [[[ 2. Change Audio Track ]]] */
    /* [[ 2.1. Get Current Audio Track ]] */
    current = libvlc_audio_get_track(player);
    /* [[ 2.2. Calcurate Next Audio Track ]] */
    current++;
    if (current == count)
    {
      current = 1;
    }
    /*  [[ 2.3. Set Next Audio Track ]] */
    libvlc_audio_set_track(player, current);
  }

  /* [[[ 3. Reply ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

unsigned char /* Status */
IsPlayingVideo(
  int fd /* Descripter */
)
{
  /* [[[ 0. Var ]]] */
  /* Reply */
  char c = REPLY_NORMAL;

  /* [[[ 1. Get Player State ]]] */
  if (libvlc_Ended == libvlc_media_player_get_state(player))
  {
    /* < End > */
    /* [[[ 2. Reply End ]]] */
    c = REPLY_STOP;
    write(fd, &c, 1);
    return RET_STOP;
  }
  /* [[[ 3. Reply Play ]]] */
  write(fd, &c, 1);
  return RET_OK;
}

int /* Listen Socket */
Listen(void)
{
  /* [[[ 0. Var ]]] */
  /* Socket */
  int sock = 0;
  /* Address */
  struct sockaddr_un sa = {0};
  /* Return Sub-Routines */
  int res = 0;

  /* [[[ 1. Create Socket ]]] */
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == sock)
  {
    /* < Failure > */
    fprintf(stderr, "create socket error\n");
    return -1;
  }

  /* [[[ 2. Bind Socket ]]] */
  /* [[ 2.1. Set Address ]] */
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, "/tmp/vlcwrapper");
  /* [[ 2.2. Remove Old File ]]*/
  remove(sa.sun_path);
  /* [[ 2.3. Bind Address to Socket ]] */
  res = bind(sock, (struct sockaddr*) &sa, sizeof(struct sockaddr_un));
  if (0 != res)
  {
    /* < Failure > */
    fprintf(stderr, "bind socket error\n");
    close(sock);
    return -1;
  }

  /* [[[ 3. Listen ]]] */
  res = listen(sock, 128);
  if (0 != res)
  {
    /* < Failure > */
    fprintf(stderr, "listen socket error\n");
    close(sock);
    return -1;
  }

  /* < Success > */
  return sock;
}

int /* Accepted Descripter */
Accept(
  int sock /* Listen Socket */
)
{
  /* [[[ 1. Accept Connection ]]] */
  return accept(sock, NULL, NULL);
}

void
Close(
  int handle /* Descripter or Socket */
)
{
  close(handle);
}

int /* Received Size */
Recv(
  int fd, /* Descripter */
  unsigned char* buffer, /* Buffer */
  size_t size /* Buffer Max Size */
)
{
  /* [[[ 1. Read From Descripter To Buffer ]]] */
  return read(fd, buffer, size-1);
}

unsigned char /* Status */
Parse(
  int fd, /* Descripter */
  unsigned char cmd /* Request Command */
)
{
  /* [[[ 1. Exec Command ]]] */
  return funcTable[cmd - 'A'](fd);
}

void
OpenVideo(
  void
)
{
  /* [[[ 0. Var ]]] */
  /* Command Line Option */
  unsigned char option[32] = {0};
  /* Instance Option */
  const char *arg[] = {"--vout", "mmal_vout"};

  /* [[[ 1. Init ]]] */
  /* [[ 1.1. Instance ]] */
  instance = libvlc_new(2, arg);
  /* [[ 1.2. Player ]] */
  player = libvlc_media_player_new(instance);
  
  /* [[[ 2. Open ]]] */
  /* [[ 2.1. Media ]] */
  media = libvlc_media_new_path(instance, path);
  sprintf(option, "audio-track=%d", audioTrack);
  libvlc_media_add_option(media, option);
  /* [[ 2.2. Volume ]] */
  libvlc_audio_set_volume(player, volume);
  /* [[ 2.3. Set Media ]] */
  libvlc_media_player_set_media(player, media);
  /* [[ 2.4. Release Media ]] */
  libvlc_media_release(media);
  /* [[ 2.5. Fullscreen ]] */
  libvlc_set_fullscreen(player, 1);

  /* [[[ 3. Play ]]] */
  libvlc_media_player_play(player);
}

int main(int argc, char **argv)
{
  /* [[[ 0. Var ]]] */
  /* Socket */
  int sock = 0;
  /* Descripter */
  int fd = 0;
  /* Receive Size */
  int recvSize = 0;
  /* Status */
  int status = 0;

  /* [[[ 1. Command Line Parameter ]]] */
  /* Path */
  path = argv[1];
  /* Audio Track */
  audioTrack = atoi(argv[2]);
  /* Volume */
  volume = atoi(argv[3]);

  /* [[[ 2. Listen ]]] */
  sock = Listen();
  if (-1 == sock)
  {
    /* < Failure > */
    return 1;
  }

  /* [[[ 3. Accept ]]] */
  fd = Accept(sock);

  /* [[[ 4. Open Video ]]] */
  OpenVideo();

  /* [[[ 5. Request Reply Loop ]]] */
  for (;;)
  {
    recvSize = Recv(fd, buffer, RECV_MAX);
    if (-1 == recvSize)
    {
      /* < Error > */
      fprintf(stderr, "recv error\n");
      goto end;
    }
    else if (0 == recvSize)
    {
      continue;
    }
    for (int count=0; count<recvSize; ++count)
    {
      status = Parse(fd, buffer[count]);
      if (RET_STOP == status)
      {
        goto end;
      }
    }
  } 
end:
  Close(fd);
  Close(sock);
  return 0;
}
