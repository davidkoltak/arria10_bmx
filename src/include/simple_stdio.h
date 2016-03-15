/*
  Simplified library of function calls to provide basic STDIO capabilities
  in a baremetal Altera Arria 10 SoC environment
  
  by  David M. Koltak  02/22/2016

***

Copyright (c) 2016 David M. Koltak

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
  
*/

#ifndef _SIMPLE_STDIO_H
#define _SIMPLE_STDIO_H

void chomp(char *s); // Remove tailing whitespace/newline

void flush(); // Flush stdout (wait until all bytes are sent)

int getchar(void);
char *safe_gets(char *s, int max);
char *gets(char *s);
int putchar(int c);
int puts(char *s);

int printf(char *f, ...);

int sprintf(char *s, char *f, ...);
int sscanf(char *s, char *f, ...);

#endif
