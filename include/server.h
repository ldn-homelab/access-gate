#pragma once


/* 1 ----- set up the access-gate server ----- */
int access_gate_set(void);

/* 2 ----- handle a client connection to the access gate ----- */
int access_gate_handle_client(const int server_fd, char *ip_address, char *host);