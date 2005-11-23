#include "Eet.h"
#include "Eet_private.h"

/*
 * rotuines for doing data -> struct and struct -> data conversion
 *
 * types:
 *
 * basic types:
 *   a sequence of...
 *
 *   char
 *   short
 *   int
 *   long long
 *   float
 *   double
 *   unsigned char
 *   unsigned short
 *   unsigned int
 *   unsgined long long
 *   string
 *
 * groupings:
 *   multiple entries ordered as...
 *
 *   fixed size array    [ of basic types ]
 *   variable size array [ of basic types ]
 *   linked list         [ of basic types ]
 *   hash table          [ of basic types ]
 *
 * need to provide builder/accessor funcs for:
 *
 *   list_next
 *   list_append
 *
 *   hash_foreach
 *   hash_add
 *
 */

/*---*/

typedef struct _Eet_Data_Element            Eet_Data_Element;
typedef struct _Eet_Data_Basic_Type_Decoder Eet_Data_Basic_Type_Decoder;
typedef struct _Eet_Data_Chunk              Eet_Data_Chunk;
typedef struct _Eet_Data_Stream             Eet_Data_Stream;
typedef struct _Eet_Data_Descriptor_Hash    Eet_Data_Descriptor_Hash;

/*---*/

struct _Eet_Data_Basic_Type_Decoder
{
   int     size;
   int   (*get) (void *src, void *src_end, void *dest);
   void *(*put) (void *src, int *size_ret);
};

struct _Eet_Data_Chunk
{
   char *name;
   int   size;
   void *data;
};

struct _Eet_Data_Stream
{
   void *data;
   int   size;
   int   pos;
};

struct _Eet_Data_Descriptor_Hash
{
   Eet_Data_Element         *element;
   Eet_Data_Descriptor_Hash *next;
};

struct _Eet_Data_Descriptor
{
   char *name;
   int   size;
   struct {
      void *(*list_next) (void *l);
      void *(*list_append) (void *l, void *d);
      void *(*list_data) (void *l);
      void *(*list_free) (void *l);
      void  (*hash_foreach) (void *h, int (*func) (void *h, const char *k, void *dt, void *fdt), void *fdt);
      void *(*hash_add) (void *h, const char *k, void *d);
      void  (*hash_free) (void *h);
   } func;
   struct {
      int               num;
      Eet_Data_Element *set;
      struct {
	 int                       size;
	 Eet_Data_Descriptor_Hash *buckets;
      } hash;
   } elements;
};

struct _Eet_Data_Element
{
   char                *name;
   int                  type;
   int                  group_type;
   int                  offset;
   int                  count;
   char                *counter_name;
   Eet_Data_Descriptor *subtype;
};

/*---*/

static int   eet_data_get_char(void *src, void *src_end, void *dest);
static void *eet_data_put_char(void *src, int *size_ret);
static int   eet_data_get_short(void *src, void *src_end, void *dest);
static void *eet_data_put_short(void *src, int *size_ret);
static int   eet_data_get_int(void *src, void *src_end, void *dest);
static void *eet_data_put_int(void *src, int *size_ret);
static int   eet_data_get_long_long(void *src, void *src_end, void *dest);
static void *eet_data_put_long_long(void *src, int *size_ret);
static int   eet_data_get_float(void *src, void *src_end, void *dest);
static void *eet_data_put_float(void *src, int *size_ret);
static int   eet_data_get_double(void *src, void *src_end, void *dest);
static void *eet_data_put_double(void *src, int *size_ret);
static int   eet_data_get_string(void *src, void *src_end, void *dest);
static void *eet_data_put_string(void *src, int *size_ret);

static int   eet_data_get_type(int type, void *src, void *src_end, void *dest);
static void *eet_data_put_type(int type, void *src, int *size_ret);

static Eet_Data_Chunk *eet_data_chunk_get(void *src, int size);
static Eet_Data_Chunk *eet_data_chunk_new(void *data, int size, char *name);
static void            eet_data_chunk_free(Eet_Data_Chunk *chnk);

static Eet_Data_Stream *eet_data_stream_new(void);
static void             eet_data_stream_write(Eet_Data_Stream *ds, void *data, int size);
static void             eet_data_stream_free(Eet_Data_Stream *ds);

static void             eet_data_chunk_put(Eet_Data_Chunk *chnk, Eet_Data_Stream *ds);

/*---*/

const Eet_Data_Basic_Type_Decoder eet_coder[] =
{
     {sizeof(char),      eet_data_get_char,      eet_data_put_char     },
     {sizeof(short),     eet_data_get_short,     eet_data_put_short    },
     {sizeof(int),       eet_data_get_int,       eet_data_put_int      },
     {sizeof(long long), eet_data_get_long_long, eet_data_put_long_long},
     {sizeof(float),     eet_data_get_float,     eet_data_put_float    },
     {sizeof(double),    eet_data_get_double,    eet_data_put_double   },
     {sizeof(char),      eet_data_get_char,      eet_data_put_char     },
     {sizeof(short),     eet_data_get_short,     eet_data_put_short    },
     {sizeof(int),       eet_data_get_int,       eet_data_put_int      },
     {sizeof(long long), eet_data_get_long_long, eet_data_put_long_long},
     {sizeof(char *),    eet_data_get_string,    eet_data_put_string   }
};

static int words_bigendian = -1;

/*---*/

#define SWAP64(x) (x) = \
   ((((unsigned long long)(x) & 0x00000000000000ffULL ) << 56) |\
       (((unsigned long long)(x) & 0x000000000000ff00ULL ) << 40) |\
       (((unsigned long long)(x) & 0x0000000000ff0000ULL ) << 24) |\
       (((unsigned long long)(x) & 0x00000000ff000000ULL ) << 8) |\
       (((unsigned long long)(x) & 0x000000ff00000000ULL ) >> 8) |\
       (((unsigned long long)(x) & 0x0000ff0000000000ULL ) >> 24) |\
       (((unsigned long long)(x) & 0x00ff000000000000ULL ) >> 40) |\
       (((unsigned long long)(x) & 0xff00000000000000ULL ) >> 56))
#define SWAP32(x) (x) = \
   ((((int)(x) & 0x000000ff ) << 24) |\
       (((int)(x) & 0x0000ff00 ) << 8) |\
       (((int)(x) & 0x00ff0000 ) >> 8) |\
       (((int)(x) & 0xff000000 ) >> 24))
#define SWAP16(x) (x) = \
   ((((short)(x) & 0x00ff ) << 8) |\
       (((short)(x) & 0xff00 ) >> 8))

#define CONV8(x)
#define CONV16(x) {if (words_bigendian) SWAP16(x);}
#define CONV32(x) {if (words_bigendian) SWAP32(x);}
#define CONV64(x) {if (words_bigendian) SWAP64(x);}

/*---*/

/* CHAR TYPE */
static int
eet_data_get_char(void *src, void *src_end, void *dst)
{
   char *s, *d;

   if (((char *)src + sizeof(char)) > (char *)src_end) return -1;
   s = (char *)src;
   d = (char *)dst;
   *d = *s;
   CONV8(*d);
   return sizeof(char);
}

static void *
eet_data_put_char(void *src, int *size_ret)
{
   char *s, *d;

   d = (char *)malloc(sizeof(char));
   if (!d) return NULL;
   s = (char *)src;
   *d = *s;
   CONV8(*d);
   *size_ret = sizeof(char);
   return d;
}

/* SHORT TYPE */
static int
eet_data_get_short(void *src, void *src_end, void *dst)
{
   short *d;

   if (((char *)src + sizeof(short)) > (char *)src_end) return -1;
   memcpy(dst, src, sizeof(short));
   d = (short *)dst;
   CONV16(*d);
   return sizeof(short);
}

static void *
eet_data_put_short(void *src, int *size_ret)
{
   short *s, *d;

   d = (short *)malloc(sizeof(short));
   if (!d) return NULL;
   s = (short *)src;
   *d = *s;
   CONV16(*d);
   *size_ret = sizeof(short);
   return d;
}

/* INT TYPE */
static int
eet_data_get_int(void *src, void *src_end, void *dst)
{
   int *d;

   if (((char *)src + sizeof(int)) > (char *)src_end) return -1;
   memcpy(dst, src, sizeof(int));
   d = (int *)dst;
   CONV32(*d);
   return sizeof(int);
}

static void *
eet_data_put_int(void *src, int *size_ret)
{
   int *s, *d;

   d = (int *)malloc(sizeof(int));
   if (!d) return NULL;
   s = (int *)src;
   *d = *s;
   CONV32(*d);
   *size_ret = sizeof(int);
   return d;
}

/* LONG LONG TYPE */
static int
eet_data_get_long_long(void *src, void *src_end, void *dst)
{
   unsigned long long *d;

   if (((char *)src + sizeof(unsigned long long)) > (char *)src_end) return -1;
   memcpy(dst, src, sizeof(unsigned long long));
   d = (unsigned long long *)dst;
   CONV64(*d);
   return sizeof(unsigned long long);
}

static void *
eet_data_put_long_long(void *src, int *size_ret)
{
   unsigned long long *s, *d;

   d = (unsigned long long *)malloc(sizeof(unsigned long long));
   if (!d) return NULL;
   s = (unsigned long long *)src;
   *d = *s;
   CONV64(*d);
   *size_ret = sizeof(unsigned long long);
   return d;
}

/* STRING TYPE */
static int
eet_data_get_string(void *src, void *src_end, void *dst)
{
   char *s, **d, *p;
   int len;

   s = (char *)src;
   d = (char **)dst;
   p = s;
   len = 0;
   while ((p < (char *)src_end) && (*p != 0)) {len++; p++;}
   if (len == 0)
     {
	*d = NULL;
	return 0;
     }
   *d = malloc(len + 1);
   if (!(*d)) return -1;
   memcpy(*d, s, len);
   (*d)[len] = 0;
   return len + 1;
}

static void *
eet_data_put_string(void *src, int *size_ret)
{
   char *s, *d;
   int len;
   const char *empty_s = "";

   s = (char *)(*((char **)src));
   if (!s) return NULL;
//   if (!s) s = (char *)empty_s;
   len = strlen(s);
   d = malloc(len + 1);
   if (!d) return NULL;
   strcpy(d, s);
   *size_ret = len + 1;
   return d;
}

/* FLOAT TYPE */
static int
eet_data_get_float(void *src, void *src_end, void *dst)
{
   float *d;
   float tf;
   char *s, *str, *p, *prev_locale;
   int len;

   s = (char *)src;
   d = (float *)dst;
   p = s;
   len = 0;
   while ((p < (char *)src_end) && (*p != 0)) {len++; p++;}
   str = malloc(len + 1);
   if (!str) return -1;
   memcpy(str, s, len);
   str[len] = 0;

   prev_locale = setlocale(LC_NUMERIC, "C");
/* solaris atof is broken and doesnt understand %a format as a float */   
/*   *d = (float)atof(str); */
   sscanf(str, "%a", &tf);
   *d = (float)tf;
   if (prev_locale) setlocale(LC_NUMERIC, prev_locale);

   free(str);
   return len + 1;
}

static void *
eet_data_put_float(void *src, int *size_ret)
{
   float *s;
   char *d, buf[64], *prev_locale;
   int len;

   s = (float *)src;
   prev_locale = setlocale(LC_NUMERIC, "C");
   snprintf(buf, sizeof(buf), "%a", (double)(*s));
   if (prev_locale) setlocale(LC_NUMERIC, prev_locale);
   len = strlen(buf);
   d = malloc(len + 1);
   if (!d) return NULL;
   strcpy(d, buf);
   *size_ret = len + 1;
   return d;
}

/* DOUBLE TYPE */
static int
eet_data_get_double(void *src, void *src_end, void *dst)
{
   double *d;
   float tf;
   char *s, *str, *p, *prev_locale;
   int len;

   s = (char *)src;
   d = (double *)dst;
   p = s;
   len = 0;
   while ((p < (char *)src_end) && (*p != 0)) {len++; p++;}
   str = malloc(len + 1);
   if (!str) return -1;
   memcpy(str, s, len);
   str[len] = 0;

   prev_locale = setlocale(LC_NUMERIC, "C");
/* solaris atof is broken and doesnt understand %a format as a float */   
/*   *d = (double)atof(str); */
   sscanf(str, "%a", &tf);
   *d = (double)tf;
   if (prev_locale) setlocale(LC_NUMERIC, prev_locale);

   free(str);
   return len + 1;
}

static void *
eet_data_put_double(void *src, int *size_ret)
{
   double *s;
   char *d, buf[128], *prev_locale;
   int len;

   s = (double *)src;
   prev_locale = setlocale(LC_NUMERIC, "C");
   snprintf(buf, sizeof(buf), "%a", (double)(*s));
   if (prev_locale) setlocale(LC_NUMERIC, prev_locale);
   len = strlen(buf);
   d = malloc(len + 1);
   if (!d) return NULL;
   strcpy(d, buf);
   *size_ret = len + 1;
   return d;
}

static int
eet_data_get_type(int type, void *src, void *src_end, void *dest)
{
   int ret;

   ret = eet_coder[type - 1].get(src, src_end, dest);
   return ret;
}

static void *
eet_data_put_type(int type, void *src, int *size_ret)
{
   void *ret;

   ret = eet_coder[type - 1].put(src, size_ret);
   return ret;
}

/* chunk format...
 *
 * char[4] = "CHnK";
 * int     = chunk size (including magic string);
 * char[]  = chuck magic/name string (0 byte terminated);
 * ... sub-chunks (a chunk can contain chuncks recusrively) ...
 * or
 * ... payload data ...
 *
 */

static Eet_Data_Chunk *
eet_data_chunk_get(void *src, int size)
{
   Eet_Data_Chunk *chnk;
   char *s;
   int ret1, ret2;

   if (!src) return NULL;
   if (size <= 8) return NULL;

   chnk = calloc(1, sizeof(Eet_Data_Chunk));
   if (!chnk) return NULL;

   s = src;
   if ((s[0] != 'C') || (s[1] != 'H') || (s[2] != 'n') || (s[3] != 'K'))
     {
	free(chnk);
	return NULL;
     }
   ret1 = eet_data_get_type(EET_T_INT, (void *)(s + 4), (void *)(s + size), &(chnk->size));
   if (ret1 <= 0)
     {
	free(chnk);
	return NULL;
     }
   if ((chnk->size < 0) || ((chnk->size + 8) > size))
     {
	free(chnk);
	return NULL;
     }
   ret2 = eet_data_get_type(EET_T_STRING, (void *)(s + 8), (void *)(s + size), &(chnk->name));
   if (ret2 <= 0)
     {
	free(chnk);
	return NULL;
     }
   chnk->data = (char *)src + 4 + ret1 + ret2;
   chnk->size -= ret2;
   return chnk;
}

static Eet_Data_Chunk *
eet_data_chunk_new(void *data, int size, char *name)
{
   Eet_Data_Chunk *chnk;

   if (!name) return NULL;
   chnk = calloc(1, sizeof(Eet_Data_Chunk));
   if (!chnk) return NULL;

   chnk->name = strdup(name);
   chnk->size = size;
   chnk->data = data;

   return chnk;
}

static void
eet_data_chunk_free(Eet_Data_Chunk *chnk)
{
   if (chnk->name) free(chnk->name);
   free(chnk);
}

static Eet_Data_Stream *
eet_data_stream_new(void)
{
   Eet_Data_Stream *ds;

   ds = calloc(1, sizeof(Eet_Data_Stream));
   if (!ds) return NULL;
   return ds;
}

static void
eet_data_stream_free(Eet_Data_Stream *ds)
{
   if (ds->data) free(ds->data);
   free(ds);
}

static void
eet_data_stream_write(Eet_Data_Stream *ds, void *data, int size)
{
   char *p;

   if ((ds->pos + size) > ds->size)
     {
	ds->data = realloc(ds->data, ds->size + size + 256);
	if (!ds->data)
	  {
	     ds->pos = 0;
	     ds->size = 0;
	     return;
	  }
	ds->size = ds->size + size + 256;
     }
   p = ds->data;
   memcpy(p + ds->pos, data, size);
   ds->pos += size;
}

static void
eet_data_chunk_put(Eet_Data_Chunk *chnk, Eet_Data_Stream *ds)
{
   int *size;
   int s;
   int size_ret;

   if (!chnk->data) return;
   /* chunk head */
   eet_data_stream_write(ds, "CHnK", 4);
   /* size of chunk payload data + name */
   s = strlen(chnk->name) + 1 + chnk->size;
   size = eet_data_put_int(&s, &size_ret);
   if (size)
     {
	eet_data_stream_write(ds, size, size_ret);
	free(size);
     }
   /* write chunk name */
   eet_data_stream_write(ds, chnk->name, strlen(chnk->name) + 1);
   /* write payload */
   eet_data_stream_write(ds, chnk->data, chnk->size);
}

/*---*/

static int
_eet_descriptor_hash_gen(char *key, int hash_size)
{
   int hash_num = 0, i;
   unsigned char *ptr;
   const int masks[9] =
     {
	0x00,
	  0x01,
	  0x03,
	  0x07,
	  0x0f,
	  0x1f,
	  0x3f,
	  0x7f,
	  0xff
     };
   
   /* no string - index 0 */
   if (!key) return 0;

   /* calc hash num */
   for (i = 0, ptr = (unsigned char *)key; *ptr; ptr++, i++)
     hash_num ^= ((int)(*ptr) | ((int)(*ptr) << 8)) >> (i % 8);

   /* mask it */
   hash_num &= masks[hash_size];
   /* return it */
   return hash_num;
}

static void
_eet_descriptor_hash_new(Eet_Data_Descriptor *edd)
{
   int i;
   
   edd->elements.hash.size = 1 << 6;
   edd->elements.hash.buckets = calloc(1, sizeof(Eet_Data_Descriptor_Hash) * edd->elements.hash.size);
   for (i = 0; i < edd->elements.num; i++)
     {
	Eet_Data_Element *ede;
	int hash;
	
	ede = &(edd->elements.set[i]);
	hash = _eet_descriptor_hash_gen(ede->name, 6);
	if (!edd->elements.hash.buckets[hash].element)
	  edd->elements.hash.buckets[hash].element = ede;
	else
	  {
	     Eet_Data_Descriptor_Hash *bucket;
	     
	     bucket = calloc(1, sizeof(Eet_Data_Descriptor_Hash));
	     bucket->element = ede;
	     bucket->next = edd->elements.hash.buckets[hash].next;
	     edd->elements.hash.buckets[hash].next = bucket;
	  }
     }
}

static void
_eet_descriptor_hash_free(Eet_Data_Descriptor *edd)
{
   int i;
   
   for (i = 0; i < edd->elements.hash.size; i++)
     {
	Eet_Data_Descriptor_Hash *bucket, *pbucket;
	
	bucket = edd->elements.hash.buckets[i].next;
	while (bucket)
	  {
	     pbucket = bucket;
	     bucket = bucket->next;
	     free(pbucket);
	  }
     }
   if (edd->elements.hash.buckets) free(edd->elements.hash.buckets);
}

static Eet_Data_Element *
_eet_descriptor_hash_find(Eet_Data_Descriptor *edd, char *name)
{
   int hash;
   Eet_Data_Descriptor_Hash *bucket;
   
   hash = _eet_descriptor_hash_gen(name, 6);
   if (!edd->elements.hash.buckets[hash].element) return NULL;
   if (!strcmp(edd->elements.hash.buckets[hash].element->name, name))
     return edd->elements.hash.buckets[hash].element;
   bucket = edd->elements.hash.buckets[hash].next;
   while (bucket)
     {
	if (!strcmp(bucket->element->name, name)) return bucket->element;
	bucket = bucket->next;
     }
   return NULL;
}

/*---*/

Eet_Data_Descriptor *
eet_data_descriptor_new(char *name,
			int size,
			void *(*func_list_next) (void *l),
			void *(*func_list_append) (void *l, void *d),
			void *(*func_list_data) (void *l),
			void *(*func_list_free) (void *l),
			void  (*func_hash_foreach) (void *h, int (*func) (void *h, const char *k, void *dt, void *fdt), void *fdt),
			void *(*func_hash_add) (void *h, const char *k, void *d),
			void  (*func_hash_free) (void *h))
{
   Eet_Data_Descriptor *edd;

   if (!name) return NULL;
   edd = calloc(1, sizeof(Eet_Data_Descriptor) + strlen(name) + 1);
   edd->name = ((char *)edd) + sizeof(Eet_Data_Descriptor);
   strcpy(edd->name, name);
   edd->size = size;
   edd->func.list_next = func_list_next;
   edd->func.list_append = func_list_append;
   edd->func.list_data = func_list_data;
   edd->func.list_free = func_list_free;
   edd->func.hash_foreach = func_hash_foreach;
   edd->func.hash_add = func_hash_add;
   edd->func.hash_free = func_hash_free;
   return edd;
}

void
eet_data_descriptor_free(Eet_Data_Descriptor *edd)
{
   int i;

   _eet_descriptor_hash_free(edd);
   for (i = 0; i < edd->elements.num; i++)
     {
	if (edd->elements.set[i].name) free(edd->elements.set[i].name);
	if (edd->elements.set[i].counter_name) free(edd->elements.set[i].counter_name);
     }
   if (edd->elements.set) free(edd->elements.set);
   free(edd);
}

void
eet_data_descriptor_element_add(Eet_Data_Descriptor *edd, char *name, int type,
				int group_type,
				int offset,
				int count, char *counter_name,
				Eet_Data_Descriptor *subtype)
{
   Eet_Data_Element *ede;

   edd->elements.num++;
   edd->elements.set = realloc(edd->elements.set, edd->elements.num * sizeof(Eet_Data_Element));
   if (!edd->elements.set) return;
   ede = &(edd->elements.set[edd->elements.num - 1]);
   ede->name = strdup(name);
   ede->type = type;
   ede->group_type = group_type;
   ede->offset = offset;
   ede->count = count;
   if (counter_name)
     ede->counter_name = strdup(counter_name);
   else ede->counter_name = NULL;
   ede->subtype = subtype;
}

void *
eet_data_read(Eet_File *ef, Eet_Data_Descriptor *edd, char *name)
{
   void *data_dec;
   void *data;
   int size;

   data = eet_read(ef, name, &size);
   if (!data) return NULL;
   data_dec = eet_data_descriptor_decode(edd, data, size);
   free(data);
   return data_dec;
}

int
eet_data_write(Eet_File *ef, Eet_Data_Descriptor *edd, char *name, void *data, int compress)
{
   void *data_enc;
   int size;
   int val;

   data_enc = eet_data_descriptor_encode(edd, data, &size);
   if (!data_enc) return 0;
   val = eet_write(ef, name, data_enc, size, compress);
   free(data_enc);
   return val;
}

static int    freelist_ref = 0;
static int    freelist_len = 0;
static int    freelist_num = 0;
static void **freelist = NULL;

static void
_eet_freelist_add(void *data)
{
   freelist_num++;
   if (freelist_num > freelist_len)
     {
	freelist_len += 16;
	freelist = realloc(freelist, freelist_len * sizeof(void *));
     }
   freelist[freelist_num - 1] = data;
}

static void
_eet_freelist_reset(void)
{
   if (freelist_ref > 0) return;
   freelist_len = 0;
   freelist_num = 0;
   if (freelist) free(freelist);
   freelist = NULL;
}

static void
_eet_freelist_free(void)
{
   int i;

   if (freelist_ref > 0) return;
   for (i = 0; i < freelist_num; i++)
     free(freelist[i]);
   _eet_freelist_reset();
}

static void
_eet_freelist_ref(void)
{
   freelist_ref++;
}

static void
_eet_freelist_unref(void)
{
   freelist_ref--;
}

static int     freelist_list_ref = 0;
static int     freelist_list_len = 0;
static int     freelist_list_num = 0;
static void ***freelist_list = NULL;

static void
_eet_freelist_list_add(void **data)
{
   int i;

   for (i = 0; i < freelist_list_num; i++)
     {
	if (freelist_list[i] == data) return;
     }
   freelist_list_num++;
   if (freelist_list_num > freelist_list_len)
     {
	freelist_list_len += 16;
	freelist_list = realloc(freelist_list, freelist_list_len * sizeof(void *));
     }
   freelist_list[freelist_list_num - 1] = data;
}

static void
_eet_freelist_list_reset(void)
{
   if (freelist_list_ref > 0) return;
   freelist_list_len = 0;
   freelist_list_num = 0;
   if (freelist_list) free(freelist_list);
   freelist_list = NULL;
}

static void
_eet_freelist_list_free(Eet_Data_Descriptor *edd)
{
   int i;

   if (freelist_list_ref > 0) return;
   for (i = 0; i < freelist_list_num; i++)
     edd->func.list_free(*(freelist_list[i]));
   _eet_freelist_list_reset();
}

static void
_eet_freelist_list_ref(void)
{
   freelist_list_ref++;
}

static void
_eet_freelist_list_unref(void)
{
   freelist_list_ref--;
}

void *
eet_data_descriptor_decode(Eet_Data_Descriptor *edd,
			   void *data_in,
			   int size_in)
{
   void *data;
   char *p;
   int size;
   Eet_Data_Chunk *chnk;

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   data = calloc(1, edd->size);
   if (!data) return NULL;
   _eet_freelist_ref();
   _eet_freelist_list_ref();
   _eet_freelist_add(data);
   chnk = eet_data_chunk_get(data_in, size_in);
   if (!chnk)
     {
	_eet_freelist_unref();
	_eet_freelist_list_unref();
	_eet_freelist_free();
	_eet_freelist_list_free(edd);
	return NULL;
     }
   if (strcmp(chnk->name, edd->name))
     {
	eet_data_chunk_free(chnk);
	_eet_freelist_unref();
	_eet_freelist_list_unref();
	_eet_freelist_free();
	_eet_freelist_list_free(edd);
	return NULL;
     }
   p = chnk->data;
   size = size_in - (4 + 4 + strlen(chnk->name) + 1);
   if (!edd->elements.hash.buckets) _eet_descriptor_hash_new(edd);
   while (size > 0)
     {
	Eet_Data_Chunk *echnk;
	Eet_Data_Element *ede;

	/* get next data chunk */
	echnk = eet_data_chunk_get(p, size);
	if (!echnk)
	  {
	     _eet_freelist_unref();
	     _eet_freelist_list_unref();
	     _eet_freelist_free();
	     _eet_freelist_list_free(edd);
	     eet_data_chunk_free(chnk);
	     return NULL;
	  }
	/* FIXME: this is a linear search/match - speed up by putting in a
	 * hash lookup
	 */
	ede = _eet_descriptor_hash_find(edd, echnk->name);
	if (ede)
	  {
	     if (ede->group_type == EET_G_UNKNOWN)
	       {
		  int ret;
		  void *data_ret;
		  
		  if ((ede->type >= EET_T_CHAR) &&
		      (ede->type <= EET_T_STRING))
		    {
		       ret = eet_data_get_type(ede->type,
					       echnk->data,
					       ((char *)echnk->data) + echnk->size,
					       ((char *)data) + ede->offset);
		    }
		  else if (ede->subtype)
		    {
		       void **ptr;
		       
		       data_ret = eet_data_descriptor_decode(ede->subtype,
							     echnk->data,
							     echnk->size);
		       if (!data_ret)
			 {
			    _eet_freelist_unref();
			    _eet_freelist_list_unref();
			    _eet_freelist_free();
			    _eet_freelist_list_free(edd);
			    eet_data_chunk_free(chnk);
			    return NULL;
			 }
		       ptr = (void **)(((char *)data) + ede->offset);
		       *ptr = (void *)data_ret;
		    }
	       }
	     else
	       {
		  switch (ede->group_type)
		    {
		     case EET_G_ARRAY:
		     case EET_G_VAR_ARRAY:
			 {
			    printf("ARRAY TYPE NOT IMPLIMENTED YET!!!\n");
			 }
		       break;
		     case EET_G_LIST:
			 {
			    int ret;
			    void *list = NULL;
			    void **ptr;
			    void *data_ret;
			    
			    ptr = (void **)(((char *)data) + ede->offset);
			    list = *ptr;
			    data_ret = NULL;
			    if ((ede->type >= EET_T_CHAR) &&
				(ede->type <= EET_T_STRING))
			      {
				 data_ret = calloc(1, eet_coder[ede->type].size);
				 if (data_ret)
				   {
				      _eet_freelist_add(data_ret);
				      ret = eet_data_get_type(ede->type,
							      echnk->data,
							      ((char *)echnk->data) + echnk->size,
							      data_ret);
				      if (ret <= 0)
					{
					   _eet_freelist_unref();
					   _eet_freelist_list_unref();
					   _eet_freelist_free();
					   _eet_freelist_list_free(edd);
					   eet_data_chunk_free(chnk);
					   return NULL;
					}
				   }
				 else
				   {
				      _eet_freelist_unref();
				      _eet_freelist_list_unref();
				      _eet_freelist_free();
				      _eet_freelist_list_free(edd);
				      eet_data_chunk_free(chnk);
				      return NULL;
				   }
			      }
			    else if (ede->subtype)
			      {
				 data_ret = eet_data_descriptor_decode(ede->subtype,
								       echnk->data,
								       echnk->size);
			      }
			    if (data_ret)
			      {
				 list = edd->func.list_append(list, data_ret);
				 *ptr = list;
				 _eet_freelist_list_add(ptr);
			      }
			    else
			      {
				 _eet_freelist_unref();
				 _eet_freelist_list_unref();
				 _eet_freelist_free();
				 _eet_freelist_list_free(edd);
				 eet_data_chunk_free(chnk);
				 return NULL;
			      }
			 }
		       break;
		     case EET_G_HASH:
		       printf("HASH TYPE NOT IMPLIMENTED YET!!!\n");
		       break;
		     default:
		       break;
		    }
	       }
	  }
	/* advance to next chunk */
	p += (4 + 4 + strlen(echnk->name) + 1 + echnk->size);
	size -= (4 + 4 + strlen(echnk->name) + 1 + echnk->size);
	eet_data_chunk_free(echnk);
     }
   eet_data_chunk_free(chnk);
   _eet_freelist_unref();
   _eet_freelist_list_unref();
   _eet_freelist_reset();
   _eet_freelist_list_reset();
   return data;
}

void *
eet_data_descriptor_encode(Eet_Data_Descriptor *edd,
			   void *data_in,
			   int *size_ret)
{
   Eet_Data_Chunk *chnk;
   Eet_Data_Stream *ds;
   int i;
   void *cdata;
   int csize;

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   ds = eet_data_stream_new();
   for (i = 0; i < edd->elements.num; i++)
     {
	Eet_Data_Element *ede;
	Eet_Data_Chunk *echnk;
	void *data;
	int size;

	ede = &(edd->elements.set[i]);
	data = NULL;
	if (ede->group_type == EET_G_UNKNOWN)
	  {
	     if ((ede->type >= EET_T_CHAR) &&
		 (ede->type <= EET_T_STRING))
	       data = eet_data_put_type(ede->type,
					((char *)data_in) + ede->offset,
					&size);
	     else if (ede->subtype)
	       {
		  if (*((char **)(((char *)data_in) + ede->offset)))
		    data = eet_data_descriptor_encode(ede->subtype,
						      *((char **)(((char *)data_in) + ede->offset)),
						      &size);
	       }
	     if (data)
	       {
		  echnk = eet_data_chunk_new(data, size, ede->name);
		  eet_data_chunk_put(echnk, ds);
		  eet_data_chunk_free(echnk);
		  free(data);
		  data = NULL;
	       }
	  }
	else
	  {
	     switch (ede->group_type)
	       {
		case EET_G_ARRAY:
		case EET_G_VAR_ARRAY:
		    {
		       printf("ARRAY TYPE NOT IMPLIMENTED YET!!!\n");
		    }
		  break;
		case EET_G_LIST:
		    {
		       void *l;

		       l = *((void **)(((char *)data_in) + ede->offset));
		       for (; l; l = edd->func.list_next(l))
			 {
			    if ((ede->type >= EET_T_CHAR) &&
				(ede->type <= EET_T_STRING))
			      data = eet_data_put_type(ede->type,
						       edd->func.list_data(l),
						       &size);
			    else if (ede->subtype)
			      data = eet_data_descriptor_encode(ede->subtype,
								edd->func.list_data(l),
								&size);
			    if (data)
			      {
				 echnk = eet_data_chunk_new(data, size, ede->name);
				 eet_data_chunk_put(echnk, ds);
				 eet_data_chunk_free(echnk);
				 free(data);
				 data = NULL;
			      }
			 }
		    }
		  break;
		case EET_G_HASH:
		    {
		       printf("HASH TYPE NOT IMPLIMENTED YET!!!\n");
		    }
		  break;
		default:
		  break;
	       }
	  }
     }
   chnk = eet_data_chunk_new(ds->data, ds->pos, edd->name);
   ds->data = NULL;
   ds->size = 0;
   eet_data_stream_free(ds);

   ds = eet_data_stream_new();
   eet_data_chunk_put(chnk, ds);
   cdata = ds->data;
   csize = ds->pos;

   ds->data = NULL;
   ds->size = 0;
   eet_data_stream_free(ds);
   *size_ret = csize;

   free(chnk->data);
   eet_data_chunk_free(chnk);

   return cdata;
}
