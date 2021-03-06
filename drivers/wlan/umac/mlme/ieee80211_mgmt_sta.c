/*
 * Copyright (c) 2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ieee80211_mlme_priv.h"

static OS_TIMER_FUNC(ieee80211_synced_channel_switch);

#if UMAC_SUPPORT_STA || UMAC_SUPPORT_BTAMP
int
ieee80211_recv_asresp(struct ieee80211_node *ni,
                      wbuf_t wbuf,
                      int subtype)
{
    struct ieee80211vap *vap = ni->ni_vap;
    struct ieee80211_frame *wh;
    u_int8_t *frm, *efrm;
    u_int16_t capinfo, associd, status;

    wh = (struct ieee80211_frame *) wbuf_header(wbuf);
    frm = (u_int8_t *)&wh[1];
    efrm = wbuf_header(wbuf) + wbuf_get_pktlen(wbuf);

    if (vap->iv_opmode != IEEE80211_M_STA && vap->iv_opmode != IEEE80211_M_BTAMP) {
        vap->iv_stats.is_rx_mgtdiscard++;
        return -EINVAL;
    }

    IEEE80211_VERIFY_ADDR(ni);

    /*
     * asresp frame format
     *  [2] capability information
     *  [2] status
     *  [2] association ID
     *  [tlv] supported rates
     *  [tlv] extended supported rates
     *  [tlv] WME
     */
    IEEE80211_VERIFY_LENGTH(efrm - frm, 6);
    capinfo = le16toh(*(u_int16_t *)frm); frm += 2;
    status = le16toh(*(u_int16_t *)frm); frm += 2;
    associd = le16toh(*(u_int16_t *)frm); frm += 2;

    ieee80211_mlme_recv_assoc_response(ni, subtype, capinfo, status,  associd, frm, efrm - frm, wbuf);

    return 0;
}

static bool
ieee80211_phymode_match(ieee80211_scan_entry_t scan_entry, struct ieee80211_node *ni)
{
    u_int32_t    ni_phy_mode, se_phy_mode;

    ni_phy_mode = wlan_channel_phymode(ni->ni_chan);
    se_phy_mode = wlan_scan_entry_phymode(scan_entry);

    return (se_phy_mode == ni_phy_mode);
}

static bool
ieee80211_ssid_match(ieee80211_scan_entry_t scan_entry, struct ieee80211_node *ni)
{
    u_int8_t    ssid_len;
    u_int8_t    *ssid;
    
    /*
     * If beacon/probe response contains SSID, accept it only if it matches
     * the node's SSID; 
     * If beacon/probe response has no SSID, accept it to reduce incidence of 
     * timeouts.
     */
    ssid = wlan_scan_entry_ssid(scan_entry, &ssid_len);
    if (ssid != NULL) {
        return (OS_MEMCMP(ssid, ni->ni_essid, ssid_len) == 0);
    }
        
    // SSID is NULL - accept it.
    return true;
}

void ieee80211_scs_vattach(struct ieee80211vap *vap) {
   struct ieee80211com *ic = vap->iv_ic;
   osdev_t os_handle = ic->ic_osdev;
   vap->iv_cswitch_rxd = 0;

   OS_INIT_TIMER(os_handle, &vap->iv_cswitch_timer, ieee80211_synced_channel_switch, vap); 
}

void ieee80211_scs_vdetach(struct ieee80211vap *vap) {
   OS_FREE_TIMER(&vap->iv_cswitch_timer); 
}

static OS_TIMER_FUNC(ieee80211_synced_channel_switch)
{
    struct ieee80211vap                          *vap = NULL;
    struct ieee80211com                          *ic = NULL;
    struct ieee80211_node                        *ni = NULL;
    struct ieee80211_channel                     *chan = NULL;
    int err = EOK;
    int status = (-EINVAL);

    OS_GET_TIMER_ARG(vap, struct ieee80211vap *);

    ic = vap->iv_ic;

    ni = vap->iv_ni;

    chan = vap->iv_cswitch_chan;
    
    if (vap->iv_cswitch_rxd) {
        /*
         * For Station, just switch channel right away.
         */
        if (!IEEE80211_IS_RADAR_ENABLED(ic) && (chan != NULL) &&
            (chan != vap->iv_bsschan)) {
        
            IEEE80211_ENABLE_RADAR(ic);
    
            /*
             * Issue a channel switch request to resource manager.
             * If the function returns EOK (0) then its ok to change the channel synchronously
             * If the function returns EBUSY then resource manager will 
             * switch channel asynchronously and post an event event handler registred by vap and
             * vap handler will inturn do the rest of the processing involved. 
             */
            err = ieee80211_resmgr_request_chanswitch(ic->ic_resmgr, vap, chan, MLME_REQ_ID);
    
            if (err == EOK) {
                status = ieee80211_set_channel(ic, chan);

                if (status == EOK) {
                    vap->iv_bsschan = ic->ic_curchan;
            
                    /* Update node channel setting */
                    ieee80211_node_set_chan(ni);
            
                } else {
                    /*
                     * If failed to switch the channel, mark the AP as radar detected and disconnect from the AP.
                     */
                    ieee80211_mlme_recv_csa(ni, IEEE80211_RADAR_DETECT_DEFAULT_DELAY,true);
                }
            
                IEEE80211_DISABLE_RADAR(ic);

            } else if (err == EBUSY) {
                err = EOK;
            }
        }
        if (err != EOK) {
            /*
             * If failed to switch the channel, mark the AP as radar detected and disconnect from the AP.
             */
            ieee80211_mlme_recv_csa(ni, IEEE80211_RADAR_DETECT_DEFAULT_DELAY, true);
        }

    }
    
    OS_CANCEL_TIMER(&vap->iv_cswitch_timer);
    vap->iv_cswitch_rxd = 0;
}

void ieee80211_recv_beacon_sta(struct ieee80211_node *ni, wbuf_t wbuf, int subtype, 
                               struct ieee80211_rx_status *rs, ieee80211_scan_entry_t  scan_entry)
{
    struct ieee80211vap                          *tmpvap, *vap = ni->ni_vap;
    struct ieee80211com                          *ic = ni->ni_ic;
    u_int16_t                                    capinfo, erp;
    u_int8_t                                     *tim_elm = NULL; 
    struct ieee80211_channelswitch_ie            *chanie = NULL;
    struct ieee80211_extendedchannelswitch_ie    *echanie = NULL;
    struct ieee80211_channel*                    chan = NULL;
    u_int8_t                                     *htcap = NULL;
    u_int8_t                                     *htinfo = NULL;
    u_int8_t                                     *wme;
    struct ieee80211_frame                       *wh;
    u_int64_t                                    tsf;
    u_int8_t                                     chno = 0;
    systime_t                                    previous_beacon_time;
                                     

    wh = (struct ieee80211_frame *)wbuf_header(wbuf);
    OS_MEMCPY((u_int8_t *)&tsf, ieee80211_scan_entry_tsf(scan_entry), sizeof(tsf));
    capinfo = ieee80211_scan_entry_capinfo(scan_entry);

    /* FIX for EV# 98854: Preserving last successful beacon */
    previous_beacon_time = vap->iv_last_beacon_time;
    /*
     * When operating in station mode, check for state updates.
     * Be careful to ignore beacons received while doing a
     * background scan.  We consider only 11g/WMM stuff right now.
     */
    chan = ieee80211_scan_entry_channel(scan_entry);
    chno = ieee80211_channel_ieee(chan);
    if ((ieee80211_node_get_associd(ni) != 0) &&
        IEEE80211_ADDR_EQ(wh->i_addr2, ieee80211_node_get_bssid(ni)) &&
        chno == vap->iv_bsschan->ic_ieee) {
        /*
         * record tsf of last beacon
         */
        OS_MEMCPY(ni->ni_tstamp.data, &tsf, sizeof(ni->ni_tstamp));

        /*
         * record absolute time of last beacon
         */
        vap->iv_last_beacon_time = OS_GET_TIMESTAMP();
#if UMAC_SUPPORT_VAP_PAUSE        
        atomic_inc(&vap->iv_pause_info.iv_pause_beacon_count);
#endif

        /*
         * check for ERP change
         */
        erp = ieee80211_scan_entry_erpinfo(scan_entry);
        if ((ni->ni_erp != erp) &&
            (ieee80211_vaps_ready(ic, IEEE80211_M_HOSTAP) == 0)) {
            IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
                           "erp change: was 0x%x, now 0x%x",
                           ni->ni_erp, erp);
            if (erp & IEEE80211_ERP_USE_PROTECTION)
                IEEE80211_ENABLE_PROTECTION(ic);
            else
                IEEE80211_DISABLE_PROTECTION(ic);
            IEEE80211_COMM_LOCK(ic);
            TAILQ_FOREACH(tmpvap, &(ic)->ic_vaps, iv_next) {
                ieee80211_vap_erpupdate_set(tmpvap);
            }
            IEEE80211_COMM_UNLOCK(ic);
            ic->ic_update_protmode(ic);
            ni->ni_erp = erp;
            if (ni->ni_erp & IEEE80211_ERP_LONG_PREAMBLE)
                IEEE80211_ENABLE_BARKER(ic);
            else
                IEEE80211_DISABLE_BARKER(ic);
        }

        /*
         * check for slot time change
         */
        if ((ni->ni_capinfo ^ capinfo) & IEEE80211_CAPINFO_SHORT_SLOTTIME) {
            IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
                           "capabilities change: was 0x%x, now 0x%x",
                           ni->ni_capinfo, capinfo);
            /*
             * NB: we assume short preamble doesn't
             *     change dynamically
             */
            ieee80211_set_shortslottime(ic, 
                                        IEEE80211_IS_CHAN_A(vap->iv_bsschan) ||
                                        IEEE80211_IS_CHAN_11NA(vap->iv_bsschan) ||
                                        (capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME));
            ni->ni_capinfo &= ~IEEE80211_CAPINFO_SHORT_SLOTTIME;
            ni->ni_capinfo |= capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME;
        }

        /*
         * check for tim
         */
        if (ieee80211_scan_can_transmit(ic->ic_scanner)) {
            tim_elm = ieee80211_scan_entry_tim(scan_entry);
            if (tim_elm != NULL) {
                struct ieee80211_tim_ie    *tim = (struct ieee80211_tim_ie *) tim_elm;
                int                        aid = IEEE80211_AID(ieee80211_node_get_associd(ni));
                int                        ix = aid / NBBY;
                int                        min_ix = tim->tim_bitctl &~ 1;
                int                        max_ix = tim->tim_len + min_ix - 4;

                if (tim->tim_bitctl & 1) {           /* dtim */
                    ieee80211_sta_power_event_dtim(vap->iv_pwrsave_sta);
                }
                
                if ((min_ix <= ix) && (ix <= max_ix) &&
                    isset(tim->tim_bitmap - min_ix, aid)) {  /* tim */
                    ieee80211_sta_power_event_tim(vap->iv_pwrsave_sta);
                }

                ni->ni_dtim_count = tim->tim_count;
                ni->ni_dtim_period = tim->tim_period;
                /* Also update vap's iv_dtim_period. OS may querry */
                ni->ni_vap->iv_dtim_period = tim->tim_period;
            }
        }

        /*
         * check for WMM parameters
         */
        if (((wme = ieee80211_scan_entry_wmeparam_ie(scan_entry)) != NULL) ||
            ((wme = ieee80211_scan_entry_wmeinfo_ie(scan_entry))  != NULL)) {
            int         _retval;
            u_int8_t    qosinfo;

            /* Node is WMM-capable if WME IE (either subtype) is present */
            ni->ni_ext_caps |= IEEE80211_NODE_C_QOS;

            if (vap->iv_opmode != IEEE80211_M_BTAMP) {
            /* Parse IE according to subtype */
            if (iswmeparam(wme)) {
                _retval = ieee80211_parse_wmeparams(vap, wme, &qosinfo, 0);
            }
            else {
                _retval = ieee80211_parse_wmeinfo(vap, wme, &qosinfo);
            }

            /* Check whether WME parameters changed */
            if (_retval > 0) {
                /* 
                 * Do not update U-APSD capability; this is done 
                 * during association request.
                 */

                /* Check whether node is in a WMM connection */
                if (ni->ni_flags & IEEE80211_NODE_QOS) {
                    /* Update WME parameters  */
                    if (iswmeparam(wme)) {
                        ieee80211_wme_updateparams(vap);
                    }
                    else {
                        ieee80211_wme_updateinfo(vap);
                    }
                }

                /* 
                 * If association used U-APSD and AP no longer 
                 * supports it we must disassociate and renegotiate
                 * U-APSD support.
                 *
                 * We don't need to do anything if association did not
                 * use U-APSD and AP now supports it.
                 */
                if (ni->ni_flags & IEEE80211_NODE_UAPSD) {
                    if ((qosinfo & WME_CAPINFO_UAPSD_EN) == 0) {
                        ieee80211node_clear_flag(ni, IEEE80211_NODE_UAPSD);

                        /* Disassociate for unspecified QOS-related reason. */
                        ieee80211_mlme_recv_deauth(ni, IEEE80211_REASON_QOS);
                    }
                }
            }
            }
        } else {
            /* If WME IE not present node is not WMM capable */
            ni->ni_ext_caps &= ~IEEE80211_NODE_C_QOS;

            /* 
             * If association used U-APSD and AP no longer 
             * supports it we must disassociate and renegotiate
             * U-APSD support.
             *
             * We don't need to do anything if association did not
             * use U-APSD and AP now supports it.
             */
            if (ni->ni_flags & IEEE80211_NODE_UAPSD) {
                ieee80211node_clear_flag(ni, IEEE80211_NODE_UAPSD);

                /* Disassociate for unspecified QOS-related reason. */
                ieee80211_mlme_recv_deauth(ni, IEEE80211_REASON_QOS);
            }
        }

        /*
         * check for spectrum management
         */
        if (capinfo & IEEE80211_CAPINFO_SPECTRUM_MGMT) {
            chanie  = (struct ieee80211_channelswitch_ie *)         ieee80211_scan_entry_csa(scan_entry);
            echanie = (struct ieee80211_extendedchannelswitch_ie *) ieee80211_scan_entry_xcsa(scan_entry);
            if (chanie) {
                chan = ieee80211_doth_findchan(vap, chanie->newchannel);
                vap->iv_cswitch_chan = chan;
            }
            else if (echanie) {
                chan = ieee80211_doth_findchan(vap, echanie->newchannel);
                vap->iv_cswitch_chan = chan;
            }
            else {
                /* Do nothing */
            }
        }


        /*
         * check HT IEs
         */
        if (IEEE80211_NODE_USE_HT(ni)) {
            htcap  = ieee80211_scan_entry_htcap(scan_entry);
            htinfo = ieee80211_scan_entry_htinfo(scan_entry);
            if (htcap) {
                ieee80211_parse_htcap(ni, htcap);
            }
            if (htinfo) {
                enum ieee80211_cwm_width ochwidth = ni->ni_chwidth;
                ieee80211_parse_htinfo(ni, htinfo);
        
                /*
                 * Check for AP changing channel width.
                 * Verify the AP is capable of 40 MHz as sometimes the AP
                 * can get into an inconsistent state (see EV 72018)
                 */
                if ((ni->ni_htcap & IEEE80211_HTCAP_C_CHWIDTH40) &&
                    (ochwidth != ni->ni_chwidth))
                {
                    u_int32_t  rxlinkspeed, txlinkspeed; /* bits/sec */
                    
                    ic->ic_chwidth_change(ni);
                    mlme_get_linkrate(ni, &rxlinkspeed, &txlinkspeed);
                    IEEE80211_DELIVER_EVENT_LINK_SPEED(vap, rxlinkspeed, txlinkspeed);
                }
            }
        } 
    }
    
    /*
     * Check BBSID before updating our beacon configuration to make 
     * sure the received beacon is really from our AP.
     */
    if ((ni == vap->iv_bss) &&
        (IEEE80211_ADDR_EQ(wh->i_addr3, ieee80211_node_get_bssid(ni)))) {
        //
        // Check ssid and phymode to make sure ESS/phymode broadcasted by the AP is the same to
        // which we're trying to connect. AP may have been reconfigured but
        // old entry remains in the scan list until aged out.
        //
        if (ieee80211_ssid_match(scan_entry, ni) && 
            ieee80211_phymode_match(scan_entry, ni) &&
            (ni->ni_chan == ieee80211_scan_entry_channel(scan_entry))) {
            vap->iv_lastbcn_phymode_mismatch = 0;
            ieee80211_mlme_join_complete_infra(ni);
        }
        else { 
         /* EV# 98854 Reverting the Beacon time stamp if scan_entry 
            channel mismatches  */
            vap->iv_lastbcn_phymode_mismatch = 1;
            vap->iv_last_beacon_time = previous_beacon_time;
        }
            
        if(ieee80211_vap_ready_is_set(vap) &&
           (subtype == IEEE80211_FC0_SUBTYPE_BEACON)) {
            ic->ic_beacon_update(ni, rs->rs_rssi);            
        }

        ieee80211_vap_compute_tsf_offset(vap, wbuf, rs);
    
          
        /*
        * Set or clear flag indicating reception of channel switch announcement
        * in this channel. This flag should be set before notifying the scan 
        * algorithm.
        * We should not send probe requests on a channel on which CSA was 
        * received until we receive another beacon without the said flag.
        */
        if ((chanie != NULL) || (echanie != NULL)) {
            ic->ic_curchan->ic_flagext |= IEEE80211_CHAN_CSA_RECEIVED;

            if(IEEE80211_VAP_IS_WDS_ENABLED(vap) 
                && !(ic->ic_flags & IEEE80211_F_CHANSWITCH)) {
                   /*
                    * This VAP is in WDS mode and the received beacon is from 
                    * our AP which contains a CSA. 
                    */
                    ic->ic_flags |= IEEE80211_F_CHANSWITCH ;
                    if(chanie != NULL) {
                            ic->ic_chanchange_chan = chanie->newchannel ;
                            ic->ic_chanchange_tbtt = chanie->tbttcount ;
                    }
                    else {
                            ic->ic_chanchange_chan = echanie->newchannel ;
                            ic->ic_chanchange_tbtt = echanie->tbttcount ;
                    }
            }
        
        }
        else if (!ieee80211_scan_entry_is_radar_detected_period(scan_entry)) {
            ic->ic_curchan->ic_flagext &= ~IEEE80211_CHAN_CSA_RECEIVED;
        }

        if ((chanie != NULL) || (echanie != NULL)) {
          vap->iv_cswitch_rxd = 1;
          if (chanie != NULL) {
            ni->ni_chanswitch_tbtt = chanie->tbttcount;
          }
          else if (echanie != NULL) {
            ni->ni_chanswitch_tbtt = echanie->tbttcount;
          }
          else {
            // should never come here
            ni->ni_chanswitch_tbtt = 0;
          }
          vap->iv_ni = ni;
          OS_SET_TIMER(&vap->iv_cswitch_timer,IEEE80211_TU_TO_MS(ni->ni_chanswitch_tbtt*ni->ni_intval));
        }
    }
}

void ieee80211_mlme_chanswitch_continue(struct ieee80211_node *ni, int status)
{
    struct ieee80211com *ic = ni->ni_ic;
    struct ieee80211vap *vap = ni->ni_vap;

    if (status == EOK) {
        vap->iv_bsschan = ic->ic_curchan;

        /* Update node channel setting */
        ieee80211_node_set_chan(ni);

        /* After switching the channel, enabling the CSA delay when AP is not available yet.
         */
        if (IEEE80211_IS_CHAN_DFS(vap->iv_bsschan)) {
            u_int32_t  csa_delay = IEEE80211_TU_TO_MS(ni->ni_chanswitch_tbtt * ni->ni_intval);

                        /*
                         * As the new channel is DFS channel, waiting till the AP beaconing. But don't disconnect from this AP.
                         */
                        ieee80211_mlme_recv_csa(ni, csa_delay, false);
                    }
    } else {

            /*
             * If failed to switch the channel, mark the AP as radar detected and disconnect from the AP.
             */
            ieee80211_mlme_recv_csa(ni, IEEE80211_RADAR_DETECT_DEFAULT_DELAY,true);
        }

    IEEE80211_DISABLE_RADAR(ic);
}

/**
 * check wpa/rsn ie is present in the ie buffer passed in. 
 */
bool ieee80211_check_wpaie(struct ieee80211vap *vap, u_int8_t *iebuf, u_int32_t length)
{
    u_int8_t *iebuf_end = iebuf + length;
    struct ieee80211_rsnparms tmp_rsn;
    bool add_wpa_ie = true;
    while (add_wpa_ie && ((iebuf+1) < iebuf_end )) {
        if (iebuf[0] == IEEE80211_ELEMID_VENDOR) {
            if (iswpaoui(iebuf) &&  (ieee80211_parse_wpa(vap, iebuf, &tmp_rsn) == 0)) {
                if (RSN_CIPHER_IS_CLEAR(&vap->iv_rsn) || vap->iv_rsn.rsn_ucastcipherset == 0) {
                    vap->iv_rsn = tmp_rsn;
                }
                /* found WPA IE */
                add_wpa_ie = false;
            }
        }
        else if(iebuf[0] == IEEE80211_ELEMID_RSN) {
            if (ieee80211_parse_rsn(vap, iebuf, &tmp_rsn) == 0) {
                /* found RSN IE */
                if (RSN_CIPHER_IS_CLEAR(&vap->iv_rsn) || vap->iv_rsn.rsn_ucastcipherset == 0) {
                vap->iv_rsn = tmp_rsn;
                }
                add_wpa_ie = false;
            }
        }
        iebuf += iebuf[1] + 2;
        continue;
    }
    return add_wpa_ie;
}

/*
 * Setup an association/reassociation request,
 * and returns the frame length.
 */
static u_int16_t
ieee80211_setup_assoc(
    struct ieee80211_node *ni,
    struct ieee80211_frame *wh,
    int reassoc,
    u_int8_t *previous_bssid
    )
{
    struct ieee80211vap *vap = ni->ni_vap;
    struct ieee80211com *ic = ni->ni_ic;
    struct ieee80211_rsnparms *rsn = &vap->iv_rsn;
    u_int8_t *frm;
    u_int16_t capinfo = 0;
    u_int8_t subtype = reassoc ? IEEE80211_FC0_SUBTYPE_REASSOC_REQ :
        IEEE80211_FC0_SUBTYPE_ASSOC_REQ;
    bool  add_wpa_ie;

    ieee80211_send_setup(vap, ni, wh,
                         (IEEE80211_FC0_TYPE_MGT | subtype),
                         vap->iv_myaddr, ni->ni_macaddr, ni->ni_bssid);
    frm = (u_int8_t *)&wh[1];

    /*
     * asreq frame format
     *[2] capability information
     *[2] listen interval
     *[6*] current AP address (reassoc only)
     *[tlv] ssid
     *[tlv] supported rates
     *[4] power capability
     *[28] supported channels element
     *[tlv] extended supported rates
     *[tlv] WME [if enabled and AP capable]
     *[tlv] HT Capabilities
     *      [tlv] Atheros advanced capabilities
     *[tlv] user-specified ie's
     */
    if (vap->iv_opmode == IEEE80211_M_IBSS)
        capinfo |= IEEE80211_CAPINFO_IBSS;
    else if (vap->iv_opmode == IEEE80211_M_STA || vap->iv_opmode == IEEE80211_M_BTAMP)
        capinfo |= IEEE80211_CAPINFO_ESS;
    else
        ASSERT(0);
    
    if (IEEE80211_VAP_IS_PRIVACY_ENABLED(vap))
        capinfo |= IEEE80211_CAPINFO_PRIVACY;
    /*
     * NB: Some 11a AP's reject the request when
     *     short premable is set.
     */
    if ((ic->ic_flags & IEEE80211_F_SHPREAMBLE) &&
        IEEE80211_IS_CHAN_2GHZ(ic->ic_curchan))
        capinfo |= IEEE80211_CAPINFO_SHORT_PREAMBLE;
    if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME) &&
        (ic->ic_flags & IEEE80211_F_SHSLOT))
        capinfo |= IEEE80211_CAPINFO_SHORT_SLOTTIME;
    if (ieee80211_ic_doth_is_set(ic) && ieee80211_vap_doth_is_set(vap))
        capinfo |= IEEE80211_CAPINFO_SPECTRUM_MGMT;

    *(u_int16_t *)frm = htole16(capinfo);
    frm += 2;

    *(u_int16_t *)frm = htole16(ic->ic_lintval);
    frm += 2;

    if (reassoc) {
        IEEE80211_ADDR_COPY(frm, previous_bssid);
        frm += IEEE80211_ADDR_LEN;
    }

    frm = ieee80211_add_ssid(frm,
                             ni->ni_essid,
                             ni->ni_esslen);
    frm = ieee80211_add_rates(frm, &ni->ni_rates);

    /*
     * DOTH elements
     */
    if (ieee80211_ic_doth_is_set(ic) && ieee80211_vap_doth_is_set(vap)) {
        frm = ieee80211_add_doth(frm, vap);
    }

    add_wpa_ie=true;
    /*
     * check if os shim has setup RSN IE it self.
     */
    IEEE80211_VAP_LOCK(vap);
    if (vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].length) {
        add_wpa_ie = ieee80211_check_wpaie(vap, vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].ie,
                                           vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].length);
    }
    if (vap->iv_opt_ie.length) {
        add_wpa_ie = ieee80211_check_wpaie(vap, vap->iv_opt_ie.ie,
                                           vap->iv_opt_ie.length);
    }
    if (add_wpa_ie) {
        /* Continue looking for the WPA IE */
        add_wpa_ie = ieee80211_mlme_app_ie_check_wpaie(vap);
    }
    IEEE80211_VAP_UNLOCK(vap);

    if (add_wpa_ie) {
        if (RSN_AUTH_IS_RSNA(rsn)) {
            frm = ieee80211_setup_rsn_ie(vap, frm);
        }
    }

#if ATH_SUPPORT_WAPI
    if (RSN_AUTH_IS_WAI(rsn))
        frm = ieee80211_setup_wapi_ie(vap, frm);
#endif

    frm = ieee80211_add_xrates(frm, &ni->ni_rates);
    wlan_set_tspecActive(vap, 0);
    if (vap->iv_ccx_evtable && vap->iv_ccx_evtable->wlan_ccx_set_vperf) {
        vap->iv_ccx_evtable->wlan_ccx_set_vperf(vap->iv_ccx_arg, 0);
    }


    /*
     * XXX: don't do WMM association with safe mode enabled, because
     * Vista OS doesn't know how to decrypt QoS frame.
     */
    if (ieee80211_vap_wme_is_set(vap) &&
        (ni->ni_ext_caps & IEEE80211_NODE_C_QOS) &&
        !IEEE80211_VAP_IS_SAFEMODE_ENABLED(vap)) {
        struct ieee80211_wme_tspec *sigtspec = &ni->ni_ic->ic_sigtspec;
        struct ieee80211_wme_tspec *datatspec = &ni->ni_ic->ic_datatspec;
        u_int8_t    tsrsiev[16];
        u_int8_t    tsrsvlen = 0;
        u_int32_t   minphyrate;

        frm = ieee80211_add_wmeinfo(frm, ni, WME_INFO_OUI_SUBTYPE, NULL, 0);
        if (iswmetspec((u_int8_t *)sigtspec)) {
            frm = ieee80211_add_wmeinfo(frm, ni, WME_TSPEC_OUI_SUBTYPE, 
                                        &sigtspec->ts_tsinfo[0], 
                                        sizeof (struct ieee80211_wme_tspec) - offsetof(struct ieee80211_wme_tspec, ts_tsinfo));
            minphyrate = *((u_int32_t *) &sigtspec->ts_min_phy[0]);
            if (vap->iv_ccx_evtable && vap->iv_ccx_evtable->wlan_ccx_fill_tsrsie) {
                vap->iv_ccx_evtable->wlan_ccx_fill_tsrsie(vap->iv_ccx_arg, 
                                ((struct ieee80211_tsinfo_bitmap *) &sigtspec->ts_tsinfo[0])->tid, 
                                minphyrate, &tsrsiev[0], &tsrsvlen);
            }
            if (tsrsvlen > 0) {
                *frm++ = IEEE80211_ELEMID_VENDOR;
                *frm++ = tsrsvlen;
                OS_MEMCPY(frm, &tsrsiev[0], tsrsvlen);
                frm += tsrsvlen;
            }
        }
        if (iswmetspec((u_int8_t *)datatspec)){
            frm = ieee80211_add_wmeinfo(frm, ni, WME_TSPEC_OUI_SUBTYPE, 
                                        &datatspec->ts_tsinfo[0], 
                                        sizeof (struct ieee80211_wme_tspec) - offsetof(struct ieee80211_wme_tspec, ts_tsinfo));
            minphyrate = *((u_int32_t *) &datatspec->ts_min_phy[0]);
            if (vap->iv_ccx_evtable && vap->iv_ccx_evtable->wlan_ccx_fill_tsrsie) {
                vap->iv_ccx_evtable->wlan_ccx_fill_tsrsie(vap->iv_ccx_arg, 
                                ((struct ieee80211_tsinfo_bitmap *) &datatspec->ts_tsinfo[0])->tid, 
                                minphyrate, &tsrsiev[0], &tsrsvlen);
            }
            if (tsrsvlen > 0) {
                *frm++ = IEEE80211_ELEMID_VENDOR;
                *frm++ = tsrsvlen;
                OS_MEMCPY(frm, &tsrsiev[0], tsrsvlen);
                frm += tsrsvlen;
            }
    }
    }

    if ((ni->ni_flags & IEEE80211_NODE_HT) && 
        IEEE80211_IS_CHAN_11N(ic->ic_curchan) && ieee80211vap_htallowed(vap)) {
        frm = ieee80211_add_htcap(frm, ni, subtype);

        if (!IEEE80211_IS_HTVIE_ENABLED(ic)) {
            frm = ieee80211_add_htcap_pre_ana(frm, ni, IEEE80211_FC0_SUBTYPE_PROBE_RESP);
        }
        else {
            frm = ieee80211_add_htcap_vendor_specific(frm, ni, subtype);
        }

        if (!(ic->ic_flags & IEEE80211_F_COEXT_DISABLE)) {
            frm = ieee80211_add_extcap(frm, ni);
        }
    }

    /* Insert ieee80211_ie_ath_extcap IE to beacon */
    if ((ni->ni_flags & IEEE80211_NODE_ATH) &&
        ic->ic_ath_extcap) {
        u_int16_t ath_extcap = 0;
        u_int8_t  ath_rxdelim = 0;
        
        if (ieee80211_has_weptkipaggr(ni)) {
                ath_extcap |= IEEE80211_ATHEC_WEPTKIPAGGR;
                ath_rxdelim = ic->ic_weptkipaggr_rxdelim;
        }

        if ((ni->ni_flags & IEEE80211_NODE_OWL_WDSWAR) &&
            (ic->ic_ath_extcap & IEEE80211_ATHEC_OWLWDSWAR)) {
                ath_extcap |= IEEE80211_ATHEC_OWLWDSWAR;
        }

        if (ieee80211com_has_extradelimwar(ic))
            ath_extcap |= IEEE80211_ATHEC_EXTRADELIMWAR;

        frm = ieee80211_add_athextcap(frm, ath_extcap, ath_rxdelim);
    }

#ifdef notyet
    if (ni->ni_ath_ie != NULL)
        frm = ieee80211_add_ath(frm, ni);
#endif
    /*
     * add wpa/rsn ie if os did not setup one.
     */
    if (add_wpa_ie) {
        if (RSN_AUTH_IS_WPA(rsn))
            frm = ieee80211_setup_wpa_ie(vap, frm);

        if (RSN_AUTH_IS_CCKM(rsn)) {
            ASSERT(!RSN_AUTH_IS_RSNA(rsn) && !RSN_AUTH_IS_WPA(rsn));
        
            /*
             * CCKM AKM can be either added to WPA IE or RSN IE,
             * depending on the AP's configuration
             */
            if (RSN_AUTH_IS_RSNA(&ni->ni_rsn))
                frm = ieee80211_setup_rsn_ie(vap, frm);
            else if (RSN_AUTH_IS_WPA(&ni->ni_rsn))
                frm = ieee80211_setup_wpa_ie(vap, frm);
        }
    }

    IEEE80211_VAP_LOCK(vap);
    if (vap->iv_opt_ie.length){
        OS_MEMCPY(frm, vap->iv_opt_ie.ie,
                  vap->iv_opt_ie.length);
        frm += vap->iv_opt_ie.length;
    }
    if (vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].length){
        OS_MEMCPY(frm, vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].ie,
                  vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].length);
        frm += vap->iv_app_ie[IEEE80211_FRAME_TYPE_ASSOCREQ].length;
    }
    /* Add the Application IE's */
    frm = ieee80211_mlme_app_ie_append(vap, IEEE80211_FRAME_TYPE_ASSOCREQ, frm);

    IEEE80211_VAP_UNLOCK(vap);

    return (frm - (u_int8_t *)wh);
}

/*
 * Send an assoc/reassoc request frame
 */
int
ieee80211_send_assoc(struct ieee80211_node *ni,
                     int reassoc, u_int8_t *prev_bssid)
{
    struct ieee80211vap *vap = ni->ni_vap;
    struct ieee80211com *ic = ni->ni_ic;
    wbuf_t wbuf;
    u_int16_t length;

    wbuf = wbuf_alloc(ic->ic_osdev, WBUF_TX_MGMT, MAX_TX_RX_PACKET_SIZE);
    if (wbuf == NULL)
        return -ENOMEM;


#ifdef IEEE80211_DEBUG_REFCNT
    printk("%s ,line %u: increase node %p <%s> refcnt to %d\n",
           __func__, __LINE__, ni, ether_sprintf(ni->ni_macaddr),
           ieee80211_node_refcnt(ni));
#endif

    length = ieee80211_setup_assoc(ni, (struct ieee80211_frame *)wbuf_header(wbuf),
                                   reassoc, prev_bssid);

#ifdef ATHR_RNWF /* TODO - application specific IEs */
#if 0
    remLen = (*pAssocPacketLength - frameLen);
    StaCcxInsertAssocIEs(pStation, *ppAssocPacket, remLen, &frameLen);
#endif
#endif

    wbuf_set_pktlen(wbuf, length);

    /* Callback to allow OS layer to copy assoc/reassoc frame (Vista requirement) */
    IEEE80211_DELIVER_EVENT_MLME_ASSOC_REQ(vap, wbuf);

    return ieee80211_send_mgmt(vap,ni,wbuf,false);
}

int
wlan_send_addts(wlan_if_t vaphandle, u_int8_t *macaddr, ieee80211_tspec_info *tsinfo)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211_node *ni;
    struct ieee80211_action_mgt_args addts_args;
    struct ieee80211_action_mgt_buf  addts_buf;
    struct ieee80211_tsinfo_bitmap *tsflags;
    struct ieee80211_wme_tspec *tspec;

    ni = ieee80211_find_txnode(vap, macaddr);
    if (ni == NULL) {
        IEEE80211_DPRINTF(vap, IEEE80211_MSG_OUTPUT,
                          "%s: could not send ADDTS, no node found for %s\n", 
                          __func__, ether_sprintf(macaddr));
        return -EINVAL;
    }
    
    /*
     * ieee80211_action_mgt_args is a generic structure. TSPEC IE
     * is filled in the buf area.
     */
    addts_args.category = IEEE80211_ACTION_CAT_WMM_QOS;
    addts_args.action   = IEEE80211_WMM_QOS_ACTION_SETUP_REQ;
    addts_args.arg1     = IEEE80211_WMM_QOS_DIALOG_SETUP; /* dialogtoken */
    addts_args.arg2     = 0; /* status code */
    addts_args.arg3     = sizeof(struct ieee80211_wme_tspec);
    
    tspec = (struct ieee80211_wme_tspec *) &addts_buf.buf;
    tsflags = (struct ieee80211_tsinfo_bitmap *) &(tspec->ts_tsinfo);
    tsflags->direction = tsinfo->direction;
    tsflags->psb = tsinfo->psb;
    tsflags->dot1Dtag = tsinfo->dot1Dtag;
    tsflags->tid = tsinfo->dot1Dtag;
    tsflags->reserved3 = tsinfo->aggregation;
    tsflags->one = tsinfo->acc_policy_edca;
    tsflags->zero = tsinfo->acc_policy_hcca;
    tsflags->reserved1 = tsinfo->traffic_type;
    tsflags->reserved2 = tsinfo->ack_policy;

    *((u_int16_t *) &tspec->ts_nom_msdu) = tsinfo->norminal_msdu_size;
    *((u_int16_t *) &tspec->ts_max_msdu) = tsinfo->max_msdu_size;
    *((u_int32_t *) &tspec->ts_min_svc) = tsinfo->min_srv_interval;
    *((u_int32_t *) &tspec->ts_max_svc) = tsinfo->max_srv_interval;
    *((u_int32_t *) &tspec->ts_inactv_intv) = tsinfo->inactivity_interval;
    *((u_int32_t *) &tspec->ts_susp_intv) = tsinfo->suspension_interval;
    *((u_int32_t *) &tspec->ts_start_svc) = tsinfo->srv_start_time;
    *((u_int32_t *) &tspec->ts_min_rate) = tsinfo->min_data_rate;
    *((u_int32_t *) &tspec->ts_mean_rate) = tsinfo->mean_data_rate;
    *((u_int32_t *) &tspec->ts_max_burst) = tsinfo->max_burst_size;
    *((u_int32_t *) &tspec->ts_min_phy) = tsinfo->min_phy_rate;
    *((u_int32_t *) &tspec->ts_peak_rate) = tsinfo->peak_data_rate;
    *((u_int32_t *) &tspec->ts_delay) = tsinfo->delay_bound;
    *((u_int16_t *) &tspec->ts_surplus) = tsinfo->surplus_bw;
    *((u_int16_t *) &tspec->ts_medium_time) = 0;

    ieee80211_send_action(ni, &addts_args, &addts_buf);
    ieee80211_free_node(ni);    /* reclaim node */
    return 0;
}

int
wlan_send_delts(wlan_if_t vaphandle, u_int8_t *macaddr, ieee80211_tspec_info *tsinfo)
{
    struct ieee80211vap *vap = vaphandle;
    struct ieee80211_node *ni;
    struct ieee80211_action_mgt_args delts_args;
    struct ieee80211_action_mgt_buf  delts_buf;
    struct ieee80211_tsinfo_bitmap *tsflags;
    struct ieee80211_wme_tspec *tspec;

    ni = ieee80211_find_txnode(vap, macaddr);
    if (ni == NULL) {
        IEEE80211_DPRINTF(vap, IEEE80211_MSG_OUTPUT,
                          "%s: could not send DELTS, no node found for %s\n", 
                          __func__, ether_sprintf(macaddr));
        return -EINVAL;
    }

    /*
     * ieee80211_action_mgt_args is a generic structure. TSPEC IE
     * is filled in the buf area.
     */
    delts_args.category = IEEE80211_ACTION_CAT_WMM_QOS;
    delts_args.action   = IEEE80211_WMM_QOS_ACTION_TEARDOWN;
    delts_args.arg1     = IEEE80211_WMM_QOS_DIALOG_SETUP; /* dialogtoken */
    delts_args.arg2     = 0; /* status code */
    delts_args.arg3     = sizeof(struct ieee80211_wme_tspec);

    tspec = (struct ieee80211_wme_tspec *) &delts_buf.buf;
    tsflags = (struct ieee80211_tsinfo_bitmap *) &(tspec->ts_tsinfo);
    tsflags->direction = tsinfo->direction;
    tsflags->psb = tsinfo->psb;
    tsflags->dot1Dtag = tsinfo->dot1Dtag;
    tsflags->tid = tsinfo->dot1Dtag;
    tsflags->reserved3 = tsinfo->aggregation;
    tsflags->one = tsinfo->acc_policy_edca;
    tsflags->zero = tsinfo->acc_policy_hcca;
    tsflags->reserved1 = tsinfo->traffic_type;
    tsflags->reserved2 = tsinfo->ack_policy;

    *((u_int16_t *) &tspec->ts_nom_msdu) = tsinfo->norminal_msdu_size;
    *((u_int16_t *) &tspec->ts_max_msdu) = tsinfo->max_msdu_size;
    *((u_int32_t *) &tspec->ts_min_svc) = tsinfo->min_srv_interval;
    *((u_int32_t *) &tspec->ts_max_svc) = tsinfo->max_srv_interval;
    *((u_int32_t *) &tspec->ts_inactv_intv) = tsinfo->inactivity_interval;
    *((u_int32_t *) &tspec->ts_susp_intv) = tsinfo->suspension_interval;
    *((u_int32_t *) &tspec->ts_start_svc) = tsinfo->srv_start_time;
    *((u_int32_t *) &tspec->ts_min_rate) = tsinfo->min_data_rate;
    *((u_int32_t *) &tspec->ts_mean_rate) = tsinfo->mean_data_rate;
    *((u_int32_t *) &tspec->ts_max_burst) = tsinfo->max_burst_size;
    *((u_int32_t *) &tspec->ts_min_phy) = tsinfo->min_phy_rate;
    *((u_int32_t *) &tspec->ts_peak_rate) = tsinfo->peak_data_rate;
    *((u_int32_t *) &tspec->ts_delay) = tsinfo->delay_bound;
    *((u_int16_t *) &tspec->ts_surplus) = tsinfo->surplus_bw;
    *((u_int16_t *) &tspec->ts_medium_time) = 0;

    ieee80211_send_action(ni, &delts_args, &delts_buf);
    ieee80211_free_node(ni);    /* reclaim node */
    return 0;
}

#endif
