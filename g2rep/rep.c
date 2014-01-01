
#include	<stdio.h>

#define		iswhite(c)	((c) == ' ' || (c) == '\t')
#define		LEN		4096
#define		TRUE		1
#define		FALSE		0


int indent;
char unit[100];
int second_indent;
int day;
int html;


int show_html = 1;

main(argc, argv)
     int argc;
     char **argv;
{
  FILE *fp;
  int errflag = 0;
  int c;
  extern int optind;

  while ((c = getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      show_html = 2;
      break;

    default:
      errflag++;
    }
  }

  if (optind != argc - 1) {
    fprintf(stderr, "usage: %s [-h] report-file\n", argv[0]);
    exit(1);
  }

  fp = fopen(argv[optind], "r");
  if (fp == NULL) {
    fprintf(stderr, "can't read %s: ", argv[optind]);
    perror("");
    exit(1);
  }

  init_spaces();
  parse_includes(fp);
  fclose(fp);

  exit(0);
}


output(s)
     char *s;
{
  char *p;

  for (p = s; *p; p++)
    if (*p == '~')
      *p = ' ';

  if (day >= 0)
    printf("%2d: %s%s\n", day, unit, s);
  else
    printf("%s%s\n", unit, s);
}


/*
 *  Word wrapper
 */


static char cur[LEN];
static int wrap_pos;
static char *spaces;
static int spaces_len;
static int wrap_output;


init_spaces() {
  int i;
  extern char *malloc();

  spaces_len = 150;
  spaces = malloc(spaces_len + 1);

  if (spaces == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  for (i = 0; i < spaces_len; i++)
    spaces[i] = ' ';

  spaces[spaces_len] = '\0';
}


/*
 *  To use:
 *	wrap_set(who);
 *	wrap(s, ...)			as many as needed
 *	wrap_done();
 */


void
wrap_start() {

  strcpy(cur, &spaces[spaces_len - indent]);
  wrap_output = FALSE;
}


wrap_done_sup() {

  output(cur);
  strcpy(cur, &spaces[spaces_len - indent]);
  wrap_output = TRUE;
}


cur_has_something() {
  char *p;

  for (p = cur; *p && iswhite(*p); p++);

  if (*p)
    return TRUE;
  return FALSE;
}


wrap_done() {

  if (cur_has_something() || !wrap_output)
    wrap_done_sup();
}


char *
word_split(s, pos)
     char *s;
     int pos;
{
  int len;

  len = strlen(s);
  if (pos >= strlen(s))
    return (&s[len]);

  while (pos > 0 && s[pos] != ' ')
    pos--;

  if (pos <= 0)
    return (NULL);

  s[pos] = '\0';

  return (&s[pos + 1]);
}


wrap_append(s, t)
     char *s;
     char *t;
{
  int slen;

  if (strcmp(t, "\"") != 0) {
    slen = strlen(s);

    if (slen > 0 && s[slen - 1] != ' ' && s[slen - 1] != '"') {
      strcat(s, " ");
      slen++;
    }

    if (slen > 1 &&
        (s[slen - 2] == '.'
         || s[slen - 2] == ':' || s[slen - 2] == '?' || s[slen - 2] == '!'))
      strcat(s, " ");
  }

  strcat(s, t);
}


wrap(s)
     char *s;
{
  char *t;
  int len;

  len = strlen(cur);

  if (strlen(s) + len < wrap_pos)
    wrap_append(cur, s);
  else {
    t = word_split(s, wrap_pos - len);
    if (t == NULL && cur_has_something()) {
      wrap_done_sup();
      wrap(s);
    }
    else {
      wrap_append(cur, s);
      wrap_done_sup();
      if (t)
        wrap(t);
    }
  }
}


/*
 *  New:
 *
 *  box-num:html:unit-num:indent::text
 *  box-num:html:unit-num:indent:day:text
 *  box-num:html:unit-num:indent/second-indent:day:text
 *  box-num:html:unit-num:indent/second-indent::text
 */

/*
 *  box-num:unit-num:indent::text
 *  box-num:unit-num:indent:day:text
 *  box-num:unit-num:indent/second-indent:day:text
 *  box-num:unit-num:indent/second-indent::text
 */

char *
strip_stuff(s)
     char *s;
{
  char *p;

/*
 *  Pull off box number
 */

  while (*s && *s != ':')
    s++;
  if (*s == ':')
    s++;

/*
 *  Pull off html style tag
 */

  html = atoi(s);

  while (*s && *s != ':')
    s++;
  if (*s == ':')
    s++;

/*
 *  Pull off unit string
 */

  for (p = unit; *s && *s != ':'; s++)
    *p++ = *s;

  if (p > unit) {
    *p++ = ':';
    *p++ = ' ';
  }
  *p = '\0';

  if (*s == ':')
    s++;

/*
 *  Now get indentation:  %d or %d/%d
 */

  indent = atoi(s);

  while (*s && *s != ':' && *s != '/')
    s++;

  if (*s == '/') {
    s++;
    second_indent = atoi(s);
    while (*s && *s != ':')
      s++;
  }
  else
    second_indent = 0;

  if (*s == ':')
    s++;

/*
 *  Now get day, if any
 */

  if (*s == ':')
    day = -1;
  else
    day = atoi(s);

  while (*s && *s != ':')
    s++;

  if (*s == ':')
    s++;

  return s;
}



/*
 *   regular text { html text | non html text } regular text
 */

#define	LEFT 1
#define	RIGHT 2

char *
dehtml(s)
     char *s;
{
  static char buf[LEN];
  char *bufp;
  char *p;
  int state = 0;

  bufp = buf;
  p = s;

  while (*p) {
    if (*p == '{') {
      state = LEFT;
      p++;
    }

    if (state == LEFT && *p == '|') {
      state = RIGHT;
      p++;
    }

    if (state && *p == '}') {
      state = 0;
      p++;
    }

    if (state == 0 ||
        (state == LEFT && show_html == 2) ||
        (state == RIGHT && show_html == 1))
      *bufp++ = *p;
    p++;
  }

  *bufp = '\0';

  return buf;
}

void
out_line(s)
     char *s;
{
  char *p;

  if (html != 0 && html != show_html)
    return;

  s = dehtml(s);

  for (p = s; *p && *p != '\n'; p++);
  *p = '\0';

  if (html)
    wrap_pos = LEN;
  else if (day >= 0)
    wrap_pos = 72 - strlen(unit);       /* left margin has 2d: on it */
  else
    wrap_pos = 76 - strlen(unit);

  wrap_start();
  indent += second_indent;
  wrap(s);
  wrap_done();
  indent -= second_indent;
}


out_line_include(fp, s)
     FILE *fp;
     char *s;
{

  if (strncmp(s, "#include ", 9) == 0)
    print_by_num(fp, atoi(&s[9]));
  else
    out_line(s);
}


print_by_num(fp, num)
     FILE *fp;
     int num;
{
  char buf[LEN];
  long l;

  l = ftell(fp);
  rewind(fp);

  while (fgets(buf, LEN, fp) != NULL) {
    if (atoi(buf) == num)
      out_line(strip_stuff(buf));
  }

  fseek(fp, l, 0);
}


parse_includes(fp)
     FILE *fp;
{
  char buf[LEN];

  while (fgets(buf, LEN, fp) != NULL) {
    if (atoi(buf) == 1)
      out_line_include(fp, strip_stuff(buf));
  }
}
