#include "json-sqlite-engine-wrapper-pro.h"

#include <stdio.h>

void main() {
  jsql_handle_t databaseHandle =
    jsql_pro_db_open(JSQL_PRO_API_VERSION, ":memory:",
      JSQL_OPEN_READWRITE | JSQL_OPEN_CREATE | JSQL_OPEN_MEMORY);

  printf("result 1: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"SELECT 1\",0,{}]", 0));

  jsql_pro_internal_cleanup(databaseHandle);

  printf("result 2: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"SELECT UPPER('Alice'), 11*11, -123.456\",0,{}]", 0));

#if 0 // TBD SKIP DUE TO KNOWN ISSUE:
  printf("result 3: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"SELECT UPPER(\\\"Anna\\\"), 11*11, -123.456\",0,{}]", 0));
# endif

  printf("result 4: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"SELECT UPPER(?), ?*?, -?\",4,\"Beth\",11,12,123.456,{}]", 0));

  printf("result 5: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"CREATE TABLE TT (FIRST,SECOND)\",0,{}]", 0));

  printf("result 6: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"INSERT INTO TT VALUES(?,?)\",2,\"Alice\",101,{}]", 0));

  printf("result 7: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"INSERT INTO TT VALUES(?,?)\",2,\"Beth\",102,{}]", 0));

  printf("result 8: %s\n",
    jsql_pro_execute(databaseHandle,
      "[{},\"SELECT * FROM TT\",0,{}]", 0));

  // clean finish at this point:
  jsql_pro_internal_cleanup(databaseHandle);
  exit(0);
}
