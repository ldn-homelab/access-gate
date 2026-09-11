/* standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* system libraries */
#include <unistd.h>
#include <sys/socket.h>

/* access-gate libraries */
#include "db.h"
#include "server.h"


/* ----- access-gate entrypoint ----- */
int main(void) {

    /* open DB connection */
    sqlite3 *db = NULL;
    access_gate_connect_db(&db);
    printf("[DB CONNECTION] : database ready at /data/access.db\n");

    /* set up the database check statement */
    sqlite3_stmt *check_statement = NULL;
    access_gate_set_db_check(db, &check_statement);

    /* set up the access-gate */
    int server_fd = access_gate_set();
    printf("[ACCESS-GATE CONNECTION] : \"access-gate\" listening on port %d\n", GATE_PORT);
    fflush(stdout);

    /* start the access-gate loop */
    for(;;) {
        char ip_address[IP_ADDRESS_SIZE] = "(none)";
        char host[HOST_SIZE] = "(none)";
        int client_fd = access_gate_handle_client(server_fd, ip_address, host);
        if (client_fd < 0) continue;

        /* print connection details */
        const char *check_result = access_gate_check_client(check_statement, ip_address, host);
        printf("[REQUEST] : \n\t- ip=%s\t- service=%s\t- decision=\"%s\"\n", ip_address, host, check_result);
        fflush(stdout);

        /* send HTTP response */
        const char *response = (strcmp(check_result, "ACCESS ALLOWED") == 0)
            ? "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
            : "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }

    /* access-gate closed */
    printf("[ACCESS-GATE CONNECTION] : \"access-gate\" closed\n");
    sqlite3_finalize(check_statement);
    sqlite3_close(db);
    fflush(stdout);
    return EXIT_SUCCESS;

}