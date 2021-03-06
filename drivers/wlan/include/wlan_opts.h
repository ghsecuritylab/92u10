#ifndef _WLAN_OPTS_H_
#define _WLAN_OPTS_H_

/* ATH_DEBUG -
 * Control whether debug features (printouts, assertions) are compiled
 * into the driver.
 */
#ifndef ATH_DEBUG
#define ATH_DEBUG                        1 /* default: include debug code */
#endif

#if ATH_DEBUG  
#define DEBUG_VAR_DECL_INIT(_var,_type,_value) _type (_var)=_value
#else
#define DEBUG_VAR_DECL_INIT(_var,_type,_value)
#endif

/* ATH_SUPPORT_WIRESHARK -
 * Control whether code that adds a radiotap packet header for consumption
 * by wireshark are compiled into the driver.
 */
#ifndef ATH_SUPPORT_WIRESHARK
#define ATH_SUPPORT_WIRESHARK            0 /* default: include radiotap/wireshark code */
#endif

#define ATH_SUPPORT_ATHVAP_INFO          1

/* ATH_SUPPORT_IBSS -
 * Control whether Adhoc support is compiled into the driver.
 */
#ifndef ATH_SUPPORT_IBSS
#define ATH_SUPPORT_IBSS                 1 /* default: include radiotap/wireshark code */
#endif

/* ATH_SLOW_ANT_DIV -
 * Control whether Slow Antenna Diversity support is compiled into the driver.
 */
#ifndef ATH_SLOW_ANT_DIV
#define ATH_SLOW_ANT_DIV                 0 /* default: do not include Slow Antenna Diversity code */
#endif

#ifndef ATH_SUPPORT_CWM
#define ATH_SUPPORT_CWM                  1 /* on unless explicitly disabled */
#endif

#ifndef ATH_SUPPORT_MULTIPLE_SCANS
#define ATH_SUPPORT_MULTIPLE_SCANS       0 /* default: do not include suport for multiple simultaneous scans, including Scan Scheduler code */
#endif

#ifndef ATH_BUILD_VISTA_ONLY
#define ATH_BUILD_VISTA_ONLY             0 /* default: build VISTA and WIN7 as the same binary */
#endif

#ifndef IEEE80211_DEBUG_REFCNT_SE
#define IEEE80211_DEBUG_REFCNT_SE        0 /* default: do not include scan_entry reference count debug info */
#endif

#ifndef ATH_OSPREY_UAPSDDEFERRED
#define ATH_OSPREY_UAPSDDEFERRED         0 /* default: handle Osprey UAPSD in ISR */
#endif

#ifndef ATH_RESET_SERIAL
#define ATH_RESET_SERIAL                0 /*default: do not run ath_reset always in passive_level */
#endif

#ifndef ATH_LOW_PRIORITY_CALIBRATE
#define ATH_LOW_PRIORITY_CALIBRATE      0 /*default: do not run ath_calibrate in workitem. */
#endif

#endif /* _WLAN_OPTS_H_ */
