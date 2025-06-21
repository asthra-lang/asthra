/**
 * Asthra Programming Language Compiler
 * DWARF Debug Information Constants
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * DWARF constants for debug information generation.
 */

#ifndef ASTHRA_DWARF_CONSTANTS_H
#define ASTHRA_DWARF_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DWARF LANGUAGE CODES
// =============================================================================

// Standard DWARF language codes
#define DW_LANG_C89             0x0001
#define DW_LANG_C               0x0002
#define DW_LANG_Ada83           0x0003
#define DW_LANG_C_plus_plus     0x0004
#define DW_LANG_Cobol74         0x0005
#define DW_LANG_Cobol85         0x0006
#define DW_LANG_Fortran77       0x0007
#define DW_LANG_Fortran90       0x0008
#define DW_LANG_Pascal83        0x0009
#define DW_LANG_Modula2         0x000a
#define DW_LANG_Java            0x000b
#define DW_LANG_C99             0x000c
#define DW_LANG_Ada95           0x000d
#define DW_LANG_Fortran95       0x000e
#define DW_LANG_PLI             0x000f
#define DW_LANG_ObjC            0x0010
#define DW_LANG_ObjC_plus_plus  0x0011
#define DW_LANG_UPC             0x0012
#define DW_LANG_D               0x0013
#define DW_LANG_Python          0x0014
#define DW_LANG_Rust            0x001c
#define DW_LANG_C11             0x001d
#define DW_LANG_Swift           0x001e
#define DW_LANG_C_plus_plus_14  0x0021
#define DW_LANG_Fortran03       0x0022
#define DW_LANG_Fortran08       0x0023

// Custom language code for Asthra (using vendor extension range)
#define DWARF_LANG_ASTHRA       0x8000

// =============================================================================
// DWARF TAG CODES
// =============================================================================

#define DW_TAG_array_type               0x01
#define DW_TAG_class_type               0x02
#define DW_TAG_entry_point              0x03
#define DW_TAG_enumeration_type         0x04
#define DW_TAG_formal_parameter         0x05
#define DW_TAG_imported_declaration     0x08
#define DW_TAG_label                    0x0a
#define DW_TAG_lexical_block            0x0b
#define DW_TAG_member                   0x0d
#define DW_TAG_pointer_type             0x0f
#define DW_TAG_reference_type           0x10
#define DW_TAG_compile_unit             0x11
#define DW_TAG_string_type              0x12
#define DW_TAG_structure_type           0x13
#define DW_TAG_subroutine_type          0x15
#define DW_TAG_typedef                  0x16
#define DW_TAG_union_type               0x17
#define DW_TAG_unspecified_parameters   0x18
#define DW_TAG_variant                  0x19
#define DW_TAG_common_block             0x1a
#define DW_TAG_common_inclusion         0x1b
#define DW_TAG_inheritance              0x1c
#define DW_TAG_inlined_subroutine       0x1d
#define DW_TAG_module                   0x1e
#define DW_TAG_ptr_to_member_type       0x1f
#define DW_TAG_set_type                 0x20
#define DW_TAG_subrange_type            0x21
#define DW_TAG_with_stmt                0x22
#define DW_TAG_access_declaration       0x23
#define DW_TAG_base_type                0x24
#define DW_TAG_catch_block              0x25
#define DW_TAG_const_type               0x26
#define DW_TAG_constant                 0x27
#define DW_TAG_enumerator               0x28
#define DW_TAG_file_type                0x29
#define DW_TAG_friend                   0x2a
#define DW_TAG_namelist                 0x2b
#define DW_TAG_namelist_item            0x2c
#define DW_TAG_packed_type              0x2d
#define DW_TAG_subprogram               0x2e
#define DW_TAG_template_type_parameter  0x2f
#define DW_TAG_template_value_parameter 0x30
#define DW_TAG_thrown_type              0x31
#define DW_TAG_try_block                0x32
#define DW_TAG_variant_part             0x33
#define DW_TAG_variable                 0x34
#define DW_TAG_volatile_type            0x35
#define DW_TAG_dwarf_procedure          0x36
#define DW_TAG_restrict_type            0x37
#define DW_TAG_interface_type           0x38
#define DW_TAG_namespace                0x39
#define DW_TAG_imported_module          0x3a
#define DW_TAG_unspecified_type         0x3b
#define DW_TAG_partial_unit             0x3c
#define DW_TAG_imported_unit            0x3d
#define DW_TAG_condition                0x3f
#define DW_TAG_shared_type              0x40
#define DW_TAG_type_unit                0x41
#define DW_TAG_rvalue_reference_type    0x42
#define DW_TAG_template_alias           0x43

// =============================================================================
// DWARF ATTRIBUTE TYPE ENCODINGS (DW_ATE_*)
// =============================================================================

#define DW_ATE_address          0x01
#define DW_ATE_boolean          0x02
#define DW_ATE_complex_float    0x03
#define DW_ATE_float            0x04
#define DW_ATE_signed           0x05
#define DW_ATE_signed_char      0x06
#define DW_ATE_unsigned         0x07
#define DW_ATE_unsigned_char    0x08
#define DW_ATE_imaginary_float  0x09
#define DW_ATE_packed_decimal   0x0a
#define DW_ATE_numeric_string   0x0b
#define DW_ATE_edited           0x0c
#define DW_ATE_signed_fixed     0x0d
#define DW_ATE_unsigned_fixed   0x0e
#define DW_ATE_decimal_float    0x0f
#define DW_ATE_UTF              0x10
#define DW_ATE_UCS              0x11
#define DW_ATE_ASCII            0x12

// =============================================================================
// DWARF OPERATION CODES (DW_OP_*)
// =============================================================================

// Literal encodings
#define DW_OP_addr              0x03
#define DW_OP_deref             0x06
#define DW_OP_const1u           0x08
#define DW_OP_const1s           0x09
#define DW_OP_const2u           0x0a
#define DW_OP_const2s           0x0b
#define DW_OP_const4u           0x0c
#define DW_OP_const4s           0x0d
#define DW_OP_const8u           0x0e
#define DW_OP_const8s           0x0f
#define DW_OP_constu            0x10
#define DW_OP_consts            0x11
#define DW_OP_dup               0x12
#define DW_OP_drop              0x13
#define DW_OP_over              0x14
#define DW_OP_pick              0x15
#define DW_OP_swap              0x16
#define DW_OP_rot               0x17
#define DW_OP_xderef            0x18
#define DW_OP_abs               0x19
#define DW_OP_and               0x1a
#define DW_OP_div               0x1b
#define DW_OP_minus             0x1c
#define DW_OP_mod               0x1d
#define DW_OP_mul               0x1e
#define DW_OP_neg               0x1f
#define DW_OP_not               0x20
#define DW_OP_or                0x21
#define DW_OP_plus              0x22
#define DW_OP_plus_uconst       0x23
#define DW_OP_shl               0x24
#define DW_OP_shr               0x25
#define DW_OP_shra              0x26
#define DW_OP_xor               0x27
#define DW_OP_skip              0x2f
#define DW_OP_bra               0x28
#define DW_OP_eq                0x29
#define DW_OP_ge                0x2a
#define DW_OP_gt                0x2b
#define DW_OP_le                0x2c
#define DW_OP_lt                0x2d
#define DW_OP_ne                0x2e

// Register location descriptions
#define DW_OP_reg0              0x50
#define DW_OP_reg1              0x51
#define DW_OP_reg2              0x52
#define DW_OP_reg3              0x53
#define DW_OP_reg4              0x54
#define DW_OP_reg5              0x55
#define DW_OP_reg6              0x56
#define DW_OP_reg7              0x57
#define DW_OP_reg8              0x58
#define DW_OP_reg9              0x59
#define DW_OP_reg10             0x5a
#define DW_OP_reg11             0x5b
#define DW_OP_reg12             0x5c
#define DW_OP_reg13             0x5d
#define DW_OP_reg14             0x5e
#define DW_OP_reg15             0x5f
#define DW_OP_reg16             0x60
#define DW_OP_reg17             0x61
#define DW_OP_reg18             0x62
#define DW_OP_reg19             0x63
#define DW_OP_reg20             0x64
#define DW_OP_reg21             0x65
#define DW_OP_reg22             0x66
#define DW_OP_reg23             0x67
#define DW_OP_reg24             0x68
#define DW_OP_reg25             0x69
#define DW_OP_reg26             0x6a
#define DW_OP_reg27             0x6b
#define DW_OP_reg28             0x6c
#define DW_OP_reg29             0x6d
#define DW_OP_reg30             0x6e
#define DW_OP_reg31             0x6f

// Base register relative location descriptions
#define DW_OP_breg0             0x70
#define DW_OP_breg1             0x71
#define DW_OP_breg2             0x72
#define DW_OP_breg3             0x73
#define DW_OP_breg4             0x74
#define DW_OP_breg5             0x75
#define DW_OP_breg6             0x76
#define DW_OP_breg7             0x77
#define DW_OP_breg8             0x78
#define DW_OP_breg9             0x79
#define DW_OP_breg10            0x7a
#define DW_OP_breg11            0x7b
#define DW_OP_breg12            0x7c
#define DW_OP_breg13            0x7d
#define DW_OP_breg14            0x7e
#define DW_OP_breg15            0x7f
#define DW_OP_breg16            0x80
#define DW_OP_breg17            0x81
#define DW_OP_breg18            0x82
#define DW_OP_breg19            0x83
#define DW_OP_breg20            0x84
#define DW_OP_breg21            0x85
#define DW_OP_breg22            0x86
#define DW_OP_breg23            0x87
#define DW_OP_breg24            0x88
#define DW_OP_breg25            0x89
#define DW_OP_breg26            0x8a
#define DW_OP_breg27            0x8b
#define DW_OP_breg28            0x8c
#define DW_OP_breg29            0x8d
#define DW_OP_breg30            0x8e
#define DW_OP_breg31            0x8f

// Stack operations
#define DW_OP_regx              0x90
#define DW_OP_fbreg             0x91
#define DW_OP_bregx             0x92
#define DW_OP_piece             0x93
#define DW_OP_deref_size        0x94
#define DW_OP_xderef_size       0x95
#define DW_OP_nop               0x96
#define DW_OP_push_object_address 0x97
#define DW_OP_call2             0x98
#define DW_OP_call4             0x99
#define DW_OP_call_ref          0x9a
#define DW_OP_form_tls_address  0x9b
#define DW_OP_call_frame_cfa    0x9c
#define DW_OP_bit_piece         0x9d
#define DW_OP_implicit_value    0x9e
#define DW_OP_stack_value       0x9f

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_DWARF_CONSTANTS_H