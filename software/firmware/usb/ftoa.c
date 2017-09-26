#include <assert.h>

#include "ftoa.h"

char *ftoa(char *buffer, double f, int precision)
{
  assert (precision >= 0);
  assert (precision <= 8);

  long p[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};

  char *alphanumeric = buffer;

  long int_equiv = (long) f;
  itoa((long) f, buffer, 10);

  while (*buffer != '\0') {
    buffer++;
  }

  *buffer++ = '.';

  long decimal = abs((long) ((f - int_equiv) * p[precision]));
  itoa(decimal, buffer, 10);

  return alphanumeric;
}
