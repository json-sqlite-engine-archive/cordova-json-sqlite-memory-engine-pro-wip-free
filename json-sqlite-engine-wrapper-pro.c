#include "json-sqlite-engine-wrapper-pro.h"

#include <stdbool.h>
#include <stddef.h> /* for NULL */

// TBD iOS vs Android vs ...
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sqlite3.h"

#define BASE_HANDLE_OFFSET 0x100000000LL

// Result of SELECT HEX('JSONSQL')
#define CHECKME_VALUE 0x4A534F4E53514C

// FUTURE TBD ...:
#define LOGV(...) ;
#define LOGE(...) ;

#define HANDLE_FROM_VP(p) ( BASE_HANDLE_OFFSET + ( (unsigned char *)(p) - (unsigned char *)NULL ) )
#define HANDLE_TO_VP(h) (void *)( (unsigned char *)NULL + (ptrdiff_t)((h) - BASE_HANDLE_OFFSET) )

struct qc_s {
  sqlite3 * mydb;
  long long checkme;
  void * vpcleanup1;
};

jsql_handle_t jsql_pro_db_open(int jsql_pro_api_version, const char * name, int flags)
{
  sqlite3 * mydb;
  int r1;
  const char * err;

  LOGV("db_open %s %d", name, flags);

  if (jsql_pro_api_version != JSQL_PRO_API_VERSION) {
    LOGE("API MISMATCH ERROR");
    return -JSQL_RESULT_ERROR;
  }

  if (!!strcmp(name, ":memory:")) {
    LOGE("Non-memory database is currently not supported");
    return -JSQL_RESULT_ERROR;
  }

  r1 = sqlite3_open_v2(name, &mydb, flags, NULL);

  LOGV("db_open %s result %d ptr %p", name, r1, d1);

  if (r1 != 0) return -r1;

#ifdef SQLITE_DBCONFIG_DEFENSIVE // XXX TBD SHOULD NOT BE CONDITIONAL:
  sqlite3_db_config(mydb, SQLITE_DBCONFIG_DEFENSIVE, 1, NULL);
#endif

  {
    struct qc_s * myqc = malloc(sizeof(struct qc_s));

    myqc->mydb = mydb;
    myqc->checkme = CHECKME_VALUE;
    myqc->vpcleanup1 = NULL;

    return HANDLE_FROM_VP(myqc);
  }
}

void jsql_pro_internal_cleanup(jsql_handle_t qc)
{
  struct qc_s * myqc;

  if (qc <= 0) {
    LOGE("ERROR: INVALID qc handle");
    return;
  }

  myqc = HANDLE_TO_VP(qc);

  free(myqc->vpcleanup1);

  myqc->vpcleanup1 = NULL;
}

int sj(const char * j, int tl, char * a)
{
  int ti=0;
  int ai=0;
  while (ti<tl) {
    const uint8_t c = j[ti];
    if (c == '\\') {
      // FUTURE TBD other escaped characters
      switch(j[ti+1]) {
      case '\"':
        a[ai++] = '\"';
        ti += 2;
        break;

      case '\\':
        a[ai++] = '\\';
        ti += 2;
        break;

      case 'r':
        a[ai++] = '\r';
        ti += 2;
        break;

      case 'n':
        a[ai++] = '\n';
        ti += 2;
        break;

      case 't':
        a[ai++] = '\t';
        ti += 2;
        break;

      default:
        // FUTURE TODO what to do??
        ti += 2;
        break;
      }
    } else if (c >= 0xf0) {
      // [WORKAROUND] - NEEDED to avoid garbage character after "?" mark
      // on Android 6.0 and greater
      a[ai++] = '?';
      ti += 4;
    } else if (c >= 0xe0) {
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
    } else if (c >= 0xc0) {
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
    } else if (c >= 128) {
      sprintf(a+ai, "-%02x-", c);
      ai += strlen(a+ai);
      ti += 1;
    } else {
      a[ai++]=j[ti++];
    }
  }

  return ai;
}

const char *jsql_pro_execute(jsql_handle_t qc, const char * batch_json, int ll)
{
  struct qc_s * myqc;

  const char * jp1 = batch_json;
  const char * jp2;
  char jv;

  int r = -1;

  int as = 0;

  char nf[22];
  int nflen = 0;

  sqlite3_stmt * s = NULL;
  int rv = -1;
  int jj, cc;

  int param_count = 0;
  int bi = 0;

  const int FIRST_ALLOC = 10000;
  const int NEXT_ALLOC = 80; // (extra padding)

  char * rr;
  int rrlen = 0;
  int arlen = 0;

  const char * pptext = 0;
  int pplen = 0;

#define EXTRA_ALLOC ((rrlen < 1000000) ? rrlen : (rrlen >> 1))

  if (qc <= 0) {
    LOGE("ERROR: INVALID qc handle");
    return "[\"jsonsqlerror\", \"internal error\", \"extra\"]";
  }

  myqc = HANDLE_TO_VP(qc);

  if (myqc->checkme != CHECKME_VALUE) return "[\"jsonsqlerror\", \"incorrect checkme value\", \"extra\"]";

  {
    sqlite3 * mydb = myqc->mydb;

    const int tc0 = sqlite3_total_changes(mydb);

    // EXTRA NOTE: Use of realloc seems to break under some bulk scenarios.
    // WORKAROUND is to do malloc/memcpy/free instead.
    // TBD VERY strange!

    free(myqc->vpcleanup1);
    myqc->vpcleanup1 = NULL;

    if (*jp1 != '[') return "[\"jsonsqlerror\", \"internal error: json parse error 1\", \"extra\"]";

    ++jp1;

    // dbid - IGNORED
    while ((jv = *jp1) != ',') ++jp1;

    ++jp1;

    myqc->vpcleanup1 = rr = malloc(arlen = FIRST_ALLOC);
    if (rr == NULL) goto batchmemoryerror;

    strcpy(rr, "[");
    rrlen = 1;

    if (*jp1 != '"') return "[\"jsonsqlerror\", \"internal error 2\", \"extra\"]";
    ++jp1;

    jp2 = jp1;
    while ((jv = *jp2) != '"')
      jp2 += (jv == '\\') ? 2 : 1;

    {
      // FUTURE TODO keep buffer & free at the end
      int tl = (jp2-jp1);
      char * a = malloc(tl+100); // extra padding
      int ai = (a == NULL) ? -1 : sj(jp1, tl, a);
      if (a == NULL) goto batchmemoryerror;
      rv = sqlite3_prepare_v2(mydb, a, ai, &s, NULL);
      free(a);
    }

    jp1 = jp2 + 2;

    jp2 = jp1;
    while ((*jp2) != ',') ++jp2;

    nflen = (jp2-jp1);
    strncpy(nf, jp1, nflen);
    nf[nflen] = '\0';
    param_count = atoi(nf);

    jp1 = jp2 + 1;

    if (rv != SQLITE_OK) {
      // SKIP:
      for (bi=1; bi<=param_count; ++bi) {
        jp2 = jp1;
        while ((*jp2) != ',') ++jp2;
        jp1 = jp2 + 1;
      }
    } else {
      for (bi=1; bi<=param_count; ++bi) {
        // FUTURE TBD BLOB
        // NOTE: Only the LAST bind result will be checked:
        jv = *jp1;
        if (jv != '"') {
          jp2 = jp1;
          while ((*jp2) != ',') ++jp2;

          if (jv == 'n') {
            rv = sqlite3_bind_null(s, bi);
          } else if (jv == 't' || jv == 'f') {
            // NOT EXPECTED:
            return "[\"jsonsqlerror\", \"internal error 3\", \"extra\"]";
          } else {
            bool f=false;
            const char * jp3;

            for (jp3=jp1; jp3<jp2; ++jp3) {
              if (*jp3=='.') {
                f = true;
                break;
              }
            }

            nflen = (jp2-jp1);
            strncpy(nf, jp1, nflen);
            nf[nflen] = '\0';

            if (f) {
              rv = sqlite3_bind_double(s, bi, atof(nf));
            } else {
              rv = sqlite3_bind_int64(s, bi, atoll(nf));
            }
          }

          jp1 = jp2 + 1;
        } else {
          // FUTURE TBD keep buffer & free at the end??
          ++jp1;

          jp2 = jp1;
          while ((jv = *jp2) != '"')
            jp2 += (jv == '\\') ? 2 : 1;

          {
            int tl = (jp2-jp1);
            char * a = malloc(tl+100); // extra padding
            int ai = (a==NULL) ? -1 : sj(jp1, tl, a);
            if (a == NULL) goto batchmemoryerror1;
            rv = sqlite3_bind_text(s, bi, a, ai, SQLITE_TRANSIENT);
            free(a);
          }
          jp1 = jp2 + 2;
        }
      }

      if (rv == SQLITE_OK) {
        rv = sqlite3_step(s);
      }

      if (rv == SQLITE_ROW) {
        strcpy(rr+rrlen, "{\"rows\":[");
        rrlen += 9;

        do {
          cc = sqlite3_column_count(s);

          strcpy(rr+rrlen, "{");
          rrlen += 1;

          for (jj=0; jj<cc; ++jj) {
            int ct = sqlite3_column_type(s, jj);

            strcpy(rr+rrlen, "\"");
            rrlen += 1;
            pptext = sqlite3_column_name(s, jj);
            pplen = strlen(pptext);
            if (rrlen + pplen + NEXT_ALLOC > arlen) {
              char * old = rr;
              arlen += EXTRA_ALLOC + pplen + NEXT_ALLOC;
              //myqc->vpcleanup1 = rr = realloc(rr, arlen);
              myqc->vpcleanup1 = rr = malloc(arlen);
              if (rr != NULL) memcpy(rr, old, rrlen);
              free(old);
              if (rr == NULL) goto batchmemoryerror1;
            }
            strcpy(rr+rrlen, pptext);
            rrlen += pplen;
            strcpy(rr+rrlen, "\":");
            rrlen += 2;

            if (ct == SQLITE_NULL) {
              strcpy(rr+rrlen, "null,");
              rrlen += 5;
            } else {
              pptext = sqlite3_column_text(s, jj);
              pplen = strlen(pptext);

              // NOTE: add double pplen for JSON encoding
              // FUTURE TBD add 3x/4x pplen to deal with certain UTF-8 chars
              if (rrlen + pplen + pplen + NEXT_ALLOC > arlen) {
                char * old = rr;
                arlen += EXTRA_ALLOC + pplen + pplen + NEXT_ALLOC;
                //myqc->vpcleanup1 = rr = realloc(rr, arlen);
                myqc->vpcleanup1 = rr = malloc(arlen);
                if (rr != NULL) memcpy(rr, old, rrlen);
                free(old);
                if (rr == NULL) goto batchmemoryerror1;
              }

              if (ct == SQLITE_INTEGER || ct == SQLITE_FLOAT) {
                strcpy(rr+rrlen, pptext);
                rrlen += pplen;
                strcpy(rr+rrlen, ",");
                rrlen += 1;
              } else {
                int pi=0;
                // FUTURE TBD BLOB
                strcpy(rr+rrlen, "\"");
                rrlen += 1;

                while (pi < pplen) {
                  // Use uint8_t (unsigned char) to avoid unwanted conversion
                  // with sign extension:
                  // sqlite3_column_text() returns pointer to unsigned char
                  // (same thing as pointer to uint8_t)
                  // THANKS to @spacepope (Hannes Petersen) for
                  // pointing this one out.
                  const uint8_t pc = pptext[pi];

                  if (pc == '\\') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\\';
                    pi += 1;
                  } else if (pc == '\"') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\"';
                    pi += 1;
                  } else if (pc >= 32 && pc < 127) {
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 0xf0) {
                    // TBD WORKAROUND SOLUTION to avoid crash
                    // in case of 4-byte UTF-8 character issue:
                    rr[rrlen++] = '?';
                    pi += 4;
                  } else if (pc >= 0xe0) {
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 0xc0) {
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 128) {
                    sprintf(rr+rrlen, "?");
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  } else if (pc == '\t') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 't';
                    pi += 1;
                  } else if (pc == '\r') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'r';
                    pi += 1;
                  } else if (pc == '\n') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'n';
                    pi += 1;
                  } else {
                    sprintf(rr+rrlen, "?%02x?", pc);
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  }
                }
                strcpy(rr+rrlen, "\",");
                rrlen += 2;
              }
            }
          }
          if (cc != 0) --rrlen;
          strcpy(rr+rrlen, "}");
          rrlen += 1;
          strcpy(rr+rrlen, ",");
          rrlen += 1;
          rv=sqlite3_step(s);
        } while (rv == SQLITE_ROW);

        --rrlen;
        strcpy(rr+rrlen, "]}]");
        rrlen += 3;
      } else if (rv == SQLITE_OK || rv == SQLITE_DONE) {
        int rowsAffected = sqlite3_total_changes(mydb) - tc0;

        if (rrlen + 200 > arlen) {
          char * old = rr;
          arlen += EXTRA_ALLOC + 200 + 50;
          //myqc->vpcleanup1 = rr = realloc(rr, arlen);
          myqc->vpcleanup1 = rr = malloc(arlen);
          if (rr != NULL) memcpy(rr, old, rrlen);
          free(old);
          if (rr == NULL) goto batchmemoryerror1;
        }

        if (rowsAffected > 0) {
          int insertId = sqlite3_last_insert_rowid(mydb);

          strcpy(rr+rrlen, "{\"rowsAffected\":");
          rrlen += 16;

          sprintf(nf, "%d", rowsAffected);
          strcpy(rr+rrlen, nf);
          rrlen += strlen(nf);

          strcpy(rr+rrlen, ",");
          ++rrlen;

          strcpy(rr+rrlen, "\"insertId\":");
          rrlen += 11;

          sprintf(nf, "%d", insertId);
          strcpy(rr+rrlen, nf);
          rrlen += strlen(nf);
          strcpy(rr+rrlen, "}");
          ++rrlen;
          strcpy(rr+rrlen, "]");
          ++rrlen;
        } else {
          strcpy(rr+rrlen, "null]");
          rrlen += 5;
        }
      }

    }

    if (rv != SQLITE_OK && rv != SQLITE_DONE) {
      const char * em = sqlite3_errmsg(mydb);
      int emlen = strlen(em);
      int pi = 0;

      if (rrlen + emlen + emlen + 100 + NEXT_ALLOC > arlen) {
        char * old = rr;
        arlen += EXTRA_ALLOC + emlen + emlen + 100;
        //myqc->vpcleanup1 = rr = realloc(rr, arlen);
        myqc->vpcleanup1 = rr = malloc(arlen);
        if (rr != NULL) memcpy(rr, old, rrlen);
        free(old);
        if (rr == NULL) goto batchmemoryerror1;
      }

      // QUICK FIX to closely emulate error code mapping of other platform implementations:
      if (rv == 19)
        strcpy(rr+rrlen, "\"error\",6,null,\"constraint fail error code: 19 message: ");
      else if (rv == 1)
        strcpy(rr+rrlen, "\"error\",5,null,\"syntax error or other error code: 1 message: ");
      else
        sprintf(rr+rrlen, "\"error\",0,null,\"other error code: %d message: ", rv);
      rrlen += strlen(rr+rrlen);


        {
            {
                int pi=0;
                const char * pptext = em;
                const int pplen = emlen;

                while (pi < pplen) {
                  // Use uint8_t (unsigned char) to avoid unwanted conversion
                  // with sign extension:
                  // sqlite3_column_text() returns pointer to unsigned char
                  // (same thing as pointer to uint8_t)
                  // THANKS to @spacepope (Hannes Petersen) for
                  // pointing this one out.
                  const uint8_t pc = pptext[pi];

                  if (pc == '\\') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\\';
                    pi += 1;
                  } else if (pc == '\"') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\"';
                    pi += 1;
                  } else if (pc >= 32 && pc < 127) {
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc == '\t') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 't';
                    pi += 1;
                  } else if (pc == '\r') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'r';
                    pi += 1;
                  } else if (pc == '\n') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'n';
                    pi += 1;
                  } else {
                    sprintf(rr+rrlen, "?%02x?", pc);
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  }
                }
                strcpy(rr+rrlen, "\",");
                rrlen += 2;
            }
        }
    }

    // FUTURE TODO what to do in case this returns an error
    sqlite3_finalize(s);

    return rr;
  }

batchmemoryerror1:
  // FUTURE TODO what to do in case this returns an error
  sqlite3_finalize(s);

batchmemoryerror:
  free(myqc->vpcleanup1);
  myqc->vpcleanup1 = NULL;

  return "[\"jsonsqlerror\", \"memory error\", \"extra\"]";
}
