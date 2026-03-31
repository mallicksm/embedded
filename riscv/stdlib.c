unsigned long strtoul(const char* nptr, char** endptr, int base) {
   const char* s = nptr;
   unsigned long result = 0;

   // skip spaces
   while (*s == ' ' || *s == '\t')
      s++;

   // auto-detect base
   if (base == 0) {
      if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
         base = 16;
         s += 2;
      } else {
         base = 10;
      }
   }

   while (*s) {
      int digit;

      if (*s >= '0' && *s <= '9')
         digit = *s - '0';
      else if (*s >= 'a' && *s <= 'f')
         digit = *s - 'a' + 10;
      else if (*s >= 'A' && *s <= 'F')
         digit = *s - 'A' + 10;
      else
         break;

      if (digit >= base)
         break;

      result = result * base + digit;
      s++;
   }

   if (endptr)
      *endptr = (char*)s;

   return result;
}
