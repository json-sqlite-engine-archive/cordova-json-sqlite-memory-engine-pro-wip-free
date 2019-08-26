/* API version to check (pre-stable API version): */
#define JSQL_PRO_API_VERSION -1999999999

/* Export some important sqlite open flags to the Java interface (VFS not supported): */
#define JSQL_OPEN_READONLY      0x00001
#define JSQL_OPEN_READWRITE     0x00002
#define JSQL_OPEN_CREATE        0x00004
#define JSQL_OPEN_URI           0x00040
#define JSQL_OPEN_MEMORY        0x00080
#define JSQL_OPEN_NOMUTEX       0x08000
#define JSQL_OPEN_FULLMUTEX     0x10000
#define JSQL_OPEN_SHAREDCACHE   0x20000
#define JSQL_OPEN_PRIVATECACHE  0x40000

/* some important sqlite result codes to the Java interface: */
#define JSQL_RESULT_OK          0
#define JSQL_RESULT_ERROR       1
#define JSQL_RESULT_INTERNAL    2
#define JSQL_RESULT_PERM        3
#define JSQL_RESULT_ABORT       4
/* TBD ... */
#define JSQL_RESULT_CONSTRAINT  19
#define JSQL_RESULT_MISMATCH    20
#define JSQL_RESULT_MISUSE      21
/* TBD ... */
#define JSQL_RESULT_ROW         100
#define JSQL_RESULT_DONE        101

/* and sqlite datatypes: */
#define JSQL_INTEGER    1
#define JSQL_FLOAT      2
#define JSQL_TEXT       3
#define JSQL_BLOB       4
#define JSQL_NULL       5

/* Could not easily get int64_t from stddef.h for gluegen */
typedef long long jsql_long_t;

/* negative number indicates an error: */
typedef jsql_long_t jsql_handle_t;

/* Check Java/native library match and open database handle */
jsql_handle_t jsql_pro_db_open(int jsql_pro_api_version, const char * filename, int flags);

const char * jsql_pro_execute(jsql_handle_t dbHandle, const char * batch_json, int ll);

void jsql_pro_internal_cleanup(jsql_handle_t dbHandle);
