/* library implementation */
#include "db.h"

/* standard library */
#include <stdio.h>
#include <stdlib.h>


/* 1 ----- set up the connection to SQLite DB ----- */
void access_gate_connect_db(sqlite3 **db) {

    /* open DB */
    if (sqlite3_open("/data/access.db", db) != SQLITE_OK) {
        fprintf(stderr, "[ERROR] !!! [DB CONNECTION] : %s\n", sqlite3_errmsg(*db));
        exit(EXIT_FAILURE);
    }
    sqlite3_busy_timeout(*db, 1000);

    /* create Table if not existing */
    char *error_message;
    const char *sql =
        "CREATE TABLE IF NOT EXISTS Access ("
        "  ip TEXT NOT NULL,"
        "  service TEXT NOT NULL,"
        "  PRIMARY KEY (ip, service)"
        ");";
    if (sqlite3_exec(*db, sql, NULL, NULL, &error_message) != SQLITE_OK) {
        fprintf(stderr, "[ERROR] !!! [DB CONNECTION] : %s\n", error_message);
        sqlite3_free(error_message);
        exit(EXIT_FAILURE);
    } else return;

}


/* 2 ----- set the DB check statement ----- */
void access_gate_set_db_check(sqlite3 *db, sqlite3_stmt **check_statement) {

    /* set up check statement */
    if (
        sqlite3_prepare_v2(
            db, "SELECT 1 FROM Access WHERE ip = ? AND service = ?",
            -1, check_statement, NULL
        ) != SQLITE_OK
    ) {
        fprintf(stderr, "[ERROR] !!! [DB CONNECTION] : %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    } else return;

}


/* 3 -----  query the DB to check if a client is allowed ----- */
const char* access_gate_check_client(sqlite3_stmt *check_statement, const char *ip_address, const char *host) {

    /* bind request parameters */
    sqlite3_reset(check_statement);
    sqlite3_bind_text(check_statement, 1, ip_address, -1, SQLITE_STATIC);
    sqlite3_bind_text(check_statement, 2, host, -1, SQLITE_STATIC);

    /* query the DB */
    int step_result = sqlite3_step(check_statement);
    char *result = NULL;
    if (step_result == SQLITE_ROW) result = "ACCESS ALLOWED";
    else if (step_result == SQLITE_DONE) result = "ACCESS DENIED";
    else result =  "QUERY ERROR";

    /* free DB connection */
    sqlite3_reset(check_statement);
    return result;

}