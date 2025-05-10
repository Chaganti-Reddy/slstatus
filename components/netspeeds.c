/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../slstatus.h"
#include "../util.h"

#define FIXED_WIDTH 6  /* Adjust this as needed for your layout */

static const char *
fmt_fixed_width_human(uintmax_t value)
{
	static char out[16];  // Enough for "999.9M/s" + safety margin

	double val = value;
	const char *unit;

	if (val < 1024) {
		unit = "B/s";
		val = value;
	} else if (val < 1024 * 1024) {
		unit = "K/s";
		val /= 1024;
	} else {
		unit = "M/s";
		val /= (1024 * 1024);
	}

	snprintf(out, sizeof(out), "%5.1f%s", val, unit);  // 999.9M/s is 8 chars

	return out;
}

#if defined(__linux__)

#define NET_RX_BYTES "/sys/class/net/%s/statistics/rx_bytes"
#define NET_TX_BYTES "/sys/class/net/%s/statistics/tx_bytes"

const char *
netspeed_rx(const char *interface)
{
	uintmax_t oldrxbytes;
	static uintmax_t rxbytes;
	extern const unsigned int interval;
	char path[PATH_MAX];

	oldrxbytes = rxbytes;

	if (esnprintf(path, sizeof(path), NET_RX_BYTES, interface) < 0)
		return NULL;
	if (pscanf(path, "%ju", &rxbytes) != 1)
		return NULL;
	if (oldrxbytes == 0)
		return NULL;

	return fmt_fixed_width_human((rxbytes - oldrxbytes) * 1000 / interval);
}

const char *
netspeed_tx(const char *interface)
{
	uintmax_t oldtxbytes;
	static uintmax_t txbytes;
	extern const unsigned int interval;
	char path[PATH_MAX];

	oldtxbytes = txbytes;

	if (esnprintf(path, sizeof(path), NET_TX_BYTES, interface) < 0)
		return NULL;
	if (pscanf(path, "%ju", &txbytes) != 1)
		return NULL;
	if (oldtxbytes == 0)
		return NULL;

	return fmt_fixed_width_human((txbytes - oldtxbytes) * 1000 / interval);
}

#elif defined(__OpenBSD__) || defined(__FreeBSD__)

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>

const char *
netspeed_rx(const char *interface)
{
	struct ifaddrs *ifal, *ifa;
	struct if_data *ifd;
	uintmax_t oldrxbytes;
	static uintmax_t rxbytes;
	extern const unsigned int interval;
	int if_ok = 0;

	oldrxbytes = rxbytes;

	if (getifaddrs(&ifal) < 0) {
		warn("getifaddrs failed");
		return NULL;
	}
	rxbytes = 0;
	for (ifa = ifal; ifa; ifa = ifa->ifa_next)
		if (!strcmp(ifa->ifa_name, interface) &&
		    (ifd = (struct if_data *)ifa->ifa_data))
			rxbytes += ifd->ifi_ibytes, if_ok = 1;

	freeifaddrs(ifal);
	if (!if_ok) {
		warn("reading 'if_data' failed");
		return NULL;
	}
	if (oldrxbytes == 0)
		return NULL;

	return fmt_fixed_width_human((rxbytes - oldrxbytes) * 1000 / interval);
}

const char *
netspeed_tx(const char *interface)
{
	struct ifaddrs *ifal, *ifa;
	struct if_data *ifd;
	uintmax_t oldtxbytes;
	static uintmax_t txbytes;
	extern const unsigned int interval;
	int if_ok = 0;

	oldtxbytes = txbytes;

	if (getifaddrs(&ifal) < 0) {
		warn("getifaddrs failed");
		return NULL;
	}
	txbytes = 0;
	for (ifa = ifal; ifa; ifa = ifa->ifa_next)
		if (!strcmp(ifa->ifa_name, interface) &&
		    (ifd = (struct if_data *)ifa->ifa_data))
			txbytes += ifd->ifi_obytes, if_ok = 1;

	freeifaddrs(ifal);
	if (!if_ok) {
		warn("reading 'if_data' failed");
		return NULL;
	}
	if (oldtxbytes == 0)
		return NULL;

	return fmt_fixed_width_human((txbytes - oldtxbytes) * 1000 / interval);
}

#endif

