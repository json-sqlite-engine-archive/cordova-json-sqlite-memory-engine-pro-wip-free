# json-sqlite-memory-engine-wrapper-pro-wip-free

**Author:** Christopher J. Brody <chris@brody.consulting>

**LICENSE:** GPL v3 or commercial

Please contact <sales@brodysoft.com> in case of interest in a commercial license.

## About

A wrapper package that supports SQLite memory database operations with statements, arguments, and results in JSON format.

Designed to support Cordova applications on Android, iOS, and macOS ("osx"). May be used to support React Native in the future.

Note that this package does not have any SQLite3 or Android NDK library artifacts committed.

Mininmum Android version supported: Android 5.0

SQLite VFS implementation(s) supported: memory only

FUTURE TODO: new version that supports "nix" VFS, likely with the application responsible for determining the correct directory path

Based on: <https://github.com/brodysoft/Android-sqlite-evcore-native-driver-free>

## Quick usage

### Sample C API usage

```c
jsql_handle_t databaseHandle =
  jsql_pro_db_open(JSQL_PRO_API_VERSION, ":memory:",
    JSQL_OPEN_READWRITE | JSQL_OPEN_CREATE | JSQL_OPEN_MEMORY);

assert(!strcmp(
  jsql_pro_execute(databaseHandle,
  "[null,\"SELECT UPPER(?) as upperText\",1,\"Alice\",null", 0),
  "[{\"rows\":[{\"upperText\":\"ALICE\"}]}]"));

jsql_pro_internal_cleanup(databaseHandle);
```

### Sample usage on Android

```Java
System.loadLibrary("json-sqlite-engine-procore-wip-ndk");

final long databaseHandle =
  JSQLPROCORENDKWrapper.sqlc_pro_db_open(1, ":memory:",
    JSQLPROCORENDKWrapper.SQLC_OPEN_READWRITE |
    JSQLPROCORENDKWrapper.SQLC_OPEN_CREATE |
    JSQLPROCORENDKWrapper.SQLC_OPEN_MEMORY);

final String jsonResult =
  JSQLPROCORENDKWrapper.sqlc_pro_qc_execute(databaseHandle,
    "[null,\"SELECT UPPER(?) as upperText\",1,\"Alice\",null", 0),

Log.v("demo", "JSON result: " + jsonResult);
```

### Sample usage from Cordova

```js
var databaseHandle = null;

window.jsonSQLiteEngine.openDatabaseHandle([':memory:'], function(openResult) {
  databaseHandle = openResult[0];

  window.jsonSQLiteEngine.jsonExecuteStatement([
    databaseHandle,
    JSON.stringify(
      [null, 'SELECT upper(?) AS upperText', 1, 'Test string', null])
  ], function(res) {
    var rs = JSON.parse(res);
    showMessage('received upperText result value (ALL CAPS): ' + rs[0].rows[0].upperText);
  }, function(error) {
    showMessage('SELECT value error: ' + error.message);
  });
});
```

## TODO items

TODO:

* [ ] test program should check for correct results
* [ ] more user-friendly promise-based wrapper for Cordova (JavaScript)
* [ ] demo program that uses the promise-based wrapper on Cordova
* [ ] test the sample usage on Android
* [ ] automatic testing on Android
* [ ] automatic testing on Cordova
* [ ] remove trailing whitespace from generated NDK JNI Java and C code
* [ ] fix internal logging
* [ ] fix error reporting and handling
* [ ] resolve build warnings
* [ ] document the API more formally
* [ ] known issue with double-quotes in keys in result rows
* [ ] consider more sensical package name on iOS & Android
* [ ] update & possibly rename android-ndk-lib/README.md
* [ ] cleanup Android NDK native
* [ ] fix needed for emojis and other 4-byte UTF-8 characters on Android NDK
* [ ] resolve other FUTURE TBD/TODO comments in the source

FUTURE TODO:

* extension functions such as BASE64, REGEXP

FOR FUTURE CONSIDERATION:

* cleaner API
* fix build to generate artifacts in less scattered output directories
* support use of encryption key

## Thanks for guidance

for Cordova plugin:

* https://cordova.apache.org/docs/en/latest/guide/hybrid/plugins/
* https://cordova.apache.org/docs/en/latest/guide/platforms/ios/plugin.html
* https://cordova.apache.org/docs/en/latest/plugin_ref/spec.html
* https://cordova.apache.org/docs/en/latest/guide/platforms/android/plugin.html

INITIAL GUIDANCE for NDK build:

* https://code.tutsplus.com/tutorials/advanced-android-getting-started-with-the-ndk--mobile-2152
* original location: http://mobile.tutsplus.com/tutorials/android/ndk-tutorial/
