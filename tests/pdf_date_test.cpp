#include "platform/mupdf_compat.h"
#include <stdexcept>
static void check(int year,int month,int day,long long expected) {
  tm value{}; value.tm_year=year-1900; value.tm_mon=month-1; value.tm_mday=day;
  if(static_cast<long long>(vreed_timegm(&value))!=expected)throw std::runtime_error("PDF UTC conversion mismatch");
}
int main() {
  check(1970,1,1,0); check(1969,12,31,-86400);
  check(2000,2,29,951782400); check(2000,3,1,951868800);
  check(1900,3,1,-2203891200LL); check(2026,10,8,1791417600);
}
