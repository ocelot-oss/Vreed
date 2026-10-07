#include "mupdf_compat.h"
#include <stdint.h>
// Gregorian UTC conversion without changing the process timezone.
time_t vreed_timegm(struct tm* value) {
  int year=value->tm_year+1900,month=value->tm_mon;
  year+=month/12; month%=12;
  if(month<0) {month+=12; --year;}
  if(year<1||year>9999)return (time_t)-1;
  const int64_t previous=year-1;
  int64_t days=365*previous+previous/4-previous/100+previous/400-719162;
  static const int starts[]={0,31,59,90,120,151,181,212,243,273,304,334};
  days+=starts[month]+value->tm_mday-1;
  if(month>1&&year%4==0&&(year%100!=0||year%400==0))++days;
  const int64_t seconds=days*86400+(int64_t)value->tm_hour*3600+value->tm_min*60+value->tm_sec;
  const time_t result=(time_t)seconds;
  return (int64_t)result==seconds?result:(time_t)-1;
}
