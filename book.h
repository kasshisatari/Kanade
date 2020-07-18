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

unsigned char /* records */
GetBookList(
  unsigned long* bookIdList,
  unsigned short* userIdList,
  char* filePathList,
  char* commentList,
  unsigned long* durationList
);

unsigned long /* Book ID */
AddBook(
  unsigned short userId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
);

unsigned long /* Book ID */
InsertBook(
  unsigned short userId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
);

unsigned long /* Book ID */
AddBreak(
  void
);

unsigned long /* Book ID */
InsertBreak(
  void
);

unsigned long
DeleteBook(
  unsigned long bookId
);

unsigned long /* Book ID */
MoveBook(
  unsigned long bookId,
  unsigned char dir
);

unsigned char /* Exist */
HasBook(
  char* path
);

unsigned long /* Book ID */
ModifyBook(
  unsigned long bookId,
  char* path,
  char* comment,
  unsigned long duration,
  unsigned char audioTrack,
  unsigned char visible
);

void
OpenBook(
  void
);

void
CloseBook(
  void
);

void
ResetBook(
  void
);

void
LockBook(
  void
);

void
UnlockBook(
  void
);

void
GetBookPath(
  unsigned long bookId,
  char* path
);

unsigned char
GetBookAudio(
  unsigned long bookId
);

void
GetLatestBook(
  unsigned long *bookId,
  unsigned short *userId,
  char *file,
  char *comment,
  unsigned char *visible
);
