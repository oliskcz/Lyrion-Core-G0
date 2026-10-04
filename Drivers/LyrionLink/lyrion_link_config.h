/**
 * @file  lyrion_link_config.h
 * @brief Lyrion Link — compile-time configuration defaults.
 *
 * Each #define can be overridden in the project's Core/Inc/config.h
 * (via -D on the command line or #define before #include).
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef LYRION_LINK_CONFIG_H
#define LYRION_LINK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/*  Build tier                                                         */
/* ------------------------------------------------------------------ */
#ifndef LL_LITE_BUILD
#define LL_LITE_BUILD           0   /* 1 = STM32C031 Lite (leaf node)      */
#endif
#ifndef LL_PRO_BUILD
#define LL_PRO_BUILD            1   /* 1 = STM32G071 Pro (mesh + file xfer)*/
#endif

/* ------------------------------------------------------------------ */
/*  Protocol version                                                   */
/* ------------------------------------------------------------------ */
#ifndef LL_VERSION
#define LL_VERSION              0x01
#endif

/* ------------------------------------------------------------------ */
/*  Packet geometry (match CC1101 64-byte FIFO)                        */
/* ------------------------------------------------------------------ */
/* 13 header + 46 payload + 4 MAC = 63 bytes (1 byte spare in FIFO).  */
#ifndef LL_HEADER_SIZE
#define LL_HEADER_SIZE          15
#endif
#ifndef LL_MAX_PAYLOAD
#define LL_MAX_PAYLOAD          44
#endif
#ifndef LL_MAC_SIZE
#define LL_MAC_SIZE              4
#endif

/* ------------------------------------------------------------------ */
/*  Crypto sizes                                                       */
/* ------------------------------------------------------------------ */
#ifndef LL_NONCE_SIZE
#define LL_NONCE_SIZE           13
#endif
#ifndef LL_PREFIX_SIZE
#define LL_PREFIX_SIZE           7     /* bytes in AES_NONCE_PREFIX     */
#endif
#ifndef LL_KEY_SIZE
#define LL_KEY_SIZE             16
#endif

/* ------------------------------------------------------------------ */
/*  Network                                                            */
/* ------------------------------------------------------------------ */
#ifndef LL_NETWORK_ID_SIZE
#define LL_NETWORK_ID_SIZE       2
#endif
#ifndef LL_ADDRESS_SIZE
#define LL_ADDRESS_SIZE          2
#endif
#ifndef LL_COUNTER_SIZE
#define LL_COUNTER_SIZE          4
#endif
#ifndef LL_PACKET_ID_SIZE
#define LL_PACKET_ID_SIZE        2
#endif


/* ------------------------------------------------------------------ */
/*  Security (Phase 2) — per-sender replay map                         */
/* ------------------------------------------------------------------ */
#ifndef LL_LARGE_JUMP_THRESHOLD
#define LL_LARGE_JUMP_THRESHOLD 50    /* counter gap indicating sender reboot */
#endif
/* ------------------------------------------------------------------ */
/*  Security (Phase 2) — per-sender replay map                         */
/* ------------------------------------------------------------------ */
#ifndef LL_MAX_NODES
#define LL_MAX_NODES            32    /* max tracked senders on Lite   */
#endif
#ifndef LL_PKTID_RING_SIZE
#define LL_PKTID_RING_SIZE       4    /* PacketID ring depth per src   */
#endif

/* ------------------------------------------------------------------ */
/*  MAC layer (Phase 4) — CSMA/CA                                      */
/* ------------------------------------------------------------------ */
#ifndef LL_ACK_TIMEOUT_MS
#define LL_ACK_TIMEOUT_MS      500
#endif
#ifndef LL_RETRY_COUNT
#define LL_RETRY_COUNT           3
#endif
#ifndef LL_BACKOFF_SLOTS
#define LL_BACKOFF_SLOTS        16    /* 0 .. 15 random backoff slots  */
#endif
#ifndef LL_BACKOFF_SLOT_MS
#define LL_BACKOFF_SLOT_MS       1
#endif
#ifndef LL_MAC_RSSI_THRESHOLD
#define LL_MAC_RSSI_THRESHOLD  (-55)  /* dBm: below = clear channel    */
#endif

/* ------------------------------------------------------------------ */
/*  TX queue                                                           */
/* ------------------------------------------------------------------ */
#ifndef LL_TX_QUEUE_SIZE
#define LL_TX_QUEUE_SIZE         1     /* Lite: 1 pending outgoing msg  */
#endif

/* ------------------------------------------------------------------ */
/*  Fragmentation (Phase 5)                                            */
/* ------------------------------------------------------------------ */
#ifndef LL_FRAG_RX_BUF_SIZE
#define LL_FRAG_RX_BUF_SIZE   256
#endif

/* ------------------------------------------------------------------ */
/*  Beacon (Phase 6)                                                   */
/* ------------------------------------------------------------------ */
#ifndef LL_BEACON_DEFAULT_S
#define LL_BEACON_DEFAULT_S      0     /* 0 = disabled by default       */
#endif

/* ------------------------------------------------------------------ */
/*  Mesh (Phase 3, Pro only)                                           */
/* ------------------------------------------------------------------ */
#ifndef LL_MESH_DEFAULT
#define LL_MESH_DEFAULT          0     /* star topology by default      */
#endif

#ifdef __cplusplus
}
#endif
#endif /* LYRION_LINK_CONFIG_H */
