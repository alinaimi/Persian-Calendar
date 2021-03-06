// Taghveem.cpp : Defines the entry point for the DLL application.
#include "contentplug.h"
#include "math.h"
#include "stdafx.h"
#include "time.h"

#define round(x) ((x) >= 0 ? floor((x)-0.5) : floor((x) + 0.5))
#define div(x, y) ((x) / (y))

#define _detectstring ""

#define fieldcount 4

SYSTEMTIME jalali_to_gregorian(const SYSTEMTIME &);
SYSTEMTIME gregorian_to_jalali(const SYSTEMTIME &);

char *fieldnames[fieldcount] = {"Text Tarikh", "creationdate", "writedate",
                                "accessdate"};

int shYear, shMonth, shDay;
int tmp;
BOOL GetValueAborted = false;

int fieldtypes[fieldcount] = {ft_string, ft_date, ft_date, ft_date};

char *fieldunits_and_multiplechoicestrings[fieldcount] = {
    "چهارشنبه 26 آبان 1361|1361/8/26|26 آبان 1361|26 آبان", "", "", ""};

char *multiplechoicevalues[3] = {"file", "folder", "reparse point"};

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  return TRUE;
}

char *strlcpy(char *p, const char *p2, int maxlen) {
  if ((int)strlen(p2) >= maxlen) {
    strncpy(p, p2, maxlen);
    p[maxlen] = 0;
  } else
    strcpy(p, p2);
  return p;
}

int __stdcall ContentGetDetectString(char *DetectString, int maxlen) {
  strlcpy(DetectString, _detectstring, maxlen);
  return 0;
}

int __stdcall ContentGetSupportedField(int FieldIndex, char *FieldName,
                                       char *Units, int maxlen) {
  if (FieldIndex < 0 || FieldIndex >= fieldcount)
    return ft_nomorefields;
  strlcpy(FieldName, fieldnames[FieldIndex], maxlen - 1);
  strlcpy(Units, fieldunits_and_multiplechoicestrings[FieldIndex], maxlen - 1);
  return fieldtypes[FieldIndex];
}

int __stdcall ContentGetValue(char *FileName, int FieldIndex, int UnitIndex,
                              void *FieldValue, int maxlen, int flags) {
  WIN32_FIND_DATA fd;
  FILETIME lt;
  SYSTEMTIME st;
  SYSTEMTIME st_tmp;
  HANDLE fh;
  DWORD handle;
  DWORD dwSize;
  __int64 filesize;
  GetValueAborted = false;

  if (flags & CONTENT_DELAYIFSLOW) {
    if (FieldIndex == 16)
      return ft_delayed;
    if (FieldIndex == 17)
      return ft_ondemand;
  }

  LPSYSTEMTIME st;

  fh = FindFirstFile(FileName, &fd);
  if (fh != INVALID_HANDLE_VALUE) {
    FindClose(fh);

    switch (FieldIndex) {
    case 0: //	"name"
      strlcpy((char *)FieldValue, "چهارشنبه 26 آبان 1366", maxlen - 1);
      break;

    case 1: // "LastAccessdate"
      FileTimeToLocalFileTime(&fd.ftLastAccessTime, &lt);
      FileTimeToSystemTime(&lt, &st);

      st_tmp = gregorian_to_jalali(st);

      ((pdateformat)FieldValue)->wYear = st_tmp.wYear;
      ((pdateformat)FieldValue)->wMonth = st_tmp.wMonth;
      ((pdateformat)FieldValue)->wDay = st_tmp.wDay;
      break;

    case 2: // "creationdate"
      FileTimeToLocalFileTime(&fd.ftLastWriteTime, &lt);
      FileTimeToSystemTime(&lt, &st);

      st_tmp = gregorian_to_jalali(st);

      ((pdateformat)FieldValue)->wYear = st_tmp.wYear;
      ((pdateformat)FieldValue)->wMonth = st_tmp.wMonth;
      ((pdateformat)FieldValue)->wDay = st_tmp.wDay;
      break;
    case 3: // "writedate",
      FileTimeToLocalFileTime(&fd.ftLastWriteTime, &lt);
      FileTimeToSystemTime(&lt, &st);

      st_tmp = gregorian_to_jalali(st);

      ((pdateformat)FieldValue)->wYear = st_tmp.wYear;
      ((pdateformat)FieldValue)->wMonth = st_tmp.wMonth;
      ((pdateformat)FieldValue)->wDay = st_tmp.wDay;
      break;

    default:
      return ft_nosuchfield;
    }
  } else
    return ft_fileerror;
  return fieldtypes[FieldIndex]; // very important!
}

int __stdcall ContentGetValueW(wchar_t *FileName, int FieldIndex, int,
                               void *FieldValue, int, int) {
  DWORD attr = GetFileAttributesW(FileName);
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return ft_fileerror;
  }
  *(BOOL *)FieldValue = attr & attributeConstants[FieldIndex];
  return ft_boolean;
}
void __stdcall ContentSetDefaultParams(ContentDefaultParamStruct *dps) {}

void __stdcall ContentStopGetValue(char *FileName) { GetValueAborted = true; }

int cnv_year(int dateTime) { return dateTime - 621; }

int __stdcall Miladi2Shamsi(int iYear, int iMonth, int iDay) {
  int jdn, jdn2, depoch, cycle, cyear, ycycle, aux1, aux2, yday;
  int epbase, epyear, mdays;
  int PERSIAN_EPOCH = 1948321;

  if (((iYear > 1582) || ((iYear == 1582) && (iMonth > 10)) ||
       ((iYear == 1582) && (iMonth == 10) && (iDay > 14))))
    jdn = ((1461 * (iYear + 4800 + ((iMonth - 14) / 12))) / 4) +
          ((367 * (iMonth - 2 - 12 * (((iMonth - 14) / 12)))) / 12) -
          ((3 * (((iYear + 4900 + ((iMonth - 14) / 12)) / 100))) / 4) + iDay -
          32075;
  else
    jdn = 367 * iYear - ((7 * (iYear + 5001 + ((iMonth - 9) / 7))) / 4) +
          ((275 * iMonth) / 9) + iDay + 1729777;

  epbase = 475 - 474;
  epyear = 474 + (epbase % 2820);
  mdays = (1 - 1) * 31;
  jdn2 = 1 + mdays + (int)(((epyear * 682) - 110) / 2816) + (epyear - 1) * 365 +
         (int)(epbase / 2820) * 1029983 + (PERSIAN_EPOCH - 1);
  depoch = jdn - jdn2;
  cycle = (int)(depoch / 1029983);
  cyear = depoch % 1029983;

  if (cyear == 1029982)
    ycycle = 2820;
  else {
    aux1 = (int)(cyear / 366);
    aux2 = cyear % 366;
    ycycle = (int)abs(((2134 * aux1) + (2816 * aux2) + 2815) / 1028522 + 0.5);
    ycycle = ycycle + aux1 + 1;
  }
  iYear = ycycle + (2820 * cycle) + 474;
  if (iYear <= 0)
    iYear = iYear - 1;
  if (iYear >= 0)
    epbase = iYear - 474;
  else
    epbase = iYear - 473;
  epyear = 474 + (epbase % 2820);
  mdays = (1 - 1) * 31;
  jdn2 = 1 + mdays + (int)(((epyear * 682) - 110) / 2816) + (epyear - 1) * 365 +
         (int)(epbase / 2820) * 1029983 + (PERSIAN_EPOCH - 1);
  yday = (jdn - jdn2) + 1;

  if (yday <= 186) {
    iMonth = -Sign2(yday / 31) * round(-abs(yday / 31));
    iMonth = round2((yday / 31) + 0.4999999999);
    dbl = floor((yday / 31) + 0.99999 + 1);
    iMonth = (int)dbl;
    iMonth = round((yday / 31) + 0.499 + 1);
    if (inDay > 30) {
      inDay = inDay - 30;
      inMonth++;
    }

    tmp = 0;
    iMonth = 1;
    for (int a = 1; a <= yday; a += 1) {
      tmp += 1;
      if (tmp >= 32) {
        iMonth++;
        tmp = 0;
      }
    }
  } else {
    iMonth = -Sign2(((yday - 6) / 30)) * round(-abs((yday - 6) / 30));
    iMonth = round2(((yday - 6) / 30) + 0.4999999999);
    iMonth = (int)floor((yday - 6) / 30 + 1);
    iMonth = round((yday - 6) / 30 + 0.499 + 1);
    tmp = 0;
    iMonth = 1;
    for (int a = 1; a <= yday; a += 1) {
      tmp += 1;
      if (tmp >= 31) {
        iMonth++;
        tmp = 0;
      }
    }
  }

  if (iYear >= 0)
    epbase = iYear - 474;
  else
    epbase = iYear - 473;
  epyear = 474 + (epbase % 2820);

  if (iMonth <= 7)
    mdays = (iMonth - 1) * 31;
  else
    mdays = (iMonth - 1) * 30 + 6;
  jdn2 = 1 + mdays + (int)(((epyear * 682) - 110) / 2816) + (epyear - 1) * 365 +
         (int)(epbase / 2820) * 1029983 + (PERSIAN_EPOCH - 1);
  iDay = (jdn - jdn2) + 1;

  st.wYear = iYear;
  st.wMonth = iMonth;
  st.wDay = iDay;

  shYear = iYear;
  shMonth = iMonth;
  shDay = iDay;

  shYear = yday;
  shMonth = floor(3.9);
  shYear = round2(12.50);
  return inttostr(iYear) + "/" + inttostr(iMonth) + "/" + inttostr(iDay);
  return round2(12.50);

  return iMonth;
}

char *__stdcall ShamsiText(int shYearT, int shMonthT, int shDayT,
                           int wDayOfWeekT) {
  switch (wDayOfWeekT) {
  case 1:
    return "شنبه";
    break;

  case 2:
    return "دوشنبه";
    break;

  case 3:
    return "دوشنبه";
    break;

  case 4:
    return "دوشنبه";
    break;

  case 5:
    return "دوشنبه";
    break;

  case 6:
    return "دوشنبه";
    break;

  case 7:
    return "دوشنبه";
    break;

  default:
    return "";
  }
}

SYSTEMTIME jalali_to_gregorian(const SYSTEMTIME &j) {
  int i, g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

  int jy = j.wYear - 979, jm = j.wMonth - 1, jd = j.wDay - 1,
      j_day_no = 365 * jy + div(jy, 33) * 8 + div(jy % 33 + 3, 4);
  for (i = 0; i < jm; ++i)
    j_day_no += j_days_in_month[i];

  j_day_no += jd;

  int g_day_no = j_day_no + 79;
  int gy = 1600 + 400 * div(g_day_no, 146097);
  g_day_no = g_day_no % 146097;

  bool leap = true;
  if (g_day_no >= 36525) /* 36525 = 365*100 + 100/4 */
  {
    g_day_no--;
    gy += 100 * div(g_day_no, 36524); /* 36524 = 365*100 + 100/4 - 100/100 */
    g_day_no = g_day_no % 36524;

    if (g_day_no >= 365)
      g_day_no++;
    else
      leap = false;
  }

  gy += 4 * div(g_day_no, 1461); /* 1461 = 365*4 + 4/4 */
  g_day_no %= 1461;

  if (g_day_no >= 366) {
    leap = false;

    g_day_no--;
    gy += div(g_day_no, 365);
    g_day_no = g_day_no % 365;
  }

  for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++)
    g_day_no -= g_days_in_month[i] + (i == 1 && leap);
  int gm = i + 1, gd = g_day_no + 1;

  SYSTEMTIME Result;
  Result.wYear = gy;
  Result.wDay = gd;
  Result.wMonth = gm;
  return Result;
}

SYSTEMTIME gregorian_to_jalali(const SYSTEMTIME &g) {
  int i, g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

  int gy = g.wYear - 1600, gm = g.wMonth - 1, gd = g.wDay - 1,

      g_day_no =
          365 * gy + div(gy + 3, 4) - div(gy + 99, 100) + div(gy + 399, 400);

  for (i = 0; i < gm; ++i)
    g_day_no += g_days_in_month[i];
  if (gm > 1 && ((gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0)))
    /* leap and after Feb */
    g_day_no++;
  g_day_no += gd;

  int j_day_no = g_day_no - 79,

      j_np = div(j_day_no, 12053); /* 12053 = 365*33 + 32/4 */
  j_day_no = j_day_no % 12053;
  int jy = 979 + 33 * j_np + 4 * div(j_day_no, 1461); /* 1461 = 365*4 + 4/4 */

  j_day_no %= 1461;

  if (j_day_no >= 366) {
    jy += div(j_day_no - 1, 365);
    j_day_no = (j_day_no - 1) % 365;
  }

  for (i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i)
    j_day_no -= j_days_in_month[i];
  int jm = i + 1, jd = j_day_no + 1;
  SYSTEMTIME Result;
  Result.wYear = jy;
  Result.wMonth = jm;
  Result.wDay = jd;
  return Result;
}
