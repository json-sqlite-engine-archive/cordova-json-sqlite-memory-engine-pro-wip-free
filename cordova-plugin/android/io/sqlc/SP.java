package io.sqlc;

public class SP extends org.apache.cordova.CordovaPlugin {
  static final String ndkLibraryName = "json-sqlite-engine-procore-wip-ndk";

  static boolean isLibLoaded = false;

  public SP() /* throws a RuntimeException in case of an issue with the lib */ {
    if (!isLibLoaded) {
      System.loadLibrary(ndkLibraryName);
      isLibLoaded = true;
    }
  }

  @Override
  public boolean execute(String a, org.json.JSONArray aa, org.apache.cordova.CallbackContext cc) {
    try {
      switch(a) {
        case "openDatabaseHandle":
          // FUTURE TBD get database name from arguments
          final String name = ":memory:";
          final long openDatabaseHandle = JSQLCOREPRONDKWrapper.jsql_pro_db_open(
              JSQLCOREPRONDKWrapper.JSQL_PRO_API_VERSION,
              name,
              JSQLCOREPRONDKWrapper.JSQL_OPEN_READWRITE | JSQLCOREPRONDKWrapper.JSQL_OPEN_CREATE | JSQLCOREPRONDKWrapper.JSQL_OPEN_MEMORY);
          final org.json.JSONArray openresult = new org.json.JSONArray();
          openresult.put(openDatabaseHandle);
          cc.success(openresult);
          return true;
        case "jsonExecuteStatement":
          if (aa.length() < 2) return false;
          final long jsonDatabaseHandle = aa.getLong(0);
          final String js = aa.getString(1);
          final String jr = JSQLCOREPRONDKWrapper.jsql_pro_execute(jsonDatabaseHandle, js, 0);
          final org.json.JSONArray r = new org.json.JSONArray();
          r.put(jr);
          JSQLCOREPRONDKWrapper.jsql_pro_internal_cleanup(jsonDatabaseHandle);
          cc.success(r);
          return true;
      }
    } catch(Exception e) { return false; }
    return false;
  }
}
