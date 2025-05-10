/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
const unsigned int interval = 500;

/* text to show if no value can be retrieved */
static const char unknown_str[] = "n/a";

/* maximum output string length */
#define MAXLEN 2048

/*
 * function            description                     argument (example)
 *
 * battery_icon        battery_perc with an icon       battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_perc        battery percentage              battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_remaining   battery remaining HH:MM         battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_state       battery charging state          battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * cat                 read arbitrary file             path
 * cpu_freq            cpu frequency in MHz            NULL
 * cpu_perc            cpu usage in percent            NULL
 * datetime            date and time                   format string (%F %T)
 * disk_free           free disk space in GB           mountpoint path (/)
 * disk_perc           disk usage in percent           mountpoint path (/)
 * disk_total          total disk space in GB          mountpoint path (/)
 * disk_used           used disk space in GB           mountpoint path (/)
 * entropy             available entropy               NULL
 * gid                 GID of current user             NULL
 * hostname            hostname                        NULL
 * ipv4                IPv4 address                    interface name (eth0)
 * ipv6                IPv6 address                    interface name (eth0)
 * kernel_release      `uname -r`                      NULL
 * keyboard_indicators caps/num lock indicators        format string (c?n?)
 *                                                     see keyboard_indicators.c
 * keymap              layout (variant) of current     NULL
 *                     keymap
 * load_avg            load average                    NULL
 * netspeed_rx         receive network speed           interface name (wlan0)
 * netspeed_tx         transfer network speed          interface name (wlan0)
 * num_files           number of files in a directory  path
 *                                                     (/home/foo/Inbox/cur)
 * ram_free            free memory in GB               NULL
 * ram_perc            memory usage in percent         NULL
 * ram_total           total memory size in GB         NULL
 * ram_used            used memory in GB               NULL
 * run_command         custom shell command            command (echo foo)
 * swap_free           free swap in GB                 NULL
 * swap_perc           swap usage in percent           NULL
 * swap_total          total swap size in GB           NULL
 * swap_used           used swap in GB                 NULL
 * temp                temperature in degree celsius   sensor file
 *                                                     (/sys/class/thermal/...)
 *                                                     NULL on OpenBSD
 *                                                     thermal zone on FreeBSD
 *                                                     (tz0, tz1, etc.)
 * uid                 UID of current user             NULL
 * uptime              system uptime                   NULL
 * username            username of current user        NULL
 * vol_icon            vol_perc with an icon           mixer file (/dev/mixer)
 *                                                     NULL on OpenBSD/FreeBSD
 * vol_perc            OSS/ALSA volume in percent      mixer file (/dev/mixer)
 *                                                     NULL on OpenBSD/FreeBSD
 * wifi_essid          WiFi ESSID                      interface name (wlan0)
 * wifi_perc           WiFi signal in percent          interface name (wlan0)
 */

static const char brightness[] = "[ $(brightnessctl g) -gt 0 ] && printf '%.0f' $(echo \"$(brightnessctl g)*100/$(brightnessctl m)\" | bc) || printf 'Off'";

// static const struct arg args[] = {
// 	/* function format          argument */
//         // { netspeed_tx, " %s ", "wlan0" },
//         // { netspeed_rx, " %sB/s | ", "wlan0" },
//         // // { wifi_perc, "(%s) | ", "wlan0" },
//   
//         { netspeed_tx, "%s  ", "wlp45s0" },
//         { netspeed_rx, "%sB/s  | ", "wlp45s0" },
//         // { wifi_perc, "(%s) | ", "wlp45s0" },
//         { uptime,             " %s | ",      NULL },
//         // { cpu_perc,             " %s%% ",      NULL },
//         { ram_used,             " %s | ",         NULL },
//         // { ram_perc,             "(%s%%) ",      NULL },
//        { vol_icon, " %s | ", NULL },
//         { run_command, ": %s% | ", "wpctl get-volume @DEFAULT_SINK@ | awk '{print ($2 * 100) ($3 == \"[MUTED]\" ? \"M\" : \"\")}'" },
//         // Microphone
//         { run_command, ": %s% | ", "wpctl get-volume @DEFAULT_SOURCE@ | awk '{print ($2 * 100) ($3 == \"[MUTED]\" ? \"M\" : \"\")}'" },
//         { run_command,          " %s%% | ",      brightness },
//         { battery_perc,         " %s%%",       "BAT0" },
//         { battery_state,        "(%s) | ",        "BAT0" },
//         { run_command,          "%s",      "date +'%a %b %d %H:%M:%S'" },
//         // { keymap,               " %s ",        NULL },
// 	      // { datetime, "%s",           "%F %T" },
// 	      // { datetime, "%s",           "%a %b %d %T" },
// };


static const struct arg args[] = {
	/* function      format           argument */
  { netspeed_tx,   "%s  ",         "wlp45s0" },
	{ netspeed_rx,   "%s  | ",    "wlp45s0" },
  { uptime,        " %s | ",       NULL },
	{ ram_used,      " %s | ",       NULL },

/* Volume (PipeWire) */
{ run_command, "%s | ", 
  "v=$(wpctl get-volume @DEFAULT_SINK@ | awk '{print int($2 * 100)}'); \
  mute=$(wpctl get-volume @DEFAULT_SINK@ | grep MUTED); \
  if [ \"$mute\" != \"\" ]; then icon='󰝟'; \
  elif [ $v -eq 0 ]; then icon='󰕿'; \
  elif [ $v -le 20 ]; then icon=''; \
  elif [ $v -le 40 ]; then icon='󰖀'; \
  elif [ $v -le 60 ]; then icon='󰕾'; \
  elif [ $v -le 90 ]; then icon=''; \
  else icon='󰝝'; fi; \
  echo \"$icon $v%\"" },

/* Microphone (PipeWire) */
{ run_command, "%s | ", 
  "v=$(wpctl get-volume @DEFAULT_SOURCE@ | awk '{print int($2 * 100)}'); \
  mute=$(wpctl get-volume @DEFAULT_SOURCE@ | grep MUTED); \
  if [ \"$mute\" != \"\" ]; then icon='󰍭'; \
  elif [ $v -le 20 ]; then icon='󰢳 '; \
  elif [ $v -le 40 ]; then icon=''; \
  elif [ $v -le 60 ]; then icon='󰢴'; \
  elif [ $v -le 80 ]; then icon=''; \
  else icon=''; fi; \
  echo \"$icon $v%\"" },


	/* Brightness */
	// { run_command,   " %s%% | ",     brightness },
  
/* Brightness with icons (dark theme optimized) */
{ run_command, "%s | ", 
  "v=$(brightnessctl g); \
   max=$(brightnessctl m); \
   if [ $v -eq 0 ]; then \
     echo ' Off'; \
   else \
     perc=$((v * 100 / max)); \
     if [ $perc -le 10 ]; then icon='󰃚'; \
     elif [ $perc -le 20 ]; then icon='󰃜'; \
     elif [ $perc -le 40 ]; then icon='󰃝'; \
     elif [ $perc -le 60 ]; then icon='󰃟'; \
     elif [ $perc -le 80 ]; then icon='󰃟'; \
     else icon='󰃠'; fi; \
     echo \"$icon $perc%\"; \
   fi"
},

	/* Battery (patched battery_icon with Nerd Font icons) */
	{ battery_icon,  "%s | ",         "BAT0" },

	/* Date & Time */
	{ run_command,   "%s",            "date +'%a %b %d %H:%M:%S'" },
};
