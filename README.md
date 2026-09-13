# ldn-homelab/access-gate

A small **HTTP** service, meant to be used alongside a *reverse-proxy*, that checks if the IP making the request is allow to access further services.

This program has been written in plain **C** with no framework or **HTTP** library, in order to have as minimimal dependencies as possible.

---

## Architecture

```
client → reverse-proxy
				↓
			forward_auth → access-gate
								↓
							SQLite lookup → 200 OK / 403 Forbidden
```

Each incoming request carries:

- `X-Forwarded-For`: the original client's IP
- `X-Forwarded-Host`: the hostname being requested

In order to store the pairs `(ip, service)`, **Access-Gate** uses a simple **SQLite** table named `Access`; if the pair exists the response will be allowed (`200 OK`), otherwhise it will be denied (`403 Forbidden`).

---

## Database

```sql
CREATE TABLE Access (
  ip      TEXT NOT NULL,
  service TEXT NOT NULL,
  PRIMARY KEY (ip, service)
);
```

Each row represent a granted access for a pair `(ip, service)`; `service` must match the exact string sent as `X-Forwarded-Host` in order for the lookup to work properly.

---

## Design notes

- **Assumes one caller class**

**Access-Gate** is meant to run on a network reachable only by your reverse proxy, not exposed publicly; it doesn't validate the request path or defend against traffic that isn't a normal forward-auth check, because nothing else should be able to reach it.

- **No HTTP library**

Parsing is done directly on the raw socket.
This process sees every request to every service behind it, so keeping its dependencies at zero was a deliberate choice, not an oversight.

- **Matches by hostname, not path**

A grant covers a whole `X-Forwarded-Host` value; if one hostname serves multiple things at different paths, **Access-Gate** can't currently tell them apart.

- **Fails closed**

Anything other than an explicit match returns `403 Forbidden`.

---

## Deployment

The following command uses **Podman**, since the image has been tested with it, but it works fine on every container runtime.

```bash
podman run -d \
  --name access-gate \
  --network <reverse-proxy-network> \
  -v <data-folder>:/data \
  ghcr.io/ldn-homelab/access-gate:latest
```

These are the full requirements:
- network shared with whatever is going to send it forward-auth checks
- a writable `/data` mount, where `access.db` is created on first start.

No port is published on purpose; per the design notes above, it's meant to be reachable only from that shared network, not from the host or the internet.

Nothing here requires root or any specific capability, so every security specification (like user, capabilities, filesystem restrictions) is left to the user to decide.

---

## Managing database

The `Access` table is created automatically on first start, at `/data/access.db`.
Rows are managed via `access-gate.sh`, meant to sit in the same directory as your `.env`:

```bash
./access-gate.sh grant  <ip> <service>
./access-gate.sh revoke <ip> <service>
./access-gate.sh check
```

It reads `PATH_ACCESS_GATE_DATA` from `.env` to find the database, no path is hardcoded in the script itself.

---

## License

See [LICENSE](./LICENSE).