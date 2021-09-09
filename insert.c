/* insert.c - To insert a row into a table without using SQL.
By: Pedro Izecksohn , izecksohn@yahoo.com
Version: 2021/September/09 04:49
Copyright: This program is free and open source.
           You may modify and distribute copies of this if you grant the user the right to receive this source code.
           For details, see GPL 3.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifndef DBNAME
#error DBNAME is not defined.
#endif

#ifndef ROWMAXLEN
#define ROWMAXLEN 5000
#endif

// On error exits.
char *new_row (void)
{
  char *ret = malloc (2);
  if (!ret) exit (EXIT_FAILURE);
  time_t seconds_since_epoch = time (NULL);
  struct tm * tmdt = localtime (&seconds_since_epoch);
  unsigned long int date_time = ((((((unsigned long int)tmdt->tm_year+1900)*100+tmdt->tm_mon+1)*100+tmdt->tm_mday)*100+tmdt->tm_hour)*100+tmdt->tm_min)*100+tmdt->tm_sec;
  sprintf (ret, "%lu\"", date_time);
  //printf ("ret=%s\n", ret);
  //printf ("tmdt->tm_year = %u\n", tmdt->tm_year);
  return ret;
}

/*
  Checks the size of the row.
  On error exits.
*/
char *row_append (char * const row, const char c)
{
  if (!row) exit (EXIT_FAILURE);
  if (!c) exit (EXIT_FAILURE);
  size_t delta = 1;
  if ((c=='"')||(c=='\\')) delta = 2;
  const size_t curlen = strlen (row);
  if ((curlen-15+delta)>=ROWMAXLEN)  // The -15 is for the initial date_time and double-quotes.
  {
    printf ("Content-type: text/html\n\n<html lang=\"en\"><head><meta charset=\"ASCII\"></head><body><center>Your entry is too big. You should write a shorter message.</center></body></html>\n");
    exit (EXIT_SUCCESS);
  }
  char *ret = realloc (row, (curlen+1+delta));
  if (!ret) exit (EXIT_FAILURE);
  if (2==delta)
  {
    ret[curlen]='\\';
    ret[curlen+1]=c;
    ret[curlen+2]=0;
    return ret;
  }
  ret[curlen]=c;
  ret[curlen+1]=0;
  return ret;
}

// On error exits.
char *row_end (char *const row)
{
  if (!row) exit (EXIT_FAILURE);
  const size_t rowcurlen = strlen (row);
  const size_t rowfutlen = rowcurlen+2;
  char *ret = realloc (row, rowfutlen+1);
  if (!ret) exit (EXIT_FAILURE);
  ret[rowcurlen]='"';
  ret[rowcurlen+1]='\n';
  ret[rowfutlen]=0;
  return ret;
}

int main ()
{
  struct stat statbuf;
  FILE *db = fopen (DBNAME, "ab");
  if (!db)
  {
    printf ("Content-type: text/html\n\n<html lang=\"en\"><head><meta charset=\"ASCII\"></head><body><center>I could not create or open the database %s<br>%s</center></body></html>\n", DBNAME, strerror(errno));
    exit (EXIT_SUCCESS);
  }
  if (fstat (fileno(db), &statbuf))
  {
    printf ("Content-type: text/html\n\n<html lang=\"en\"><head><meta charset=\"ASCII\"></head><body><center>I could not fstat the database %s<br>%s</center></body></html>\n", DBNAME, strerror(errno));
    exit (EXIT_SUCCESS);
  }
#ifndef DBSIZEMAX
#define DBSIZEMAX 10000000
#endif
  if (statbuf.st_size >= DBSIZEMAX)
  {
    printf ("Content-type: text/html\n\n<html lang=\"en\"><head><meta charset=\"ASCII\"></head><body><center>The database is full.</center></body></html>\n");
    exit (EXIT_SUCCESS);
  }
  char *row = new_row ();
  while (1)
  {
    const int C = fgetc (stdin);
    if (EOF==C)
    {
      break;
    }
    row = row_append (row, (char)C);
  }
  row = row_end (row);  
  const size_t len = strlen(row);
  const size_t ret = fwrite (row, 1, len, db);
  if (ret!=len) exit (EXIT_FAILURE);
  if (fclose(db)) exit (EXIT_FAILURE);
#ifndef SUCCESS_MESSAGE
#define SUCCESS_MESSAGE "It will be read soon."
#endif
#ifndef LANG
#define LANG "en"
#endif
#ifndef CHARSET
#define CHARSET "ASCII"
#endif
#ifndef TITLE
#define TITLE "Result"
#endif
  printf ("Content-type: text/html\n\n<html lang=\"");
  printf (LANG);
  printf ("\"><head><meta charset=\"");
  printf (CHARSET);
  printf ("\" /><title>");
  printf (TITLE);
  printf ("</title></head><body><center>\n");
  printf (SUCCESS_MESSAGE);
  printf ("</center></body></html>\n");
  return EXIT_SUCCESS;
}
