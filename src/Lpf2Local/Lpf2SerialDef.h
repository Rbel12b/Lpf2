#pragma once
#ifndef _LPF2_SERIAL_DEFINITIONS_H_
#define _LPF2_SERIAL_DEFINITIONS_H_

#define   BYTE_SYNC                     0x00                            // Synchronisation byte
#define   BYTE_ACK                      0x04                            // Acknowledge byte
#define   BYTE_NACK                     0x02                            // Not acknowledge byte (keep-alive)

#define   MESSAGE_SYS                   0x00                            // System message
#define   MESSAGE_CMD                   0x40                            // Command message
#define   MESSAGE_INFO                  0x80                            // Info message
#define   MESSAGE_DATA                  0xC0                            // Data message
#define   GET_MESSAGE_TYPE(B)           (B & 0xC0)                      // Get message type

#define   CMD_TYPE                      0x00                            // CMD command - TYPE     (device type for VM reference)
#define   CMD_MODES                     0x01                            // CMD command - MODES    (number of supported modes 0=1)
#define   CMD_SPEED                     0x02                            // CMD command - SPEED    (maximun communication speed)
#define   CMD_SELECT                    0x03                            // CMD command - SELECT   (select mode)
#define   CMD_WRITE                     0x04                            // CMD command - WRITE    (write to device)
#define   CMD_EXT_MODE                  0x06                            // CMD command - EXT_MODE (value will be added to mode in CMD_WRITE_DATA - LPF2 only)
#define   CMD_VERSION                   0x07                            // CMD command - VERSION  (device firmware and hardware versions)
#define   GET_CMD_COMMAND(B)            (B & 0x07)                      // Get CMD command

#define   GET_MODE(B)                   (B & 0x07)                      // Get mode

#define   GET_MESSAGE_LENGTH(B)         (1 << ((B >> 3) & 0x07))

#define   MAKE_CMD_COMMAND(C,LC)        (MESSAGE_CMD + (C & 0x07) + ((LC & 0x07) << 3))

// SECOND INFO BYTE

#define   INFO_NAME                     0x00    // INFO command - NAME    (device name)
#define   INFO_RAW                      0x01    // INFO command - RAW     (device RAW value span)
#define   INFO_PCT                      0x02    // INFO command - PCT     (device PCT value span)
#define   INFO_SI                       0x03    // INFO command - SI      (device SI  value span)
#define   INFO_UNITS                    0x04    // INFO command - UNITS   (device SI  unit symbol)
#define   INFO_MAPPING                  0x05    // INFO command - MAPPING (input/output value type flags)
#define   INFO_MODE_COMBOS              0x06    // INFO command - COMBOS  (mode combinations - LPF2-only)
#define   INFO_UNK7                     0x07    // INFO command - unknown (LPF2-only)
#define   INFO_UNK8                     0x08    // INFO command - unknown (LPF2-only)
#define   INFO_UNK9                     0x09    // INFO command - unknown (LPF2-only)
#define   INFO_UNK10                    0x0a    // INFO command - unknown (LPF2-only)
#define   INFO_UNK11                    0x0b    // INFO command - unknown (LPF2-only)
#define   INFO_UNK12                    0x0c    // INFO command - unknown (LPF2-only)
#define   INFO_MODE_PLUS_8              0x20    // Bit flag used in powered up devices to indicate that the mode is 8 + the mode specified in the first byte
#define   INFO_FORMAT                   0x80    // INFO command - FORMAT  (device data sets and format)

#define   LENGTH_1                      (0x00 << 3)
#define   LENGTH_2                      (0x01 << 3)
#define   LENGTH_4                      (0x02 << 3)
#define   LENGTH_8                      (0x03 << 3)
#define   LENGTH_16                     (0x04 << 3)
#define   LENGTH_32                     (0x05 << 3)
#define   LENGTH_64                     (0x06 << 3)
#define   LENGTH_128                    (0x07 << 3)

#endif