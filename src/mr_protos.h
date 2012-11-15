/* -*- C -*- */
/* I hate this bloody country. Smash. */
/* This file is part of Metaresc project */

#include <metaresc.h>

TYPEDEF_UNION (mr_ptr_t, ATTRIBUTES ( , "pointer on any type"),
	       (void *, ptr, , "default void pointer")
	       )

TYPEDEF_FUNC (string_t, char, , ATTRIBUTES ( , "tricky way to declare type equivalent to char *", .mr_type = MR_TYPE_CHAR_ARRAY))

TYPEDEF_ENUM (mr_log_level_t, ATTRIBUTES ( ,"Log levels enum"),
	      (MR_LL_ALL, = 0),
	      MR_LL_TRACE,
	      MR_LL_DEBUG,
	      MR_LL_INFO,
	      MR_LL_WARN,
	      MR_LL_ERROR,
	      MR_LL_FATAL,
	      MR_LL_OFF,
	      )

TYPEDEF_ENUM (mr_message_id_t, ATTRIBUTES ( , "Messages enum. Message string saved in .comment field of meta info"),
	      (MR_MESSAGE_OUT_OF_MEMORY, , "Out of memory."),
	      (MR_MESSAGE_UNEXPECTED_END, , "Unexpected end of file or end of string."),
	      (MR_MESSAGE_UNEXPECTED_DATA, , "Unexpected data in stream."),
	      (MR_MESSAGE_UNBALANCED_TAGS, , "Unbalanced tags."),
	      (MR_MESSAGE_XDR_WRONG_ENCODING_MODE, , "XDR wrong encoding mode."),
	      (MR_MESSAGE_XDR_SAVE_FAILED, , "XDR save to resizable array failed."),
	      (MR_MESSAGE_XML_SAVE_FAILED, , "XML save failed."),
	      (MR_MESSAGE_LOAD_STRUCT_FAILED, , "Structure load failed."),
	      (MR_MESSAGE_LOAD_METHOD_MISSED, , "Load method missed."),
	      (MR_MESSAGE_XDR_LOAD_FAILED, , "XDR load failed."),
	      (MR_MESSAGE_NO_TYPE_DESCRIPTOR, , "Can't find type descriptor for '%s'."),
	      (MR_MESSAGE_TYPE_NOT_STRUCT, , "Type '%s' is not a struct."),
	      (MR_MESSAGE_TYPE_NOT_UNION, , "Type '%s' is not a union."),
	      (MR_MESSAGE_TYPE_NOT_ENUM, , "Type '%s' is not a enum."),
	      (MR_MESSAGE_STRING_IS_NULL, , "Input string is NULL."),
	      (MR_MESSAGE_NULL_POINTER, , "Can't save/load to/from NULL pointer."),
	      (MR_MESSAGE_IDS_COLLISION, , "Two XML nodes have the same %s = %" SCNd32 "."),
	      (MR_MESSAGE_UNDEFINED_REF_IDX, , "Reference on a node with %s = %" SCNd32 " could not be found."),
	      (MR_MESSAGE_UNKNOWN_ENUM, , "Unknown enum value '%s'."),
	      (MR_MESSAGE_READ_INT, , "Can't read integer value from string `%s`."),
	      (MR_MESSAGE_READ_ENUM, , "Can't read enum value from string `%s`."),
	      (MR_MESSAGE_READ_BITMASK, , "Can't read bitmask from string `%s`."),
	      (MR_MESSAGE_READ_FLOAT, , "Can't read float from string `%s`."),
	      (MR_MESSAGE_READ_DOUBLE, , "Can't read double from string `%s`."),
	      (MR_MESSAGE_READ_LONG_DOUBLE, , "Can't read long double from string `%s`."),
	      (MR_MESSAGE_READ_CHAR, , "Can't read char from string `%s`."),
	      (MR_MESSAGE_STRING_TRUNCATED, , "Char array range check: string truncated."),
	      (MR_MESSAGE_UNKNOWN_SUBNODE, , "Struct type '%s': unknown subnode '%s'."),
	      (MR_MESSAGE_SAVE_IDX_RANGE_CHECK, , "Save node index is out of range."),
	      (MR_MESSAGE_NODE_NAME_MISSMATCH, , "Node name mismatch: expected '%s' got '%s'."),
	      (MR_MESSAGE_NODE_TYPE_MISSMATCH, , "Node type mismatch: expected '%s' got '%s'."),
	      (MR_MESSAGE_UNSUPPORTED_NODE_TYPE, , "Unsupported node type: %s(%d):%s(%d)."),
	      (MR_MESSAGE_RANGE_CHECK, , "Range check error in array '%s'."),
	      (MR_MESSAGE_READ_REF, , "Can't read '%s' as index."),
	      (MR_MESSAGE_READ_RARRAY_SIZE, , "Can't read '%s' as resizable array size."),
	      (MR_MESSAGE_SAVE_ENUM, , "Can't find enum name for value %" SCNd64 " type '%s'."),
	      (MR_MESSAGE_SAVE_BITMASK, , "Can't decompose on bitmask %llu."),
	      (MR_MESSAGE_DUPLICATED_ENUMS, , "Duplicated enum definition for `%s` in enum '%s'."),
	      (MR_MESSAGE_DUPLICATED_FIELDS, , "Duplicated field definition for `%s` in type '%s'."),
	      (MR_MESSAGE_INT_OF_UNKNOWN_SIZE, , "Failed to stringify integer of unknown size: %" SCNd32 "."),
	      (MR_MESSAGE_PARSE_ERROR, , "Parser error: '%s'. Position: %" SCNd32 ":%" SCNd32 "-%" SCNd32 ":%" SCNd32 "."),
	      (MR_MESSAGE_ANON_UNION_TYPE_ERROR, , "Can't create type for anonymous union due to type name collision: '%s'."),
	      (MR_MESSAGE_UNKNOWN_XML_ESC, , "Unknown XML escape sequence '%s'."),
	      (MR_MESSAGE_WRONG_XML_ESC, , "Wrong XML escape sequence '%s'."),
	      (MR_MESSAGE_TYPES_HASHES_MATCHED, , "For types '%s' and '%s' hashes matched. Last type is not available."),
	      (MR_MESSAGE_RARRAY_FAILED, , "Failed to find field .data."),
	      (MR_MESSAGE_UNION_DISCRIMINATOR_ERROR, , "Union failed to find field by discriminator '%s'."),
	      (MR_MESSAGE_XML_STRING_ENCODING_FAILED, , "XML2 encoding failed for string '%s'."),
	      (MR_MESSAGE_LAST, , "Last message ID."),
	      )

TYPEDEF_ENUM (mr_type_t, ATTRIBUTES ( , "Metaresc types"),
	      (MR_TYPE_NONE, = 0),
	      MR_TYPE_STRING,
	      MR_TYPE_CHAR_ARRAY,
	      MR_TYPE_CHAR,
	      MR_TYPE_VOID,
	      MR_TYPE_INT8,
	      MR_TYPE_UINT8,
	      MR_TYPE_INT16,
	      MR_TYPE_UINT16,
	      MR_TYPE_INT32,
	      MR_TYPE_UINT32,
	      MR_TYPE_INT64,
	      MR_TYPE_UINT64,
	      MR_TYPE_FLOAT,
	      MR_TYPE_DOUBLE,
	      MR_TYPE_LONG_DOUBLE,
	      MR_TYPE_STRUCT,
	      MR_TYPE_ENUM,
	      MR_TYPE_FUNC_TYPE,
	      (MR_TYPE_ENUM_VALUE, , "enum_value"), /* comment refers to union member in mr_fd_param_t */
	      (MR_TYPE_FUNC, , "func_param"),
	      (MR_TYPE_BITFIELD, , "bitfield_param"),
	      MR_TYPE_BITMASK,
	      MR_TYPE_UNION,
	      MR_TYPE_ANON_UNION,
	      MR_TYPE_NAMED_ANON_UNION,
	      MR_TYPE_END_ANON_UNION,
	      MR_TYPE_TRAILING_RECORD,
	      MR_TYPE_LAST, /* keep it last */
	      )

TYPEDEF_ENUM (mr_type_ext_t, ATTRIBUTES ( , "Metaresc types extension"),
	      MR_TYPE_EXT_NONE,
	      MR_TYPE_EXT_ARRAY,
	      MR_TYPE_EXT_RARRAY,
	      MR_TYPE_EXT_RARRAY_DATA,
	      MR_TYPE_EXT_POINTER,
	      MR_TYPE_EXT_LAST,  /* keep it last */
	      )

/* mr_rarray_t defenition should be syncroonized with MR_RARRAY_PROTO macro */
TYPEDEF_STRUCT (mr_rarray_t, ATTRIBUTES (__attribute__((packed)), "resizable array type"),
		(void *, data, , "pointer on data array"),
		(int32_t, size, , "used space in bytes"),
		(int32_t, alloc_size, , "allocated space in bytes"),
		(mr_ptr_t, ext, , "ptr_type"), /* extra pointer for user data */
		(char *, ptr_type, , "union discriminator"),
		)

TYPEDEF_ENUM (mr_red_black_t, ATTRIBUTES (__attribute__ ((packed, aligned (sizeof (unsigned char)))), "Red/Black enum"),
	      (MR_BLACK, = 0),
	      (MR_RED, = 1),
	      )

TYPEDEF_STRUCT (mr_red_black_tree_node_t, ATTRIBUTES ( , "red/black tree node"),
		(const void *, key, , "void pointer"),
		(mr_red_black_tree_node_t *, left, , "left child"),
		(mr_red_black_tree_node_t *, right, , "right child"),
		(mr_red_black_t, red),
		)

TYPEDEF_STRUCT (mr_array_param_t, ATTRIBUTES ( , "array parameters"),
		(int, count, , "array size"),
		(int, row_count, , "row size"),
		)

TYPEDEF_STRUCT (mr_bitfield_param_t, ATTRIBUTES ( , "bit-field parameters"),
		(int, width, , "bit-field width in bits"),
		(int, shift, , "bit-field shift in first byte"),
		RARRAY (uint8_t, bitfield, "zero-struct with flagged bit-fields"),
		)

TYPEDEF_UNION (mr_fd_param_t, ATTRIBUTES ( , "optional parameters for different types"),
	       (mr_array_param_t, array_param, , "array parameters - default for serialization"),
	       (int64_t, enum_value, , "enum value"),
	       (mr_bitfield_param_t, bitfield_param, , "bit-field parameters"),
	       RARRAY (struct mr_fd_t, func_param, "function arguments descriptors"),
	       )

TYPEDEF_STRUCT (mr_fd_t, ATTRIBUTES ( , "Metaresc field descriptor"),
		(char *, type, , "stringified type name"),
		(char *, name, , "name of the field"),
		(uint64_t, hash_value, , "hash of the name"),
		(int, offset, , "offset in structure"),
		(int, size, , "size of field"),
		(mr_type_t, mr_type, , "Metaresc type"),
		(mr_type_t, mr_type_aux, , "Metaresc type if field is a pointer on builtin types or bit-field"),
		(mr_type_ext_t, mr_type_ext, , "Metaresc type extension"),
		(mr_fd_param_t, param, , "mr_type"),
		(char *, comment, , "field comments"),
		/*
		  ext field can be used by user for extended information
		  placed after comments for tricky intialization
		  sample:
		  MR_NONE (void *, ext, , "user extended info", { &((ext_info_t){ .field = XXX }) }, "ext_info_t")
		  or
		  MR_NONE (void *, ext, , "user extended info", { (ext_info_t[]){ {.field = XXX}, {.field = YYY} } }, "ext_info_t")
		  or
		  MR_NONE (void *, ext, , "user extended info", { "one more extra string" }, "string_t")
		*/
		(mr_ptr_t, ext, , "ptr_type"), /* extra pointer for user data */
		(char *, ptr_type, , "union discriminator"),
		)

TYPEDEF_STRUCT (mr_fd_ptr_t, ATTRIBUTES ( , "mr_fd_t pointer wrapper"),
		(mr_fd_t *, fdp, , "pointer on field descriptor"),
		)

TYPEDEF_STRUCT (mr_td_t, ATTRIBUTES ( , "Metaresc type descriptor"),
		(mr_type_t, mr_type, , "Metaresc type"), /* possible variants MR_TYPE_ENUM, MR_TYPE_STRUCT, MR_TYPE_UNION */
		(int, size, , "size of type"),
		(char *, type, , "stringified type name"),
		(char *, attr, , "stringified typedef attributes"),
		(mr_type_t, mr_type_effective, , "automatic type detection is required for enums size adjustment"),
		(int, size_effective, , "effective size"),
		(mr_red_black_tree_node_t *, lookup_by_value, , "RB-tree for enums values lookup"),
#ifndef MR_TREE_LOOKUP
		(uint64_t, hash_value, , "type name hash value"),
#endif /* MR_TREE_LOOKUP */
		RARRAY (mr_fd_ptr_t, lookup_by_name, "hash for lookup by field name"),
		RARRAY (mr_fd_t, fields, "fields or enums descriptors"),
		(char *, comment, , "type comments"),
		(mr_ptr_t, ext, , "ptr_type"), /* extra pointer for user data */
		(char *, ptr_type, , "union discriminator"),
		) /* type descriptor */

TYPEDEF_STRUCT (mr_td_ptr_t, ATTRIBUTES ( , "mr_td_t pointer wrapper"),
		(mr_td_t *, tdp, , "pointer on type descriptor"),
		)

TYPEDEF_STRUCT (mr_mem_t, ATTRIBUTES ( , "Metaresc memory operations"),
		(float, mem_alloc_strategy, , "memory allocation strategy"),
		(void *, malloc, (const char *, const char *, int, size_t), "pointer on malloc() function"),
		(void *, realloc, (const char *, const char *, int, void *, size_t), "pointer on realloc() function"),
		(char *, strdup, (const char *, const char *, int, const char *), "pointer on strdup() function"),
		(void, free, (const char *, const char *, int, void *), "pointer on free() function"),
		)

TYPEDEF_STRUCT (mr_ra_mr_td_ptr_t,
		RARRAY (mr_td_ptr_t, ra, "non-collision hash table"),
		)

TYPEDEF_ENUM (mr_bool_t, ATTRIBUTES ( , "boolean type"),
	      (MR_FALSE, = 0),
	      (MR_TRUE, = !0),
	      )

TYPEDEF_STRUCT (mr_ptrdes_flags_t, ATTRIBUTES (__attribute__ ((packed, aligned (sizeof (uint8_t)))), "ponter descriptor flag bitfield values"),
		BITFIELD (mr_bool_t, is_null, :1),
		BITFIELD (mr_bool_t, is_referenced, :1),
		BITFIELD (mr_bool_t, is_content_reference, :1),
		BITFIELD (mr_bool_t, is_opaque_data, :1),
	      )

TYPEDEF_STRUCT (mr_ptrdes_t, ATTRIBUTES ( , "pointer descriptor type"),
		(void *, data, , "pointer on binary data"),
		(mr_fd_t, fd, , "field descriptor"),
		(int, level, , "level from the root element"),
		(int, idx, , "public index"),
		(int, ref_idx, , "reference index (internal enumeration)"),
		(int, parent, , "parent index"),
		(int, first_child, , "first child index"),
		(int, last_child, , "last child index"),
		(int, prev, , "previous sibling index"),
		(int, next, , "next sibling index"),
		(mr_ptrdes_flags_t, flags),
		(char *, union_field_name, , "field descriptor for unions"),
		(char *, value, , "stringified value"),
		(mr_ptr_t, ext, , "ptr_type"), /* extra pointer for user data */
		(char *, ptr_type, , "union discriminator"),
		) /* pointer descriptor */

TYPEDEF_STRUCT (mr_ra_mr_ptrdes_t, ATTRIBUTES ( , "mr_ptrdes_t resizable array"),
		RARRAY (mr_ptrdes_t, ra, "resizable array with mr_ptrdes_t"),
		)

TYPEDEF_STRUCT (mr_save_type_data_t, ATTRIBUTES ( , "serialization of the node"),
		(char *, named_field_template, , "statically allocated string"),
		(char *, prefix, , "statically allocated string"),
		(char *, content, , "dynamically formed string. Need to be freed."),
		(char *, suffix, , "statically allocated string"),
		)

TYPEDEF_STRUCT (mr_lloc_t, ATTRIBUTES ( , "parser location"),
		(int, lineno, , "parser location - line number"),
		(int, column, , "parser location - column number"),
		(int, offset, , "parser location - offset in string"),
		)
  
TYPEDEF_STRUCT (mr_token_lloc_t, ATTRIBUTES ( , "token location"),
		(mr_lloc_t, start, , "start of the token"),
		(mr_lloc_t, end, , "end of the token"),
		)

TYPEDEF_STRUCT (mr_load_t, ATTRIBUTES ( , "Metaresc load parser data"),
		(mr_lloc_t, lloc, , "current location of parser"),
		(char *, str, , "string to parse"),
		(char *, buf, , "parser internal buffer"),
		(int, parent, , "index of current parent"),
		(mr_ra_mr_ptrdes_t *, ptrs, , "resizable array with mr_ptrdes_t"),
		)

TYPEDEF_STRUCT (mr_substr_t, ATTRIBUTES ( , "substring and postprocessor"),
		RARRAY (char, substr, "substring descriptor"),
		(char *, unquote, (char *, int), "unquote handler"),
		)

TYPEDEF_STRUCT (mr_load_data_t,
		(mr_ra_mr_ptrdes_t, ptrs, , "internal representation of a loaded tree"),
		RARRAY (int, mr_ra_idx, "indexes of postponed nodes"),
		)

TYPEDEF_STRUCT (mr_save_data_t, ATTRIBUTES ( , "save routines data and lookup structures"),
		(mr_ra_mr_ptrdes_t, ptrs, , "internal representation of a saved tree"),
		(int, parent, , "index of current parent"),
		(mr_red_black_tree_node_t *, typed_ptrs_tree, , "RB-tree with typed nodes"),
		(mr_red_black_tree_node_t *, untyped_ptrs_tree, , "RB-tree with untyped nodes"),
		RARRAY (int, mr_ra_idx, "indexes of postponed nodes"),
		)

TYPEDEF_STRUCT (mr_load_io_t, ATTRIBUTES ( , "load handlers"),
		(int, rl, (int, mr_load_data_t *), "handler for internal format parsing"),
		(int, xdr, (XDR *, int, mr_ra_mr_ptrdes_t *), "handler for XDR parsing"),
		)

TYPEDEF_STRUCT (mr_save_io_t, ATTRIBUTES ( , "save handlers"),
		(void, rl, (mr_save_data_t *), "handler for internal format"),
		(int, xdr, (XDR *, int, mr_ra_mr_ptrdes_t *), "handler for XDR"),
		(char *, xml, (int, mr_ra_mr_ptrdes_t *), "handler for XML"),
		(char *, xml1, (int, mr_ra_mr_ptrdes_t *), "handler for internaml XML implementation"),
		(char *, xml2, (int, mr_ra_mr_ptrdes_t *), "handler for XML implementation based on libxml2"),
		(int, cinit, (int, mr_ra_mr_ptrdes_t *, mr_save_type_data_t *), "handler for CINIT"),
		(int, json, (int, mr_ra_mr_ptrdes_t *, mr_save_type_data_t *), "handler for JSON"),
		(char *, scm, (int, mr_ra_mr_ptrdes_t *), "handler for SCHEME"),
		)

TYPEDEF_STRUCT (mr_io_handler_t, ATTRIBUTES ( , "input/ouput handlers"),
		(mr_load_io_t, load, , "load handlers"),
		(mr_save_io_t, save, , "save handlers"),
		)

TYPEDEF_FUNC (mr_output_format_t, char *, (mr_ptrdes_t *), ATTRIBUTES ( , "formater handler"))

TYPEDEF_STRUCT (mr_conf_t, ATTRIBUTES ( , "Metaresc configuration"),
		(mr_mem_t, mr_mem, , "memory operations"),
		(mr_log_level_t, log_level),
		(void, msg_handler, (const char *, const char *, int, mr_log_level_t, mr_message_id_t, va_list), "handler for error messages"),
#ifndef MR_TREE_LOOKUP
		(mr_ra_mr_td_ptr_t, hash, , "hash for type descriptors lookup"),
#else /* MR_TREE_LOOKUP */
		(mr_red_black_tree_node_t *, tree, , "RB-tree for type descriptors lookup"),
#endif /* MR_TREE_LOOKUP */
		RARRAY (mr_td_ptr_t, des, "types descriptors"),
		(mr_red_black_tree_node_t *, enum_by_name, , "RB-tree with enums mapping"),
		NONE (mr_output_format_t, output_format, [MR_MAX_TYPES], "formaters"),
		NONE (mr_io_handler_t, io_handlers, [MR_MAX_TYPES], "io handlers"),
		NONE (mr_io_handler_t, io_ext_handlers, [MR_MAX_TYPES], "io handlers"),
		)
