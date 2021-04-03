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

void
AddFile(
  char* path /* file path */
);

void
GetFilePath(
	unsigned long fileId,
	char* path
);

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
);

long /* file size */
GetFileDetail(
	unsigned long fileId, /* file id */
	char* path, /* file path */
	char* modifyTime /* modify time */
);

void
InitFile(
  void
);

void
OpenFile(
  void
);

void
CloseFile(
  void
);

void
ResetFile(
  void
);

void
LockFile(
  void
);

void
UnlockFile(
  void
);

unsigned long
GetFileCount(
  void
);

unsigned long
GetFileRandomId(
  void
);
