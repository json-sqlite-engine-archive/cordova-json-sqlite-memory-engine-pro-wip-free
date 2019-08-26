window['jsonSQLiteEngine'] = {
  //* FUTURE TBD ???:
  //* echo: function(x, cb) {
  //*   cb(x);
  //* },
  openDatabaseHandle: function(args, res, reject) {
    cordova.exec(res, reject, 'SP', 'openDatabaseHandle', [].concat(args));
  },
  jsonExecuteStatement: function(args, res, reject) {
    cordova.exec(res, reject, 'SP', 'jsonExecuteStatement', [].concat(args));
  },
  //* FUTURE TBD/TODO SUPPORT CLOSE DATABASE OPERATION
};
