/* -*- C -*- */
/* I hate this bloody country. Smash. */
/* This file is part of Metaresc project */

#ifdef HAVE_CONFIG_H
#include <mr_config.h>
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#include <string.h>
#ifdef HAVE_DLFCN_H
#define __USE_GNU
#include <dlfcn.h>
#endif /* HAVE_DLFCN_H */

#include <metaresc.h>
#include <mr_ic.h>
#include <mr_load.h>

/**
 * Post load references setting. If node was marked as references
 * it should be substitude with actual pointer. This substition
 * can't be made during structure loading because of forward references.
 * @param mr_load_data structures that holds context of loading
 * @return Status. !0 - ok. 0 - rl nodes indexes colision
 */
static int
mr_set_crossrefs (mr_load_data_t * mr_load_data)
{
  int i;
  int count = mr_load_data->ptrs.ra.size / sizeof (mr_load_data->ptrs.ra.data[0]);
  int max_idx = -1;
  int * table;

  for (i = 0; i < count; ++i)
    if (mr_load_data->ptrs.ra.data[i].idx > max_idx)
      max_idx = mr_load_data->ptrs.ra.data[i].idx;

  if (max_idx < 0)
    return (!0);

  table = (int*)MR_MALLOC (sizeof (int) * (max_idx + 1));
  if (NULL == table)
    {
      MR_MESSAGE (MR_LL_FATAL, MR_MESSAGE_OUT_OF_MEMORY);
      return (0);
    }

  for (i = 0; i <= max_idx; ++i)
    table[i] = -1;
  for (i = 0; i < count; ++i)
    if (mr_load_data->ptrs.ra.data[i].idx >= 0)
      {
	/* checking indexes collision */
	if (table[mr_load_data->ptrs.ra.data[i].idx] >= 0)
	  MR_MESSAGE (MR_LL_WARN, MR_MESSAGE_IDS_COLLISION, MR_REF_IDX, mr_load_data->ptrs.ra.data[i].idx);
	table[mr_load_data->ptrs.ra.data[i].idx] = i;
      }

  /* set all cross refernces */
  for (i = 0; i < count; ++i)
    if (mr_load_data->ptrs.ra.data[i].ref_idx >= 0)
      {
	int idx = table[mr_load_data->ptrs.ra.data[i].ref_idx];
	if (idx < 0)
	  MR_MESSAGE (MR_LL_WARN, MR_MESSAGE_UNDEFINED_REF_IDX, MR_REF_IDX, mr_load_data->ptrs.ra.data[i].ref_idx);
	else
	  {
	    void * data;
	    if (mr_load_data->ptrs.ra.data[i].flags.is_content_reference)
	      data = *(void**)(mr_load_data->ptrs.ra.data[idx].data);
	    else
	      data = mr_load_data->ptrs.ra.data[idx].data;
	    
	    if ((MR_TYPE_EXT_POINTER == mr_load_data->ptrs.ra.data[i].fd.mr_type_ext) ||
		(MR_TYPE_EXT_RARRAY_DATA == mr_load_data->ptrs.ra.data[i].fd.mr_type_ext) ||
		(MR_TYPE_STRING == mr_load_data->ptrs.ra.data[i].fd.mr_type))
	      *(void**)mr_load_data->ptrs.ra.data[i].data = data;
	  }
      }
  MR_FREE (table);
  return (!0);
}

/**
 * Read enum value from string
 * @param data pointer on place to save value
 * @param str string with enum
 * @return A pointer on the rest of parsed string
 */
static char *
mr_get_enum (uint64_t * data, char * str)  
{
  char * name = str;
  int size;
	  
  while (isalnum (*str) || (*str == '_'))
    ++str;
  size = str - name;

  {
    char name_[size + 1];
    mr_fd_t * fdp;
    memcpy (name_, name, size);
    name_[size] = 0;

    fdp = mr_get_enum_by_name (name_);
    if (fdp)
      {
	*data = fdp->param.enum_value;
	return (str);
      }

    MR_MESSAGE (MR_LL_WARN, MR_MESSAGE_UNKNOWN_ENUM, name_);
    return (NULL);
  }
}

/**
 * Read int value from string (may be as ENUM)
 * @param data pointer on place to save int
 * @param str string with int
 * @return A pointer on the rest of parsed string
 */
static char *
mr_get_int (uint64_t * data, char * str)
{
  int offset;
  while (isspace (*str))
    ++str;
  if (isalpha (*str))
    str = mr_get_enum (data, str);
  else if ('0' == *str)
    {
      if ('x' == str[1])
	{
	  if (1 != sscanf (str, "%" SCNx64 "%n", data, &offset))
	    {
	      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, str);
	      return (NULL);
	    }
	}
      else
	{
	  if (1 != sscanf (str, "%" SCNo64 "%n", data, &offset))
	    {
	      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, str);
	      return (NULL);
	    }
	}	
      str += offset;
    }
  else
    {
      if ((1 == sscanf (str, "%" SCNu64 "%n", data, &offset)) || (1 == sscanf (str, "%" SCNd64 "%n", data, &offset)))
	str += offset;
      else 
	{
	  MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, str);
	  return (NULL);
	}
    }
  return (str);
}

/**
 * MR_NONE load handler (dummy)
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_none (int idx, mr_load_data_t * mr_load_data)
{
  return (!0);
}

/**
 * MR_INTEGER load handler
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_integer (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  uint64_t value;
  
  if (NULL == str)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, mr_load_data->ptrs.ra.data[idx].value);
      return (0);
    }
  str = mr_get_int (&value, str);
  if (str)
    {
      while (isspace (*str))
	++str;
    }
  if ((NULL == str) || (*str != 0))
    {
      if (str)
	MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, mr_load_data->ptrs.ra.data[idx].value);
      return (0);
    }
  memcpy (mr_load_data->ptrs.ra.data[idx].data, &value, mr_load_data->ptrs.ra.data[idx].fd.size);
  return (!0);
}

/**
 * MR_ENUM load handler
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
int
mr_load_enum (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  char * tail;
  mr_td_t * tdp = mr_get_td_by_name (mr_load_data->ptrs.ra.data[idx].fd.type);
  mr_fd_t * fdp;

  if (NULL == str)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_ENUM, mr_load_data->ptrs.ra.data[idx].value);
      return (0);
    }
  if (tdp)
    {
      while (isspace (*str))
	++str;
      tail = &str[strlen (str) - 1];
      while ((tail > str) && isspace (*tail))
	--tail;
      *++tail = 0;
      fdp = mr_get_fd_by_name (tdp, str);
      if (fdp)
	{
	  memcpy (mr_load_data->ptrs.ra.data[idx].data, &fdp->param.enum_value, mr_load_data->ptrs.ra.data[idx].fd.size);
	  return (!0);
	}
    }
  return (mr_load_integer (idx, mr_load_data));
}

/**
 * MR_BITFIELD load handler. Load int from string and save it to
 * bit shifted field.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_bitfield (int idx, mr_load_data_t * mr_load_data)
{
  mr_ptrdes_t * ptrdes = &mr_load_data->ptrs.ra.data[idx];
  uint64_t value = 0;
  char * str = mr_get_int (&value, ptrdes->value);

  if ((NULL == str) || (*str != 0))
    {
      if (str)
	MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_INT, ptrdes->value);
      return (0);
    }

  return (EXIT_SUCCESS == mr_load_bitfield_value (ptrdes, &value));
}

/**
 * MR_BITMASK load handler. Handles logical OR operation.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
int
mr_load_bitmask (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  int64_t value = 0;
  
  if (NULL == str)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_BITMASK, mr_load_data->ptrs.ra.data[idx].value);
      return (0);
    }
  for (;;)
    {
      uint64_t bit;
      str = mr_get_int (&bit, str);
      if (NULL == str)
	return (0);
      value |= bit;
      while (isspace (*str))
	++str;
      if (*str != '|')
	break;
      ++str;
    }
  if (*str != 0)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_BITMASK, mr_load_data->ptrs.ra.data[idx].value);
      return (0);
    }
  memcpy (mr_load_data->ptrs.ra.data[idx].data, &value, mr_load_data->ptrs.ra.data[idx].fd.size);
  return (!0);
}

/**
 * MR_FLOAT, MR_DOUBLE, MR_LONG_DOUBLE load handler.
 */
#define MR_LOAD_FLOAT_TYPE(TYPE, FORMAT, ERROR_ENUM)			\
  static int								\
  mr_load_ ## TYPE (int idx, mr_load_data_t * mr_load_data)		\
  {									\
    char * str = mr_load_data->ptrs.ra.data[idx].value;			\
    int offset;								\
    if (NULL == str)							\
      {									\
	MR_MESSAGE (MR_LL_ERROR, ERROR_ENUM, mr_load_data->ptrs.ra.data[idx].value); \
	return (0);							\
      }									\
    if (1 != sscanf (str, FORMAT "%n", (TYPE*)mr_load_data->ptrs.ra.data[idx].data, &offset)) \
      {									\
	MR_MESSAGE (MR_LL_ERROR, ERROR_ENUM, mr_load_data->ptrs.ra.data[idx].value); \
	return (0);							\
      }									\
    str += offset;							\
    while (isspace (*str))						\
      ++str;								\
    if (*str != 0)							\
      {									\
	MR_MESSAGE (MR_LL_ERROR, ERROR_ENUM, mr_load_data->ptrs.ra.data[idx].value); \
	return (0);							\
      }									\
    return (!0);							\
  }

MR_LOAD_FLOAT_TYPE (float, "%f", MR_MESSAGE_READ_FLOAT)
MR_LOAD_FLOAT_TYPE (double, "%lg", MR_MESSAGE_READ_DOUBLE)
MR_LOAD_FLOAT_TYPE (long_double_t, "%Lg", MR_MESSAGE_READ_LONG_DOUBLE)

/**
 * MR_CHAR load handler. Handles nonprint characters in octal format.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_char (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  
  if (NULL == str)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_CHAR, str);
      return (0);
    }
  else if ((0 == *str) || (0 == str[1]))
    *(char*)mr_load_data->ptrs.ra.data[idx].data = str[0];
  else if ('\\' != *str)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_CHAR, str);
      return (0);
    }
  else
    {
      int offset;
      int val = 0;
      if (1 != sscanf (str + 1, "%o%n", &val, &offset))
	{
	  MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_CHAR, str);
	  return (0);
	}
      else if (str[offset + 1] != 0)
	{
	  MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_READ_CHAR, str);
	  return (0);
	}
      *(char*)mr_load_data->ptrs.ra.data[idx].data = val;
    }
  return (!0);
}     

/**
 * MR_STRING load handler. Allocate memory for a string.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_string (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  if ((mr_load_data->ptrs.ra.data[idx].flags.is_null) || (mr_load_data->ptrs.ra.data[idx].ref_idx >= 0))
    *(char**)mr_load_data->ptrs.ra.data[idx].data = NULL;
  else
    *(char**)mr_load_data->ptrs.ra.data[idx].data = str ? MR_STRDUP (str) : NULL;
  return (!0);
}

/**
 * MR_CHAR_ARRAY load handler.
 * Save string in place (truncate string if needed).
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_char_array (int idx, mr_load_data_t * mr_load_data)
{
  char * str = mr_load_data->ptrs.ra.data[idx].value;
  int max_size = mr_load_data->ptrs.ra.data[idx].fd.param.array_param.count * mr_load_data->ptrs.ra.data[idx].fd.size;
  if (str)
    {
      int str_len = strlen (str);
      if ((0 == strcmp (mr_load_data->ptrs.ra.data[idx].fd.type, "string_t")) &&
	  (mr_load_data->ptrs.ra.data[idx].parent >= 0) &&
	  (MR_TYPE_EXT_POINTER == mr_load_data->ptrs.ra.data[mr_load_data->ptrs.ra.data[idx].parent].fd.mr_type_ext))
	{
	  void * data = MR_REALLOC (mr_load_data->ptrs.ra.data[idx].data, str_len + 1);
	  mr_load_data->ptrs.ra.data[idx].data = data;
	  *(void**)mr_load_data->ptrs.ra.data[mr_load_data->ptrs.ra.data[idx].parent].data = data;
	  if (NULL == data)
	    {
	      MR_MESSAGE (MR_LL_FATAL, MR_MESSAGE_OUT_OF_MEMORY);
	      return (0);
	    }
	}
      else if (str_len >= max_size)
	{
	  str[max_size - 1] = 0;
	  MR_MESSAGE (MR_LL_WARN, MR_MESSAGE_STRING_TRUNCATED);
	}
      strcpy (mr_load_data->ptrs.ra.data[idx].data, str);
    }
  else
    *(char*)mr_load_data->ptrs.ra.data[idx].data = 0;
  return (!0);
}

/**
 * MR_TYPE_FUNC & MR_TYPE_FUNC_TYPE load handler.
 * Pointer to function loader. It might be a symbol name or an integer casted to void*
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
int
mr_load_func (int idx, mr_load_data_t * mr_load_data)
{
  char * value = mr_load_data->ptrs.ra.data[idx].value;
  void * func = NULL;

  *(void**)mr_load_data->ptrs.ra.data[idx].data = NULL;

  if (MR_TRUE == mr_load_data->ptrs.ra.data[idx].flags.is_null)
    return (!0);
  if (NULL == value)
    return (!0);
  if (0 == value[0])
    return (!0);
  
  if (isdigit (value[0]))
    return (mr_load_integer (idx, mr_load_data));

#ifdef HAVE_LIBDL
  func = dlsym (RTLD_DEFAULT, value);
  if (NULL == func)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_LOAD_FUNC_FAILED, mr_load_data->ptrs.ra.data[idx].fd.name.str);
      return (0);
    }
  *(void**)mr_load_data->ptrs.ra.data[idx].data = func;
  return (!0);
#else /* ! HAVE_LIBDL */
  return (0);
#endif /* HAVE_LIBDL */

}

static mr_fd_t *
mr_load_struct_next_field (mr_td_t * tdp, mr_fd_t * fdp)
{
  int i, count = tdp->fields.size / sizeof (tdp->fields.data[0]);

  for (i = 0; i < count; ++i)
    if (NULL == fdp)
      return (tdp->fields.data[i].fdp);
    else if (tdp->fields.data[i].fdp == fdp)
      fdp = NULL;
      
  return (NULL);
}

/**
 * MR_STRUCT load handler.
 * Save content of subnodes to structure fileds.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @param tdp type descriptor
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_struct_inner (int idx, mr_load_data_t * mr_load_data, mr_td_t * tdp)
{
  char * data = mr_load_data->ptrs.ra.data[idx].data;
  int first_child = mr_load_data->ptrs.ra.data[idx].first_child;
  mr_fd_t * fdp = NULL;
  
  /* get pointer on structure descriptor */
  if (NULL == tdp)
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_NO_TYPE_DESCRIPTOR, mr_load_data->ptrs.ra.data[idx].fd.type);
      return (0);
    }

  /* for C init style we can get union descriptor only from type cast */
  if ((0 == strcmp (tdp->name.str, "mr_ptr_t")) && (first_child >= 0) &&
      mr_load_data->ptrs.ra.data[first_child].fd.type && (NULL == mr_load_data->ptrs.ra.data[first_child].fd.name.str))
    {
      mr_load_data->ptrs.ra.data[first_child].fd.name.str = mr_load_data->ptrs.ra.data[first_child].fd.type;
      mr_load_data->ptrs.ra.data[first_child].fd.type = NULL;
    }

  /* loop on all subnodes */
  for (idx = first_child; idx >= 0; idx = mr_load_data->ptrs.ra.data[idx].next)
    {
      if (mr_load_data->ptrs.ra.data[idx].fd.name.str)
	fdp = mr_get_fd_by_name (tdp, mr_load_data->ptrs.ra.data[idx].fd.name.str);
      else
	fdp = mr_load_struct_next_field (tdp, fdp);
      
      if (NULL == fdp)
	{
	  MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_UNKNOWN_SUBNODE, tdp->name.str, mr_load_data->ptrs.ra.data[idx].fd.name.str);
	  return (0);
	}
      
      /* recursively load subnode */
      if (!mr_load (&data[fdp->offset], fdp, idx, mr_load_data))
	return (0);
    }
  return (!0);
}

/**
 * MR_STRUCT load handler. Wrapper over mr_load_struct_inner.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_struct (int idx, mr_load_data_t * mr_load_data)
{
  return (mr_load_struct_inner (idx, mr_load_data, mr_get_td_by_name (mr_load_data->ptrs.ra.data[idx].fd.type)));
}

/**
 * MR_ARRAY load handler.
 * Save content of subnodes to array elements.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_array (int idx, mr_load_data_t * mr_load_data)
{
  char * data = mr_load_data->ptrs.ra.data[idx].data;
  mr_fd_t fd_ = mr_load_data->ptrs.ra.data[idx].fd;
  int row_count = fd_.param.array_param.row_count;
  int count = fd_.param.array_param.count;
  int i = 0;
  
  if (1 == fd_.param.array_param.row_count)
    fd_.mr_type_ext = MR_TYPE_EXT_NONE; /* prepare copy of filed descriptor for array elements loading */
  else
    {
      fd_.param.array_param.count = row_count;
      fd_.param.array_param.row_count = 1;
    }
  
  /* loop on subnodes */
  for (idx = mr_load_data->ptrs.ra.data[idx].first_child; idx >= 0; idx = mr_load_data->ptrs.ra.data[idx].next)
    {
      /* check if array index is in range */
      if ((i < 0) || (i >= count))
	{
	  MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_RANGE_CHECK, fd_.name.str);
	  return (0);
	}
      /* load recursively */
      if (!mr_load (&data[i * fd_.size], &fd_, idx, mr_load_data))
	return (0);
      i += row_count;
    }
  return (!0);
}

static int
mr_load_rarray_data (int idx, mr_load_data_t * mr_load_data)
{
  char * ra_data = mr_load_data->ptrs.ra.data[idx].data;
  mr_rarray_t * ra = (void*)&ra_data[-offsetof (mr_rarray_t, data)];
  int i, count = 0;
  mr_fd_t fd_ = mr_load_data->ptrs.ra.data[idx].fd;

  for (i = mr_load_data->ptrs.ra.data[idx].first_child; i >= 0; i = mr_load_data->ptrs.ra.data[i].next)
    ++count;
  fd_.mr_type_ext = MR_TYPE_EXT_NONE;
  fd_.name = mr_load_data->ptrs.ra.data[mr_load_data->ptrs.ra.data[idx].parent].fd.name;

  ra->size = ra->alloc_size = count * fd_.size;
  ra->data = NULL;
  if ((mr_load_data->ptrs.ra.data[idx].ref_idx < 0) && (count > 0))
    {
      ra->data = MR_MALLOC (ra->size);
      if (NULL == ra->data)
	{
	  ra->alloc_size = ra->size = 0;
	  MR_MESSAGE (MR_LL_FATAL, MR_MESSAGE_OUT_OF_MEMORY);
	  return (0);
	}
      memset (ra->data, 0, ra->size);

      /* loop on subnodes */
      i = 0;
      for (idx = mr_load_data->ptrs.ra.data[idx].first_child; idx >= 0; idx = mr_load_data->ptrs.ra.data[idx].next)
	if (!mr_load (&((char*)ra->data)[fd_.size * i++], &fd_, idx, mr_load_data))
	  return (0);
    }
  return (!0);
}

int
mr_load_rarray_type (mr_fd_t * fdp, int (*action) (mr_td_t *, void *), void * context)
{
  mr_td_t * tdp = mr_get_td_by_name ("mr_rarray_t");
  int status = 0;
  if (NULL == tdp)
    MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_NO_TYPE_DESCRIPTOR, "mr_rarray_t");
  else
    {
      mr_td_t td = *tdp;
      int fields_count = td.fields.size / sizeof (td.fields.data[0]);
      mr_fd_ptr_t fields_data[fields_count];
      mr_fd_t * data_fdp;
      mr_fd_t fd;
      int i;
      
      memcpy (fields_data, td.fields.data, td.fields.size);
      td.fields.data = fields_data;
      for (i = 0; i < fields_count; ++i)
	{
	  data_fdp = fields_data[i].fdp;
	  if (0 == strcmp ("data", data_fdp->name.str))
	    break;
	}
      if (i >= fields_count)
	MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_RARRAY_FAILED);
      else
	{
	  fd = *fdp; /* make a copy of 'data' field descriptor */
	  fd.mr_type_ext = MR_TYPE_EXT_RARRAY_DATA;
	  fd.name = data_fdp->name;
	  fd.offset = data_fdp->offset;
	  fields_data[i].fdp = &fd; /* replace 'data' descriptor on a local copy */
	  mr_ic_none_new (&td.lookup_by_name, mr_hashed_string_cmp, "mr_fd_t");
	  mr_ic_index (&td.lookup_by_name, (mr_ic_rarray_t*)&td.fields, NULL);
	  status = action (&td, context);
	  mr_ic_free (&td.lookup_by_name, NULL);
	}
    }
  return (status);
}

TYPEDEF_STRUCT (mr_load_rarray_struct_t,
		int idx,
		(mr_load_data_t *, mr_load_data)
		)

static int
mr_load_rarray_inner (mr_td_t * tdp, void * context)
{
  mr_load_rarray_struct_t * mr_load_rarray_struct = context;
  return (mr_load_struct_inner (mr_load_rarray_struct->idx, mr_load_rarray_struct->mr_load_data, tdp));
}

/**
 * MR_RARRAY load handler.
 * Save content of subnodes to resizeable array elements
 * (allocate/reallocate required memory).
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_rarray (int idx, mr_load_data_t * mr_load_data)
{
  mr_rarray_t * ra = mr_load_data->ptrs.ra.data[idx].data;
  mr_load_rarray_struct_t mr_load_rarray_struct = {
    .idx = idx,
    .mr_load_data = mr_load_data,
  };

  memset (ra, 0, sizeof (*ra));
  
  if (!mr_load_rarray_type (&mr_load_data->ptrs.ra.data[idx].fd, mr_load_rarray_inner, &mr_load_rarray_struct))
    return (0);
  ra->alloc_size = ra->size;

  return (!0);
}

/**
 * MR_TYPE_EXT_POINTER load handler. Initiated as postponed call thru mr_load_pointer via stack.
 * Loads element into newly allocate memory.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_pointer_postponed (int idx, mr_load_data_t * mr_load_data)
{
  void ** data = mr_load_data->ptrs.ra.data[idx].data;
  mr_fd_t fd_ = mr_load_data->ptrs.ra.data[idx].fd;
  fd_.mr_type_ext = MR_TYPE_EXT_NONE;
  /* allocate memory */
  *data = MR_MALLOC (fd_.size);
  if (NULL == *data)
    {
      MR_MESSAGE (MR_LL_FATAL, MR_MESSAGE_OUT_OF_MEMORY);
      return (0);
    }
  memset (*data, 0, fd_.size);
  /* load recursively */
  return (mr_load (*data, &fd_, mr_load_data->ptrs.ra.data[idx].first_child, mr_load_data));
}

/**
 * MR_POINTER_STRUCT load handler. Schedule element postponed loading via stack.
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_pointer (int idx, mr_load_data_t * mr_load_data)
{
  void ** data = mr_load_data->ptrs.ra.data[idx].data;
  /* default initializer */
  *data = NULL;
  if (mr_load_data->ptrs.ra.data[idx].ref_idx >= 0)
    return (!0);
  if (mr_load_data->ptrs.ra.data[idx].first_child < 0)
    return (!0);
  /* check whether pointer should have offsprings or not */
  if ((MR_TYPE_NONE != mr_load_data->ptrs.ra.data[idx].fd.mr_type) && (MR_TYPE_VOID != mr_load_data->ptrs.ra.data[idx].fd.mr_type))
    {
      int * idx_ = mr_rarray_append ((void*)&mr_load_data->mr_ra_idx, sizeof (mr_load_data->mr_ra_idx.data[0]));
      if (NULL == idx_)
	return (0);
      *idx_ = idx;
      return (!0);
    }

  return (!0);
}

/**
 * MR_ANON_UNION load handler.
 * Load anonymous union
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
static int
mr_load_anon_union (int idx, mr_load_data_t * mr_load_data)
{
  /*
    Anonimous unions in C init style saved as named field folowed by union itself. Named field has type of zero length static string and must be inited by empty string. Here is an example.
    .anon_union_79 = "", {
    .union_float = 3.1415927410,
    },
  */
  int next = mr_load_data->ptrs.ra.data[idx].next;
  if ((mr_load_data->ptrs.ra.data[idx].first_child < 0) && /* if node has no childs, then it is C init style anonumous union */
      mr_load_data->ptrs.ra.data[idx].value && (0 == mr_load_data->ptrs.ra.data[idx].value[0]) && /* content must be an empty string */
      (next >= 0) && (NULL == mr_load_data->ptrs.ra.data[next].fd.name.str)) /* there should be a next node without name */
    {
      if (mr_load_data->ptrs.ra.data[idx].fd.name.str) /* sainity check - this field can't be NULL */
	mr_load_data->ptrs.ra.data[next].fd.name.str = MR_STRDUP (mr_load_data->ptrs.ra.data[idx].fd.name.str);
      return (!0); /* now next node has a name and will be loaded by top level procedure */
    }
  return (mr_load_struct (idx, mr_load_data));
}

/**
 * Cleanup helper. Deallocates all dynamically allocated resources.
 * @param ptrs resizeable array with pointers descriptors
 * @return Status of read (0 - failure, !0 - success)
 */
int
mr_free_ptrs (mr_ra_mr_ptrdes_t ptrs)
{
  if (ptrs.ra.data)
    {
      int count = ptrs.ra.size / sizeof (ptrs.ra.data[0]);
      int i;
      for (i = 0; i < count; ++i)
	{
	  if (ptrs.ra.data[i].value)
	    MR_FREE (ptrs.ra.data[i].value);
	  ptrs.ra.data[i].value = NULL;
	  if (ptrs.ra.data[i].fd.type)
	    MR_FREE (ptrs.ra.data[i].fd.type);
	  ptrs.ra.data[i].fd.type = NULL;
	  if (ptrs.ra.data[i].fd.name.str)
	    MR_FREE (ptrs.ra.data[i].fd.name.str);
	  ptrs.ra.data[i].fd.name.str = NULL;
	}
      MR_FREE (ptrs.ra.data);
      ptrs.ra.data = NULL;
      ptrs.ra.size = ptrs.ra.alloc_size = 0;
    }
  return (EXIT_SUCCESS);
}

/**
 * Init IO handlers Table
 */
static mr_load_handler_t mr_load_handler[] =
  {
    [MR_TYPE_NONE] = mr_load_none,
    [MR_TYPE_VOID] = mr_load_none,
    [MR_TYPE_ENUM] = mr_load_enum,
    [MR_TYPE_BITFIELD] = mr_load_bitfield,
    [MR_TYPE_BITMASK] = mr_load_bitmask,
    [MR_TYPE_INT8] = mr_load_integer,
    [MR_TYPE_UINT8] = mr_load_integer,
    [MR_TYPE_INT16] = mr_load_integer,
    [MR_TYPE_UINT16] = mr_load_integer,
    [MR_TYPE_INT32] = mr_load_integer,
    [MR_TYPE_UINT32] = mr_load_integer,
    [MR_TYPE_INT64] = mr_load_integer,
    [MR_TYPE_UINT64] = mr_load_integer,
    [MR_TYPE_FLOAT] = mr_load_float,
    [MR_TYPE_DOUBLE] = mr_load_double,
    [MR_TYPE_LONG_DOUBLE] = mr_load_long_double_t,
    [MR_TYPE_CHAR] = mr_load_char,
    [MR_TYPE_CHAR_ARRAY] = mr_load_char_array,
    [MR_TYPE_STRING] = mr_load_string,
    [MR_TYPE_STRUCT] = mr_load_struct,
    [MR_TYPE_FUNC] = mr_load_func,
    [MR_TYPE_FUNC_TYPE] = mr_load_func,
    [MR_TYPE_UNION] = mr_load_struct,
    [MR_TYPE_ANON_UNION] = mr_load_struct,
    [MR_TYPE_NAMED_ANON_UNION] = mr_load_anon_union,
  };

static mr_load_handler_t mr_ext_load_handler[] =
  {
    [MR_TYPE_EXT_ARRAY] = mr_load_array,
    [MR_TYPE_EXT_RARRAY] = mr_load_rarray,
    [MR_TYPE_EXT_RARRAY_DATA] = mr_load_rarray_data,
    [MR_TYPE_EXT_POINTER] = mr_load_pointer,
  };

/**
 * Public function. Load router. Load any object from internal representation graph.
 * @param data pointer on place to save data
 * @param fdp filed descriptor
 * @param idx node index
 * @param mr_load_data structures that holds context of loading
 * @return Status of read (0 - failure, !0 - success)
 */
int
mr_load (void * data, mr_fd_t * fdp, int idx, mr_load_data_t * mr_load_data)
{
  int status = 0;

  if (0 == idx)
    {
      mr_load_data->mr_ra_idx.data = NULL;
      mr_load_data->mr_ra_idx.size = 0;
      mr_load_data->mr_ra_idx.alloc_size = 0;
    }
  
  if ((idx < 0) || (idx >= mr_load_data->ptrs.ra.size / sizeof (mr_load_data->ptrs.ra.data[0])))
    {
      MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_SAVE_IDX_RANGE_CHECK);
      return (0);
    }
  
  mr_load_data->ptrs.ra.data[idx].data = data;
  if (mr_load_data->ptrs.ra.data[idx].fd.name.str && fdp->name.str)
    if (strcmp (fdp->name.str, mr_load_data->ptrs.ra.data[idx].fd.name.str))
      {
	MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_NODE_NAME_MISSMATCH, fdp->name.str, mr_load_data->ptrs.ra.data[idx].fd.name.str);
	return (0);
      }

  if (mr_load_data->ptrs.ra.data[idx].fd.type && fdp->type)
    if (strcmp (fdp->type, mr_load_data->ptrs.ra.data[idx].fd.type))
      {
	MR_MESSAGE (MR_LL_ERROR, MR_MESSAGE_NODE_TYPE_MISSMATCH, fdp->type, mr_load_data->ptrs.ra.data[idx].fd.type);
	return (0);
      }
  
  if ((NULL == mr_load_data->ptrs.ra.data[idx].fd.name.str) && (fdp->name.str))
    mr_load_data->ptrs.ra.data[idx].fd.name.str = MR_STRDUP (fdp->name.str);
  if ((NULL == mr_load_data->ptrs.ra.data[idx].fd.type) && (fdp->type))
    mr_load_data->ptrs.ra.data[idx].fd.type = MR_STRDUP (fdp->type);
  mr_load_data->ptrs.ra.data[idx].fd.size = fdp->size;
  mr_load_data->ptrs.ra.data[idx].fd.mr_type = fdp->mr_type;
  mr_load_data->ptrs.ra.data[idx].fd.mr_type_aux = fdp->mr_type_aux;
  mr_load_data->ptrs.ra.data[idx].fd.mr_type_ext = fdp->mr_type_ext;
  mr_load_data->ptrs.ra.data[idx].fd.param = fdp->param;
  
  /* route loading */
  if ((fdp->mr_type_ext >= 0) && (fdp->mr_type_ext < MR_TYPE_EXT_LAST)
      && mr_ext_load_handler[fdp->mr_type_ext])
    status = mr_ext_load_handler[fdp->mr_type_ext] (idx, mr_load_data);
  else if ((fdp->mr_type >= 0) && (fdp->mr_type < MR_TYPE_LAST)
	   && mr_load_handler[fdp->mr_type])
    status = mr_load_handler[fdp->mr_type] (idx, mr_load_data);
  else
    MR_MESSAGE_UNSUPPORTED_NODE_TYPE_ (fdp);    

  /* set cross references at the upper level */
  if (0 == idx)
    {
      while (mr_load_data->mr_ra_idx.size > 0)
	{
	  mr_load_data->mr_ra_idx.size -= sizeof (mr_load_data->mr_ra_idx.data[0]);
	  mr_load_pointer_postponed (mr_load_data->mr_ra_idx.data[mr_load_data->mr_ra_idx.size / sizeof (mr_load_data->mr_ra_idx.data[0])], mr_load_data);
	}
      if (status)
	status = mr_set_crossrefs (mr_load_data);
      if (mr_load_data->mr_ra_idx.data)
	{
	  MR_FREE (mr_load_data->mr_ra_idx.data);
	  mr_load_data->mr_ra_idx.data = NULL;
	  mr_load_data->mr_ra_idx.size = 0;
	  mr_load_data->mr_ra_idx.alloc_size = 0;
	}
    }
  
  return (status);
}     
