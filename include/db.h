#pragma once

/* DB libraries */
#include <sqlite3.h>


/* 1 ----- set up the connection to SQLite DB ----- */
void access_gate_connect_db(sqlite3 **db);

/* 2 ----- set the DB check statement ----- */
void access_gate_set_db_check(sqlite3 *db, sqlite3_stmt **check_statement);

/* 3 -----  query the DB to check if a client is allowed ----- */
const char* access_gate_check_client(sqlite3_stmt *check_statement, const char *ip_address, const char *host);