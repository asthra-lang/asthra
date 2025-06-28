/**
 * DWARF Constants for ELF Writer Tests
 * 
 * Minimal set of DWARF constants needed for ELF writer testing.
 * Based on DWARF debugging format standards.
 */

#ifndef DWARF_CONSTANTS_H
#define DWARF_CONSTANTS_H

// DWARF Tag constants
#define DW_TAG_base_type        0x24
#define DW_TAG_pointer_type     0x0f
#define DW_TAG_array_type       0x01
#define DW_TAG_structure_type   0x13
#define DW_TAG_union_type       0x17

// DWARF Attribute Type Encoding constants
#define DW_ATE_void             0x00
#define DW_ATE_address          0x01
#define DW_ATE_boolean          0x02
#define DW_ATE_complex_float    0x03
#define DW_ATE_float            0x04
#define DW_ATE_signed           0x05
#define DW_ATE_signed_char      0x06
#define DW_ATE_unsigned         0x07
#define DW_ATE_unsigned_char    0x08

// DWARF Operation constants
#define DW_OP_fbreg             0x91
#define DW_OP_reg0              0x50
#define DW_OP_reg1              0x51

#endif // DWARF_CONSTANTS_H