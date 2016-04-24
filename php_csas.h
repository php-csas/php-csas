/*
  +----------------------------------------------------------------------+
  | Taint                                                                |
  +----------------------------------------------------------------------+
  | Copyright (c) 2012-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Xinchen Hui    <laruence@php.net>                           |
  +----------------------------------------------------------------------+

  +----------------------------------------------------------------------+
  | CSAS                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2012-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Matt Van Gundy    <mvangund@cisco.com>                      |
  |          Jared Smith       <jms@vols.utk.edu>                        |
  |          Joseph Connor     <rconnor6@vols.utk.edu>                   |
  |          David Cunningham  <davpcunn@vols.utk.edu>                   |
  |          Kyle Bashour      <kbashour@vols.utk.edu>                   |
  |          Travis working    <wwork@vols.utk.edu>                      |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_CSAS_H
#define PHP_CSAS_H

extern zend_module_entry csas_module_entry;
#define phpext_csas_ptr &csas_module_entry

#ifdef PHP_WIN32
#define PHP_CSAS_API __declspec(dllexport)
#else
#define PHP_CSAS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_CSAS_VERSION "1.3.0-dev"

#define PHP_CSAS_MAGIC_NUMBER           0x6A8FCE84
#define PHP_CSAS_MAGIC_LENGTH   (sizeof(unsigned)*2)

#define PHP_CSAS_UNSAFE            0
#define PHP_CSAS_SAFE_PCDATA       (1 << 1)
#define PHP_CSAS_SAFE_ATTR_QUOT    (1 << 2)
#define PHP_CSAS_SAFE_ATTR_UNQUOT  (1 << 3)
#define PHP_CSAS_SAFE_URL_START    (1 << 4)
#define PHP_CSAS_SAFE_URL_QUERY    (1 << 5)
#define PHP_CSAS_SAFE_URL_GENERAL  (1 << 6)
#define PHP_CSAS_SAFE_JS_STRING    (1 << 7)
#define PHP_CSAS_SAFE_ALL          0xFFFFFFFF

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
#  define CSAS_OP1_TYPE(n)         ((n)->op1.op_type)
#  define CSAS_OP2_TYPE(n)         ((n)->op2.op_type)
#  define CSAS_OP1_NODE_PTR(n)     (&(n)->op1)
#  define CSAS_OP2_NODE_PTR(n)     (&(n)->op2)
#  define CSAS_OP1_VAR(n)          ((n)->op1.u.var)
#  define CSAS_OP2_VAR(n)          ((n)->op2.u.var)
#  define CSAS_RESULT_VAR(n)       ((n)->result.u.var)
#  define CSAS_OP1_CONSTANT_PTR(n) (&(n)->op1.u.constant)
#  define CSAS_OP2_CONSTANT_PTR(n) (&(n)->op2.u.constant)
#  define CSAS_GET_ZVAL_PTR_CV_2ND_ARG(t) (execute_data->Ts)
#  define CSAS_RETURN_VALUE_USED(n) (!((&(n)->result)->u.EA.type & EXT_TYPE_UNUSED))
#  define CSAS_OP_LINENUM(n)       ((n).u.opline_num)
#  define CSAS_AI_SET_PTR(ai, val)    	\
	(ai).ptr = (val);					\
	(ai).ptr_ptr = &((ai).ptr);
#else
#  define CSAS_OP1_TYPE(n)         ((n)->op1_type)
#  define CSAS_OP2_TYPE(n)         ((n)->op2_type)
#  define CSAS_OP1_NODE_PTR(n)     ((n)->op1.var)
#  define CSAS_OP2_NODE_PTR(n)     ((n)->op2.var)
#  define CSAS_OP1_VAR(n)          ((n)->op1.var)
#  define CSAS_OP2_VAR(n)          ((n)->op2.var)
#  define CSAS_RESULT_VAR(n)       ((n)->result.var)
#  define CSAS_OP1_CONSTANT_PTR(n) ((n)->op1.zv)
#  define CSAS_OP2_CONSTANT_PTR(n) ((n)->op2.zv)
#  define CSAS_GET_ZVAL_PTR_CV_2ND_ARG(t) (t)
#  define CSAS_RETURN_VALUE_USED(n) (!((n)->result_type & EXT_TYPE_UNUSED))
#  define CSAS_OP_LINENUM(n)       ((n).opline_num)
#  define CSAS_AI_SET_PTR(t, val) do {		\
		temp_variable *__t = (t);			\
		__t->var.ptr = (val);				\
		__t->var.ptr_ptr = &__t->var.ptr;	\
	} while (0)
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3)
#  define CSAS_ARG_PUSH(v)         zend_ptr_stack_push(&EG(argument_stack), v TSRMLS_CC)
#else
#  define CSAS_ARG_PUSH(v)         zend_vm_stack_push(v TSRMLS_CC)
#endif

#ifndef Z_SET_ISREF_PP
#  define Z_SET_ISREF_PP(n) ((*n)->is_ref = 1)
#endif
#ifndef Z_UNSET_ISREF_PP
#  define Z_UNSET_ISREF_PP(n)  ((*n)->is_ref = 0)
#endif
#ifndef Z_REFCOUNT_PP
#  define Z_REFCOUNT_PP(n)  ((*n)->refcount)
#endif

#ifndef INIT_PZVAL_COPY
#define INIT_PZVAL_COPY(z,v) \
	(z)->value = (v)->value; \
	Z_TYPE_P(z) = Z_TYPE_P(v); \
	(z)->refcount = 1; \
	(z)->is_ref = 0;
#endif

#ifndef MAKE_REAL_ZVAL_PTR
#define MAKE_REAL_ZVAL_PTR(val) \
    do { \
        zval *_tmp; \
        ALLOC_ZVAL(_tmp); \
        INIT_PZVAL_COPY(_tmp, (val)); \
        (val) = _tmp; \
    } while (0)
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
#define CSAS_T(offset) (*EX_TMP_VAR(execute_data, offset))
#define CSAS_CV(i)     (*EX_CV_NUM(execute_data, i))
#define CSAS_CV_OF(i)   (*EX_CV_NUM(EG(current_execute_data), i))
#define CSAS_PZVAL_LOCK(z)  Z_ADDREF_P((z))
#else
#define CSAS_T(offset) (*(temp_variable *)((char *) execute_data->Ts + offset))
#define CSAS_CV(i)     (EG(current_execute_data)->CVs[i])
#define CSAS_CV_OF(i)     (EG(current_execute_data)->CVs[i])
#define CSAS_PZVAL_LOCK(z, f) csas_pzval_lock_func(z, f);
#endif

#define CSAS_TS(offset) (*(temp_variable *)((char *)Ts + offset))



#define CSAS_PZVAL_UNLOCK(z, f) csas_pzval_unlock_func(z, f, 1)
#define CSAS_PZVAL_UNLOCK_FREE(z) csas_pzval_unlock_free_func(z)

#define CSAS_CV_DEF_OF(i) (EG(active_op_array)->vars[i])
#define CSAS_TMP_FREE(z) (zval*)(((zend_uintptr_t)(z)) | 1L)

#define CSAS_AI_USE_PTR(ai) \
	if ((ai).ptr_ptr) { \
		(ai).ptr = *((ai).ptr_ptr); \
		(ai).ptr_ptr = &((ai).ptr); \
	} else { \
		(ai).ptr = NULL; \
	}

#define CSAS_AI_SET_PTR(t, val) do {   \
        temp_variable *__t = (t); \
        __t->var.ptr = (val);  \
        __t->var.ptr_ptr = &__t->var.ptr; \
    } while (0)

#define CSAS_FREE_OP(should_free) \
	if (should_free.var) { \
		if ((zend_uintptr_t)should_free.var & 1L) { \
			zval_dtor((zval*)((zend_uintptr_t)should_free.var & ~1L)); \
		} else { \
			zval_ptr_dtor(&should_free.var); \
		} \
	}
#define CSAS_FREE_OP_VAR_PTR(should_free) \
	if (should_free.var) { \
		zval_ptr_dtor(&should_free.var); \
	}

#define PHP_CSAS_SET_SAFETY_(zv, mark) *((unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1 + sizeof(uint))) = (mark)
#define PHP_CSAS_GET_SAFETY_(zv) ((*((unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1 + sizeof(uint)))))

#define PHP_CSAS_IS_MARKED(zv) ((*((unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1))) == PHP_CSAS_MAGIC_NUMBER)
#define PHP_CSAS_SET_MARKED(zv) ((*((unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1))) = PHP_CSAS_MAGIC_NUMBER)



//#define PHP_CSAS_POSSIBLE(zv) (*(unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1) == PHP_CSAS_MAGIC_POSSIBLE)
//#define PHP_CSAS_UNCSAS(zv)  (*(unsigned *)(Z_STRVAL_P(zv) + Z_STRLEN_P(zv) + 1) == PHP_CSAS_MAGIC_UNCSAS)

#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3))
#  define Z_ADDREF_P   ZVAL_ADDREF
#  define Z_REFCOUNT_P ZVAL_REFCOUNT
#  define Z_DELREF_P   ZVAL_DELREF
#  define Z_SET_REFCOUNT_P(pz, rc)  (pz)->refcount = rc
#  define Z_UNSET_ISREF_P(pz) (pz)->is_ref = 0
#  define Z_ISREF_P(pz)       (pz)->is_ref
#endif

typedef struct  _csas_free_op {
	zval* var;
	int   is_ref;
	int   type;
} csas_free_op;

enum CsasContext { CSAS_UNUSED, CSAS_HTML, CSAS_JS, CSAS_CSS, CSAS_JSON,
                           CSAS_XML, CSAS_MANUAL };

PHP_MINIT_FUNCTION(csas);
PHP_MSHUTDOWN_FUNCTION(csas);
PHP_RINIT_FUNCTION(csas);
PHP_RSHUTDOWN_FUNCTION(csas);
PHP_MINFO_FUNCTION(csas);

PHP_FUNCTION(csas);
PHP_FUNCTION(uncsas);
PHP_FUNCTION(csas_mark_safe);
PHP_FUNCTION(csas_mark_unsafe);
PHP_FUNCTION(is_csased);

PHP_FUNCTION(csas_strval);
PHP_FUNCTION(csas_sprintf);
PHP_FUNCTION(csas_vsprintf);
PHP_FUNCTION(csas_explode);
PHP_FUNCTION(csas_implode);
PHP_FUNCTION(csas_trim);
PHP_FUNCTION(csas_rtrim);
PHP_FUNCTION(csas_ltrim);
PHP_FUNCTION(csas_strstr);
PHP_FUNCTION(csas_substr);
PHP_FUNCTION(csas_str_replace);
PHP_FUNCTION(csas_str_pad);
PHP_FUNCTION(csas_strtolower);
PHP_FUNCTION(csas_strtoupper);
PHP_FUNCTION(csas_htmlspecialchars);

PHP_FUNCTION(csas_fgetc);
PHP_FUNCTION(csas_fgetcsv);
PHP_FUNCTION(csas_fgets);
PHP_FUNCTION(csas_fgetss);
PHP_FUNCTION(csas_file_get_contents);
PHP_FUNCTION(csas_file);
PHP_FUNCTION(csas_fread);
PHP_FUNCTION(csas_fscanf);
PHP_FUNCTION(csas_socket_read);
PHP_FUNCTION(csas_socket_recv);
PHP_FUNCTION(csas_socket_recvfrom);
PHP_FUNCTION(csas_getenv);

PHP_FUNCTION(csas_printf);
PHP_FUNCTION(csas_vprintf);
PHP_FUNCTION(csas_readfile);
PHP_FUNCTION(csas_fpassthru);

PHP_FUNCTION(csas_mysqli_result_fetch_assoc);
PHP_FUNCTION(csas_mysqli_result_fetch_array);
PHP_FUNCTION(csas_mysqli_result_fetch_all);
PHP_FUNCTION(csas_mysqli_result_fetch_object);
PHP_FUNCTION(csas_mysqli_result_fetch_row);

PHP_FUNCTION(csas_mysqli_fetch_assoc);
PHP_FUNCTION(csas_mysqli_fetch_array);
PHP_FUNCTION(csas_mysqli_fetch_all);
PHP_FUNCTION(csas_mysqli_fetch_object);
PHP_FUNCTION(csas_mysqli_fetch_row);

PHP_FUNCTION(csas_pdo_fetch);
PHP_FUNCTION(csas_pdo_fetch_all);
PHP_FUNCTION(csas_pdo_fetch_column);
PHP_FUNCTION(csas_pdo_fetch_object);

PHP_FUNCTION(html_quoted_sanitize);
PHP_FUNCTION(html_unquoted_sanitize);
PHP_FUNCTION(js_sanitize);
PHP_FUNCTION(url_query_sanitize);
PHP_FUNCTION(url_start_sanitize);
PHP_FUNCTION(url_general_sanitize);

typedef void (*php_func)(INTERNAL_FUNCTION_PARAMETERS);

ZEND_BEGIN_MODULE_GLOBALS(csas)
	zend_bool enable;
	int       error_level;
ZEND_END_MODULE_GLOBALS(csas)

#ifdef ZTS
#define CSAS_G(v) TSRMG(csas_globals_id, zend_csas_globals *, v)
#else
#define CSAS_G(v) (csas_globals.v)
#endif

#endif	/* PHP_CSAS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
