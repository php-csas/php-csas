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
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stdio.h"
#include "php.h"
#include "SAPI.h"
#include "zend_compile.h"
#include "zend_execute.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_csas.h"
#include "htmlparser/htmlparser.cc"

ZEND_DECLARE_MODULE_GLOBALS(csas)

/* {{{ CSAS_ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(csas_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(1, string)
	ZEND_ARG_INFO(1, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(uncsas_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(1, string)
	ZEND_ARG_INFO(1, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(is_csased_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ csas_functions[]
 */
zend_function_entry csas_functions[] = {
	PHP_FE(csas, csas_arginfo)
	PHP_FE(uncsas, uncsas_arginfo)
	PHP_FE(is_csased, is_csased_arginfo)
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ module depends
 */
#if ZEND_MODULE_API_NO >= 20050922
zend_module_dep csas_deps[] = {
	ZEND_MOD_CONFLICTS("xdebug")
	{NULL, NULL, NULL}
};
#endif
/* }}} */

/* {{{ csas_module_entry
 */
zend_module_entry csas_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX, NULL,
	csas_deps,
#else
	STANDARD_MODULE_HEADER,
#endif
	"csas",
	csas_functions,
	PHP_MINIT(csas),
	PHP_MSHUTDOWN(csas),
	PHP_RINIT(csas),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(csas),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(csas),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_CSAS_VERSION, /* Replace with version number for your extension */
#endif
	PHP_MODULE_GLOBALS(csas),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static struct csas_overridden_fucs /* {{{ */ {
	php_func strval;
	php_func sprintf;
	php_func vsprintf;
	php_func explode;
	php_func implode;
	php_func trim;
	php_func rtrim;
	php_func ltrim;
	php_func strstr;
	php_func str_pad;
	php_func str_replace;
	php_func substr;
	php_func strtolower;
	php_func strtoupper;
} csas_origin_funcs;

#define CSAS_O_FUNC(m) (csas_origin_funcs.m)
/* }}} */

static void php_csas_mark_strings(zval *symbol_table TSRMLS_DC) /* {{{ */ {
	zval **ppzval;
	HashTable *ht = Z_ARRVAL_P(symbol_table);
	HashPosition pos = {0};

	for(zend_hash_internal_pointer_reset_ex(ht, &pos);
			zend_hash_has_more_elements_ex(ht, &pos) == SUCCESS;
			zend_hash_move_forward_ex(ht, &pos)) {
		if (zend_hash_get_current_data_ex(ht, (void**)&ppzval, &pos) == FAILURE) {
			continue;
		}
        if (Z_TYPE_PP(ppzval) == IS_ARRAY) {
			php_csas_mark_strings(*ppzval TSRMLS_CC);
		} else if (IS_STRING == Z_TYPE_PP(ppzval)) {
			Z_STRVAL_PP(ppzval) = erealloc(Z_STRVAL_PP(ppzval), Z_STRLEN_PP(ppzval) + 1 + PHP_CSAS_MAGIC_LENGTH);
		    PHP_CSAS_MARK(*ppzval, PHP_CSAS_MAGIC_POSSIBLE);
		}
	}
} /* }}} */

static void csas_pzval_unlock_func(zval *z, csas_free_op *should_free, int unref) /* {{{ */ {
    if (!Z_DELREF_P(z)) {
        Z_SET_REFCOUNT_P(z, 1);
        Z_UNSET_ISREF_P(z);
        should_free->var = z;
    } else {
        should_free->var = 0;
        if (unref && Z_ISREF_P(z) && Z_REFCOUNT_P(z) == 1) {
			should_free->is_ref = 1;
			Z_UNSET_ISREF_P(z);
        }
    }
} /* }}} */

static void csas_pzval_unlock_free_func(zval *z) /* {{{ */ {
    if (!Z_DELREF_P(z)) {
        zval_dtor(z);
        safe_free_zval_ptr(z);
    }
} /* }}} */

static void csas_pzval_lock_func(zval *z, csas_free_op *should_free) /* {{{ */ {
	if (should_free->type == IS_VAR) {
		Z_ADDREF_P(z);
		if (should_free->var && should_free->is_ref) {
			Z_SET_ISREF_P(z);
		}
	}
} /* }}} */

static void php_csas_get_cv_address(zend_compiled_variable *cv, zval ***ptr, temp_variable *Ts TSRMLS_DC) /* {{{ */ {
	zval *new_zval = &EG(uninitialized_zval);

	Z_ADDREF_P(new_zval);
	zend_hash_quick_update(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, &new_zval, sizeof(zval *), (void **)ptr);
}
/* }}} */

static zval **php_csas_get_obj_zval_ptr_ptr_unused(TSRMLS_D) /* {{{ */ {
	if (EG(This)) {
		return &EG(This);
	} else {
		zend_error(E_ERROR, "Using $this when not in object context");
		return NULL;
	}
} /* }}} */

static void make_real_object(zval **object_ptr TSRMLS_DC)  /* {{{ */ {
	if (Z_TYPE_PP(object_ptr) == IS_NULL
		|| (Z_TYPE_PP(object_ptr) == IS_BOOL && Z_LVAL_PP(object_ptr) == 0)
		|| (Z_TYPE_PP(object_ptr) == IS_STRING && Z_STRLEN_PP(object_ptr) == 0)
	) {
		zend_error(E_STRICT, "Creating default object from empty value");

		SEPARATE_ZVAL_IF_NOT_REF(object_ptr);
		zval_dtor(*object_ptr);
		object_init(*object_ptr);
	}
} /* }}} */

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
static zval * php_csas_get_zval_ptr_var(znode *node, temp_variable *Ts, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
    zval *ptr = CSAS_TS(node->u.var).var.ptr;
    if (ptr) {
        CSAS_PZVAL_UNLOCK(ptr, should_free);
        return ptr;
    } else {
        temp_variable *T = (temp_variable *)((char *)Ts + node->u.var);
        zval *str = T->str_offset.str;

        /* string offset */
        ALLOC_ZVAL(ptr);
        T->str_offset.ptr = ptr;
        should_free->var = ptr;

        if (T->str_offset.str->type != IS_STRING
            || ((int)T->str_offset.offset < 0)
            || (T->str_offset.str->value.str.len <= (int)T->str_offset.offset)) {
            ptr->value.str.val = STR_EMPTY_ALLOC();
            ptr->value.str.len = 0;
        } else {
            char c = str->value.str.val[T->str_offset.offset];

            ptr->value.str.val = estrndup(&c, 1);
            ptr->value.str.len = 1;
        }
        CSAS_PZVAL_UNLOCK_FREE(str);
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3)
        ptr->refcount = 1;
        ptr->is_ref = 1;
#else
        ptr->refcount__gc = 1;
        ptr->is_ref__gc = 1;
#endif
        ptr->type = IS_STRING;
        return ptr;
    }
} /* }}} */

static zval * php_csas_get_zval_ptr_cv(znode *node, temp_variable *Ts TSRMLS_DC) /* {{{ */ {
	zval ***ptr = &CSAS_CV_OF(node->u.var);
	if (!*ptr) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(node->u.var);
		if (!EG(active_symbol_table) || zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)ptr) == FAILURE) {
			zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
			return &EG(uninitialized_zval);
		}
	}
	return **ptr;
} /* }}} */

static zval * php_csas_get_zval_ptr_tmp(znode *node, temp_variable *Ts, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	return should_free->var = &CSAS_TS(node->u.var).tmp_var;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr_var(znode *node, temp_variable *Ts, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	zval** ptr_ptr = CSAS_TS(node->u.var).var.ptr_ptr;

	if (ptr_ptr) {
		CSAS_PZVAL_UNLOCK(*ptr_ptr, should_free);
	} else {
		/* string offset */
		CSAS_PZVAL_UNLOCK(CSAS_TS(node->u.var).str_offset.str, should_free);
	}
	return ptr_ptr;
} /* }}} */

static zval **php_csas_get_zval_ptr_ptr_cv(znode *node, temp_variable *Ts, int type TSRMLS_DC) /* {{{ */ {
	zval ***ptr = &CSAS_CV_OF(node->u.var);

	if (!*ptr) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(node->u.var);
		if (!EG(active_symbol_table) 
				|| zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void **) ptr )==FAILURE) {
			switch (type) {
				case BP_VAR_R:
				case BP_VAR_UNSET:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_IS:
					return &EG(uninitialized_zval_ptr);
					break;
				case BP_VAR_RW:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_W:
					php_csas_get_cv_address(cv, ptr, Ts TSRMLS_CC);
					break;
			}
		}
	}
	return *ptr;
} /* }}} */

static zval **php_csas_get_zval_ptr_ptr(znode *node, temp_variable *Ts, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
	should_free->type = node->op_type;
	if (node->op_type == IS_CV) {
		should_free->var = 0;
		return php_csas_get_zval_ptr_ptr_cv(node, Ts, type TSRMLS_CC);
	} else if (node->op_type == IS_VAR) {
		return php_csas_get_zval_ptr_ptr_var(node, Ts, should_free TSRMLS_CC);
	} else {
		should_free->var = 0;
		return NULL;
	}
} /* }}} */

static zval *php_csas_get_zval_ptr(znode *node, temp_variable *Ts, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
/*	should_free->is_var = 0; */
	switch (node->op_type) {
		case IS_CONST:
			should_free->var = 0;
			return &node->u.constant;
			break;
		case IS_TMP_VAR:
			should_free->var = CSAS_TMP_FREE(&CSAS_TS(node->u.var).tmp_var);
			return &CSAS_TS(node->u.var).tmp_var;
			break;
		case IS_VAR:
			return php_csas_get_zval_ptr_var(node, Ts, should_free TSRMLS_CC);
			break;
		case IS_UNUSED:
			should_free->var = 0;
			return NULL;
			break;
		case IS_CV:
			should_free->var = 0;
			return php_csas_get_zval_ptr_cv(node, Ts TSRMLS_CC);
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return NULL;
} /* }}} */

static int php_csas_qm_assign_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL;
	csas_free_op free_op1 = {0};

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
			break;
		case IS_VAR:
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
			break;
		case IS_CV:
			op1 = php_csas_get_zval_ptr_cv(CSAS_OP1_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
			op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}

	CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var = *op1;

	if (!((zend_uintptr_t)free_op1.var & 1L)) {
		zval_copy_ctor(&CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var);
		if (op1 && IS_STRING == Z_TYPE_P(op1) && PHP_CSAS_POSSIBLE(op1)) {
			zval *result = &CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var;
			Z_STRVAL_P(result) = erealloc(Z_STRVAL_P(result), Z_STRLEN_P(result) + 1 + PHP_CSAS_MAGIC_LENGTH);
			PHP_CSAS_MARK(result, PHP_CSAS_MAGIC_POSSIBLE);
		}
	}

	switch (CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
			zval_dtor(free_op1.var);
			break;
		case IS_VAR:
			if (free_op1.var) {
				zval_ptr_dtor(&free_op1.var);
			}
			break;
	}

	execute_data->opline++;

	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
static zval * php_csas_get_zval_ptr_var(zend_uint var, const zend_execute_data *execute_data, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	zval *ptr = CSAS_T(var).var.ptr;
	CSAS_PZVAL_UNLOCK(ptr, should_free);
	return ptr;
} /* }}} */

static zval * php_csas_get_zval_ptr_cv(zend_uint var, int type TSRMLS_DC) /* {{{ */ {
	zval ***ptr = EX_CV_NUM(EG(current_execute_data), var);

	if (UNEXPECTED(*ptr == NULL)) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(var);
		if (!EG(active_symbol_table) || zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)ptr) == FAILURE) {
			zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
			return &EG(uninitialized_zval);
		}
	}
	return **ptr;
} /* }}} */

static zval * php_csas_get_zval_ptr_tmp(zend_uint var, const zend_execute_data *execute_data, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	return should_free->var = &CSAS_T(var).tmp_var;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr_var(zend_uint var, const zend_execute_data *execute_data, csas_free_op *should_free TSRMLS_DC)/* {{{ */ {
	zval** ptr_ptr = CSAS_T(var).var.ptr_ptr;

	if (EXPECTED(ptr_ptr != NULL)) {
		CSAS_PZVAL_UNLOCK(*ptr_ptr, should_free);
	} else {
		/* string offset */
		CSAS_PZVAL_UNLOCK(CSAS_T(var).str_offset.str, should_free);
	}
	return ptr_ptr;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr_cv(zend_uint var, int type TSRMLS_DC) /* {{{ */  {
	zval ***ptr = &CSAS_CV_OF(var);

	if (UNEXPECTED(*ptr == NULL)) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(var);
		if (!EG(active_symbol_table) 
				|| zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void **) ptr )==FAILURE) {
			switch (type) {
				case BP_VAR_R:
				case BP_VAR_UNSET:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_IS:
					return &EG(uninitialized_zval_ptr);
					break;
				case BP_VAR_RW:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_W:
					Z_ADDREF(EG(uninitialized_zval));
					if (!EG(active_symbol_table)) {
						*ptr = (zval**)EX_CV_NUM(EG(current_execute_data), EG(active_op_array)->last_var + var);
						**ptr = &EG(uninitialized_zval);
					} else {
						zend_hash_quick_update(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, &EG(uninitialized_zval_ptr), sizeof(zval *), (void **)ptr);
					}
					break;
			}
		}
	}
	return *ptr;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr(int op_type, const znode_op *node, const zend_execute_data *execute_data, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
	should_free->type = op_type;
	if (op_type == IS_CV) {
		should_free->var = 0;
		return php_csas_get_zval_ptr_ptr_cv(node->var, type TSRMLS_CC);
	} else if (op_type == IS_VAR) {
		return php_csas_get_zval_ptr_ptr_var(node->var, execute_data, should_free TSRMLS_CC);
	} else {
		should_free->var = 0;
		return NULL;
	}
} /* }}} */

static zval *php_csas_get_zval_ptr(int op_type, const znode_op *node, const zend_execute_data *execute_data, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
/*	should_free->is_var = 0; */
	switch (op_type) {
		case IS_CONST:
			should_free->var = 0;
			return node->zv;
			break;
		case IS_TMP_VAR:
			should_free->var = CSAS_TMP_FREE(&CSAS_T(node->var).tmp_var);
			return &CSAS_T(node->var).tmp_var;
			break;
		case IS_VAR:
			return php_csas_get_zval_ptr_var(node->var, execute_data, should_free TSRMLS_CC);
			break;
		case IS_UNUSED:
			should_free->var = 0;
			return NULL;
			break;
		case IS_CV:
			should_free->var = 0;
			return php_csas_get_zval_ptr_cv(node->var, type TSRMLS_CC);
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return NULL;
} /* }}} */ 
#else
static zval * php_csas_get_zval_ptr_var(zend_uint var, const temp_variable *Ts, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	zval *ptr = CSAS_TS(var).var.ptr;
	CSAS_PZVAL_UNLOCK(ptr, should_free);
	return ptr;
} /* }}} */

static zval * php_csas_get_zval_ptr_cv(zend_uint var, int type TSRMLS_DC) /* {{{ */ {
	zval ***ptr = &CSAS_CV_OF(var);

	if (UNEXPECTED(*ptr == NULL)) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(var);
		if (!EG(active_symbol_table) || zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)ptr) == FAILURE) {
			zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
			return &EG(uninitialized_zval);
		}
	}
	return **ptr;
} /* }}} */

static zval * php_csas_get_zval_ptr_tmp(zend_uint var, const temp_variable *Ts, csas_free_op *should_free TSRMLS_DC) /* {{{ */ {
	return should_free->var = &CSAS_TS(var).tmp_var;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr_var(zend_uint var, const temp_variable *Ts, csas_free_op *should_free TSRMLS_DC)/* {{{ */ {
	zval** ptr_ptr = CSAS_TS(var).var.ptr_ptr;

	if (EXPECTED(ptr_ptr != NULL)) {
		CSAS_PZVAL_UNLOCK(*ptr_ptr, should_free);
	} else {
		/* string offset */
		CSAS_PZVAL_UNLOCK(CSAS_TS(var).str_offset.str, should_free);
	}
	return ptr_ptr;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr_cv(zend_uint var, int type TSRMLS_DC) /* {{{ */  {
	zval ***ptr = &CSAS_CV_OF(var);

	if (UNEXPECTED(*ptr == NULL)) {
		zend_compiled_variable *cv = &CSAS_CV_DEF_OF(var);
		if (!EG(active_symbol_table) 
				|| zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void **) ptr )==FAILURE) {
			switch (type) {
				case BP_VAR_R:
				case BP_VAR_UNSET:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_IS:
					return &EG(uninitialized_zval_ptr);
					break;
				case BP_VAR_RW:
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					/* break missing intentionally */
				case BP_VAR_W:
					Z_ADDREF(EG(uninitialized_zval));
					if (!EG(active_symbol_table)) {
						*ptr = (zval**)EG(current_execute_data)->CVs + (EG(active_op_array)->last_var + var);
						**ptr = &EG(uninitialized_zval);
					} else {
						zend_hash_quick_update(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, &EG(uninitialized_zval_ptr), sizeof(zval *), (void **)ptr);
					}
					break;
			}
		}
	}
	return *ptr;
} /* }}} */

static zval ** php_csas_get_zval_ptr_ptr(int op_type, const znode_op *node, const temp_variable *Ts, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
	should_free->type = op_type;
	if (op_type == IS_CV) {
		should_free->var = 0;
		return php_csas_get_zval_ptr_ptr_cv(node->var, type TSRMLS_CC);
	} else if (op_type == IS_VAR) {
		return php_csas_get_zval_ptr_ptr_var(node->var, Ts, should_free TSRMLS_CC);
	} else {
		should_free->var = 0;
		return NULL;
	}
} /* }}} */

static zval *php_csas_get_zval_ptr(int op_type, const znode_op *node, const temp_variable *Ts, csas_free_op *should_free, int type TSRMLS_DC) /* {{{ */ {
/*	should_free->is_var = 0; */
	switch (op_type) {
		case IS_CONST:
			should_free->var = 0;
			return node->zv;
			break;
		case IS_TMP_VAR:
			should_free->var = CSAS_TMP_FREE(&CSAS_TS(node->var).tmp_var);
			return &CSAS_TS(node->var).tmp_var;
			break;
		case IS_VAR:
			return php_csas_get_zval_ptr_var(node->var, Ts, should_free TSRMLS_CC);
			break;
		case IS_UNUSED:
			should_free->var = 0;
			return NULL;
			break;
		case IS_CV:
			should_free->var = 0;
			return php_csas_get_zval_ptr_cv(node->var, type TSRMLS_CC);
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return NULL;
} /* }}} */
#endif

static void php_csas_error(const char *docref TSRMLS_DC, const char *format, ...) /* {{{ */ {
	va_list args;
	va_start(args, format);
	php_verror(docref, "", CSAS_G(error_level), format, args TSRMLS_CC);
	va_end(args);
} /* }}} */

char* cast_zval_to_string(zval *z) {
	if (Z_TYPE_P(z) != IS_STRING) {
	    return NULL;
	}
	return Z_STRVAL_P(z);
}

static int php_csas_echo_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL;
	csas_free_op free_op1 = {0};

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr;
			break;
		case IS_CV: {
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = *t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = *t;
					}
				}
		    }
			break;
	}

	// op1 at this point could be null but it represents an opcode
	if (op1 != NULL) {
		php_printf("<br>");
		php_printf("OP1 is not Null, printing OP1");
		php_printf("<br>");
		char* s = cast_zval_to_string(op1);
		if (s != NULL) {
			php_printf("<br>");
			php_printf("%s", s);
			php_printf("<br>");
		}
		// Now we check if op1 is a string, and if its possible for it to be csased
		if (op1 && IS_STRING == Z_TYPE_P(op1) && PHP_CSAS_POSSIBLE(op1)) {
			php_printf("<br>");
			php_printf("OP1 is also possibly csased, printing OP1\n");
			php_printf("<br>");
			char* s = cast_zval_to_string(op1);
			if (s != NULL) {
				php_printf("<br>");
				php_printf("%s", s);
				php_printf("<br>");
			}
			// Here, ZEND_ECHO is the signature for the echo language construct. If the opcode associated with the opline handled in this function is an echo construct, then print out the appropriate error that you are trying to echo possibly csased output.
// The other case is that it is a print construct.
			if (ZEND_ECHO == opline->opcode) {
				php_csas_error("function.echo" TSRMLS_CC, "Attempt to echo a string that might be csased");
			} else {
				php_csas_error("function.print" TSRMLS_CC, "Attempt to print a string that might be csased");
			}
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL;
	csas_free_op free_op1 = {0};

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr;
			break;
		case IS_CV: {
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = *t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = *t;
					}
				}
		    }
			break;
		case IS_CONST:
	 		op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}

	if ((op1 && IS_STRING == Z_TYPE_P(op1) && PHP_CSAS_POSSIBLE(op1)))
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
		switch (Z_LVAL(opline->op2.u.constant)) {
#else
		switch (opline->extended_value) {
#endif
			case ZEND_INCLUDE_ONCE:
				php_csas_error("function.include_once" TSRMLS_CC, "File path contains data that might be csased");
				break;
			case ZEND_REQUIRE_ONCE:
				php_csas_error("function.require_once" TSRMLS_CC, "File path contains data that might be csased");
				break;
			case ZEND_INCLUDE:
				php_csas_error("function.include" TSRMLS_CC, "File path contains data that might be csased");
				break;
			case ZEND_REQUIRE:
				php_csas_error("function.require" TSRMLS_CC, "File path contains data that might be csased");
				break;
			case ZEND_EVAL:
				php_csas_error("function.eval" TSRMLS_CC, "Eval code contains data that might be csased");
				break;
		}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_concat_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL, *op2 = NULL, *result;
	csas_free_op free_op1 = {0}, free_op2 = {0};
	uint csased = 0;

	result = &CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var;
	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			op1 = php_csas_get_zval_ptr_cv(CSAS_OP1_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}

	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op2 = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			op2 = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op2 = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			op2 = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			op2 = php_csas_get_zval_ptr_cv(CSAS_OP2_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op2 = CSAS_OP2_CONSTANT_PTR(opline);
			break;
	}

	if ((op1 && IS_STRING == Z_TYPE_P(op1) && PHP_CSAS_POSSIBLE(op1))
			|| (op2 && IS_STRING == Z_TYPE_P(op2) && PHP_CSAS_POSSIBLE(op2))) {
		csased = 1;
	}

	concat_function(result, op1, op2 TSRMLS_CC);

	if (csased && IS_STRING == Z_TYPE_P(result)) {
		Z_STRVAL_P(result) = erealloc(Z_STRVAL_P(result), Z_STRLEN_P(result) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(result, PHP_CSAS_MAGIC_POSSIBLE);
	}

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
			zval_dtor(free_op1.var);
			break;
		case IS_VAR:
			if (free_op1.var) {
				zval_ptr_dtor(&free_op1.var);
			}
			break;
	}

	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
			zval_dtor(free_op2.var);
			break;
		case IS_VAR:
			if (free_op2.var) {
				zval_ptr_dtor(&free_op2.var);
			}
			break;
	}

	execute_data->opline++;

	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static zval **php_csas_fetch_dimension_address_inner(HashTable *ht, zval *dim, int dim_type, int type TSRMLS_DC) /* {{{ */ {
	zval **retval;
	char *offset_key;
	int offset_key_length;
	ulong hval;

	switch (dim->type) {
		case IS_NULL:
			offset_key = "";
			offset_key_length = 0;
		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
			hval = zend_inline_hash_func("", 1);
		#endif
			goto fetch_string_dim;

		case IS_STRING:
			offset_key = dim->value.str.val;
			offset_key_length = dim->value.str.len;
		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
			if (dim_type == IS_CONST) {
				hval = Z_HASH_P(dim);
			} else {
				ZEND_HANDLE_NUMERIC_EX(offset_key, offset_key_length+1, hval, goto num_index);
				if (IS_INTERNED(offset_key)) {
					hval = INTERNED_HASH(offset_key);
				} else {
					hval = zend_hash_func(offset_key, offset_key_length+1);
				}
			}
		#endif
			
fetch_string_dim:
		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
			if (zend_symtable_find(ht, offset_key, offset_key_length+1, (void **) &retval) == FAILURE) {
		#else
			if (zend_hash_quick_find(ht, offset_key, offset_key_length+1, hval, (void **) &retval) == FAILURE) {
		#endif
				switch (type) {
					case BP_VAR_R:
						zend_error(E_NOTICE, "Undefined index: %s", offset_key);
						/* break missing intentionally */
					case BP_VAR_UNSET:
					case BP_VAR_IS:
						retval = &EG(uninitialized_zval_ptr);
						break;
					case BP_VAR_RW:
						zend_error(E_NOTICE,"Undefined index: %s", offset_key);
						/* break missing intentionally */
					case BP_VAR_W: {
							zval *new_zval = &EG(uninitialized_zval);
							Z_ADDREF_P(new_zval);
						#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
							zend_symtable_update(ht, offset_key, offset_key_length+1, &new_zval, sizeof(zval *), (void **) &retval);
						#else
							zend_hash_quick_update(ht, offset_key, offset_key_length+1, hval, &new_zval, sizeof(zval *), (void **) &retval);
						#endif
						}
						break;
				}
			}
		#if 0
			}
		#endif
			break;
		case IS_DOUBLE:
			hval = zend_dval_to_lval(Z_DVAL_P(dim));
			goto num_index;
		case IS_RESOURCE:
			zend_error(E_STRICT, "Resource ID#%ld used as offset, casting to integer (%ld)", Z_LVAL_P(dim), Z_LVAL_P(dim));
			/* Fall Through */
		case IS_BOOL:
		case IS_LONG:
			hval = Z_LVAL_P(dim);
num_index:
			if (zend_hash_index_find(ht, hval, (void **) &retval) == FAILURE) {
				switch (type) {
					case BP_VAR_R:
						zend_error(E_NOTICE,"Undefined offset: %ld", hval);
						/* break missing intentionally */
					case BP_VAR_UNSET:
					case BP_VAR_IS:
						retval = &EG(uninitialized_zval_ptr);
						break;
					case BP_VAR_RW:
						zend_error(E_NOTICE,"Undefined offset: %ld", hval);
						/* break missing intentionally */
					case BP_VAR_W: {
						zval *new_zval = &EG(uninitialized_zval);

						Z_ADDREF_P(new_zval);
						zend_hash_index_update(ht, hval, &new_zval, sizeof(zval *), (void **) &retval);
					}
					break;
				}
			}
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return (type == BP_VAR_W || type == BP_VAR_RW) ?
				&EG(error_zval_ptr) : &EG(uninitialized_zval_ptr);
	}
	return retval;
} /* }}} */

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
static void php_csas_fetch_dimension_address(temp_variable *result, zval **container_ptr, zval *dim, int dim_is_tmp_var, int type TSRMLS_DC)
#else
static void php_csas_fetch_dimension_address(temp_variable *result, zval **container_ptr, zval *dim, int dim_type, int type TSRMLS_DC)
#endif
{
	zval *container = *container_ptr;
	zval **retval;

	switch (Z_TYPE_P(container)) {

		case IS_ARRAY:
			if (type != BP_VAR_UNSET && Z_REFCOUNT_P(container)>1 && !Z_ISREF_P(container)) {
				SEPARATE_ZVAL(container_ptr);
				container = *container_ptr;
			}
fetch_from_array:
			if (dim == NULL) {
				zval *new_zval = &EG(uninitialized_zval);

				Z_ADDREF_P(new_zval);
				if (zend_hash_next_index_insert(Z_ARRVAL_P(container), &new_zval, sizeof(zval *), (void **) &retval) == FAILURE) {
					zend_error(E_WARNING, "Cannot add element to the array as the next element is already occupied");
					retval = &EG(error_zval_ptr);
					Z_DELREF_P(new_zval);
				}
			} else {
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				retval = php_csas_fetch_dimension_address_inner(Z_ARRVAL_P(container), dim, 0, type TSRMLS_CC);
			#else
				retval = php_csas_fetch_dimension_address_inner(Z_ARRVAL_P(container), dim, dim_type, type TSRMLS_CC);
			#endif
			}
			result->var.ptr_ptr = retval;
			Z_ADDREF_P(*retval);
			return;
			break;

		case IS_NULL:
			if (container == &EG(error_zval)) {
				result->var.ptr_ptr = &EG(error_zval_ptr);
				Z_ADDREF_P(EG(error_zval_ptr));
			} else if (type != BP_VAR_UNSET) {
convert_to_array:
				if (!Z_ISREF_P(container)) {
					SEPARATE_ZVAL(container_ptr);
					container = *container_ptr;
				}
				zval_dtor(container);
				array_init(container);
				goto fetch_from_array;
			} else {
				/* for read-mode only */
				result->var.ptr_ptr = &EG(uninitialized_zval_ptr);
				Z_ADDREF_P(EG(uninitialized_zval_ptr));
			}
			return;
			break;

		case IS_STRING: {
				zval tmp;

				if (type != BP_VAR_UNSET && Z_STRLEN_P(container)==0) {
					goto convert_to_array;
				}
				if (dim == NULL) {
					zend_error(E_ERROR, "[] operator not supported for strings");
					return;
				}

				if (Z_TYPE_P(dim) != IS_LONG) {

					switch(Z_TYPE_P(dim)) {
						/* case IS_LONG: */
						case IS_STRING:
							if (IS_LONG == is_numeric_string(Z_STRVAL_P(dim), Z_STRLEN_P(dim), NULL, NULL, -1)) {
								break;
							}
							if (type != BP_VAR_UNSET) {
								zend_error(E_WARNING, "Illegal string offset '%s'", dim->value.str.val);
							}

							break;
						case IS_DOUBLE:
						case IS_NULL:
						case IS_BOOL:
							zend_error(E_NOTICE, "String offset cast occurred");
							break;
						default:
							zend_error(E_WARNING, "Illegal offset type");
							break;
					}

					tmp = *dim;
					zval_copy_ctor(&tmp);
					convert_to_long(&tmp);
					dim = &tmp;
				}
				if (type != BP_VAR_UNSET) {
					SEPARATE_ZVAL_IF_NOT_REF(container_ptr);
				}
				container = *container_ptr;
				result->str_offset.str = container;
				Z_ADDREF_P(container);
				result->str_offset.offset = Z_LVAL_P(dim);
				result->str_offset.ptr_ptr = NULL;
				return;
			}
			break;

		case IS_OBJECT:
			if (!Z_OBJ_HT_P(container)->read_dimension) {
				zend_error(E_ERROR, "Cannot use object as array");
				return;
			} else {
				zval *overloaded_result;
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				if (dim_is_tmp_var) {
			#else
				if (dim_type == IS_TMP_VAR) {
			#endif
					zval *orig = dim;
					MAKE_REAL_ZVAL_PTR(dim);
					ZVAL_NULL(orig);
				}
			#if 0
				}
			#endif
				overloaded_result = Z_OBJ_HT_P(container)->read_dimension(container, dim, type TSRMLS_CC);

				if (overloaded_result) {
					if (!Z_ISREF_P(overloaded_result)) {
						if (Z_REFCOUNT_P(overloaded_result) > 0) {
							zval *tmp = overloaded_result;

							ALLOC_ZVAL(overloaded_result);
							/* ZVAL_COPY_VALUE(overloaded_result, tmp); */
							overloaded_result->value = tmp->value;
							Z_TYPE_P(overloaded_result) = Z_TYPE_P(tmp);
							zval_copy_ctor(overloaded_result);
							Z_UNSET_ISREF_P(overloaded_result);
							Z_SET_REFCOUNT_P(overloaded_result, 0);
						}
						if (Z_TYPE_P(overloaded_result) != IS_OBJECT) {
							zend_class_entry *ce = Z_OBJCE_P(container);
							zend_error(E_NOTICE, "Indirect modification of overloaded element of %s has no effect", ce->name);
						}
					}
					retval = &overloaded_result;
				} else {
					retval = &EG(error_zval_ptr);
				}
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				CSAS_AI_SET_PTR(result->var, *retval);
			#else
				CSAS_AI_SET_PTR(result, *retval);
			#endif
				Z_ADDREF_P(*retval);
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				if (dim_is_tmp_var) {
			#else
				if (dim_type == IS_TMP_VAR) {
			#endif
					zval_ptr_dtor(&dim);
				}
			#if 0
				}
			#endif
			}
			return;
			break;

		case IS_BOOL:
			if (type != BP_VAR_UNSET && Z_LVAL_P(container)==0) {
				goto convert_to_array;
			}
			/* break missing intentionally */

		default:
			if (type == BP_VAR_UNSET) {
				zend_error(E_WARNING, "Cannot unset offset in a non-array variable");
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				CSAS_AI_SET_PTR(result->var, EG(uninitialized_zval_ptr));
			#else
				CSAS_AI_SET_PTR(result, &EG(uninitialized_zval));
			#endif
				Z_ADDREF_P(&EG(uninitialized_zval));
			} else {
				zend_error(E_WARNING, "Cannot use a scalar value as an array");
				result->var.ptr_ptr = &EG(error_zval_ptr);
				Z_ADDREF_P(EG(error_zval_ptr));
			}
			break;
	}
#if 0
}
#endif
}

static int php_csas_binary_assign_op_obj_helper(int (*binary_op)(zval *result, zval *op1, zval *op2 TSRMLS_DC), ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
	zend_op *opline = execute_data->opline;
	zend_op *op_data = opline+1;
	csas_free_op free_op1 = {0}, free_op2 = {0}, free_op_data1 = {0};
	zval **object_ptr = NULL, *object = NULL, *property = NULL;
	int have_get_ptr = 0;
	uint csased = 0;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
	zval *value = php_csas_get_zval_ptr(&op_data->op1, execute_data->Ts, &free_op_data1, BP_VAR_R TSRMLS_CC);
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
	zval *value = php_csas_get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data, &free_op_data1, BP_VAR_R TSRMLS_CC);
#else
	zval *value = php_csas_get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data->Ts, &free_op_data1, BP_VAR_R TSRMLS_CC);
#endif
	zval **retval = &CSAS_T(CSAS_RESULT_VAR(opline)).var.ptr;

	switch (CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			object_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			object_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			if (!object_ptr) {
				zend_error(E_ERROR, "Cannot use string offset as an object");
				return 0;
			}
			break;
		case IS_CV:
		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
			object_ptr = php_csas_get_zval_ptr_ptr_cv(&opline->op1, execute_data->Ts, BP_VAR_W TSRMLS_CC);
		#else
			object_ptr = php_csas_get_zval_ptr_ptr_cv(opline->op1.var, BP_VAR_W TSRMLS_CC);
		#endif
			break;
		case IS_UNUSED:
			object_ptr = php_csas_get_obj_zval_ptr_ptr_unused(TSRMLS_C);
			break;
		default:
			/* do nothing */
			break;
	}
	
	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			property = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			property = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			property = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			property = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			property = php_csas_get_zval_ptr_cv(CSAS_OP2_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
			property = CSAS_OP2_CONSTANT_PTR(opline);
			break;
		case IS_UNUSED:
			property = NULL;
			break;
		default:
			/* do nothing */
			break;
	}
	
	CSAS_T(CSAS_RESULT_VAR(opline)).var.ptr_ptr = NULL;
	make_real_object(object_ptr TSRMLS_CC);
	object = *object_ptr;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		zend_error(E_WARNING, "Attempt to assign property of non-object");
		switch(CSAS_OP2_TYPE(opline)) {
			case IS_TMP_VAR:
				zval_dtor(free_op2.var);
				break;
			case IS_VAR:
				if (free_op2.var) {zval_ptr_dtor(&free_op2.var);};
				break;
			case IS_CV:
			case IS_CONST:
			case IS_UNUSED:
			default:
				/* do nothing */
				break;
		}
		CSAS_FREE_OP(free_op_data1);

		if (CSAS_RETURN_VALUE_USED(opline)) {
			*retval = EG(uninitialized_zval_ptr);
			Z_ADDREF_P(*retval);
		}
	} else {
		/* here we are sure we are dealing with an object */
		if (IS_TMP_VAR == CSAS_OP2_TYPE(opline)) {
			MAKE_REAL_ZVAL_PTR(property);
		}

		/* here property is a string */
		if (opline->extended_value == ZEND_ASSIGN_OBJ
			&& Z_OBJ_HT_P(object)->get_property_ptr_ptr) {
		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
			zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property TSRMLS_CC);
		#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, BP_VAR_RW, ((CSAS_OP2_TYPE(opline) == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
		#else
			zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, ((IS_CONST == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
		#endif
			if (zptr != NULL) { 			/* NULL means no success in getting PTR */
				if ((*zptr && IS_STRING == Z_TYPE_PP(zptr) && Z_STRLEN_PP(zptr) && PHP_CSAS_POSSIBLE(*zptr)) 
					|| (value && IS_STRING == Z_TYPE_P(value) && Z_STRLEN_P(value) && PHP_CSAS_POSSIBLE(value))){
					csased = 1;
				}
				
				SEPARATE_ZVAL_IF_NOT_REF(zptr);
				have_get_ptr = 1;
				
				binary_op(*zptr, *zptr, value TSRMLS_CC);
				if (csased && IS_STRING == Z_TYPE_PP(zptr) && Z_STRLEN_PP(zptr)) {
					Z_STRVAL_PP(zptr) = erealloc(Z_STRVAL_PP(zptr), Z_STRLEN_PP(zptr) + 1 + PHP_CSAS_MAGIC_LENGTH);
					PHP_CSAS_MARK(*zptr, PHP_CSAS_MAGIC_POSSIBLE);
				}
				if (CSAS_RETURN_VALUE_USED(opline)) {
					*retval = *zptr;
					Z_ADDREF_P(*retval);
				}
			}
		}

		if (!have_get_ptr) {
			zval *z = NULL;

			switch (opline->extended_value) {
				case ZEND_ASSIGN_OBJ:
					if (Z_OBJ_HT_P(object)->read_property) {
					#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
						z = Z_OBJ_HT_P(object)->read_property(object, property, BP_VAR_R TSRMLS_CC);
					#else
						z = Z_OBJ_HT_P(object)->read_property(object, property, BP_VAR_R, ((CSAS_OP2_TYPE(opline) == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
					#endif
					}
					break;
				case ZEND_ASSIGN_DIM:
					if (Z_OBJ_HT_P(object)->read_dimension) {
						z = Z_OBJ_HT_P(object)->read_dimension(object, property, BP_VAR_R TSRMLS_CC);
					}
					break;
			}
			if (z) {
				if (Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get) {
					zval *value = Z_OBJ_HT_P(z)->get(z TSRMLS_CC);

					if (Z_REFCOUNT_P(z) == 0) {
						zval_dtor(z);
						FREE_ZVAL(z);
					}
					z = value;
				}
				Z_ADDREF_P(z);
				if ((z && IS_STRING == Z_TYPE_P(z) && Z_STRLEN_P(z) && PHP_CSAS_POSSIBLE(z)) 
					|| (value && IS_STRING == Z_TYPE_P(value) && Z_STRLEN_P(value) && PHP_CSAS_POSSIBLE(value))) {
					csased = 1;
				}
				
				SEPARATE_ZVAL_IF_NOT_REF(&z);
				binary_op(z, z, value TSRMLS_CC);
				if (csased && IS_STRING == Z_TYPE_P(z) && Z_STRLEN_P(z)) {
					Z_STRVAL_P(z) = erealloc(Z_STRVAL_P(z), Z_STRLEN_P(z) + 1 + PHP_CSAS_MAGIC_LENGTH);
					PHP_CSAS_MARK(z, PHP_CSAS_MAGIC_POSSIBLE);
				}
				
				switch (opline->extended_value) {
					case ZEND_ASSIGN_OBJ:
					#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
						Z_OBJ_HT_P(object)->write_property(object, property, z TSRMLS_CC);
					#else
						Z_OBJ_HT_P(object)->write_property(object, property, z, ((CSAS_OP2_TYPE(opline) == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
					#endif
						break;
					case ZEND_ASSIGN_DIM:
						Z_OBJ_HT_P(object)->write_dimension(object, property, z TSRMLS_CC);
						break;
				}
				if (CSAS_RETURN_VALUE_USED(opline)) {
					*retval = z;
					Z_ADDREF_P(*retval);
				}
				zval_ptr_dtor(&z);
			} else {
				zend_error(E_WARNING, "Attempt to assign property of non-object");
				if (CSAS_RETURN_VALUE_USED(opline)) {
					*retval = EG(uninitialized_zval_ptr);
					Z_ADDREF_P(*retval);
				}
			}
		}

		switch(CSAS_OP2_TYPE(opline)) {
			case IS_TMP_VAR:
				zval_ptr_dtor(&property);
				break;
			case IS_VAR:
				if (free_op2.var) {zval_ptr_dtor(&free_op2.var);};
				break;
			case IS_CV:
			case IS_CONST:
			case IS_UNUSED:
			default:
				/* do nothing */
				break;
		}
		
		CSAS_FREE_OP(free_op_data1);
	}

	if (IS_VAR == CSAS_OP1_TYPE(opline) && free_op1.var) {zval_ptr_dtor(&free_op1.var);};
	/* assign_obj has two opcodes! */
	execute_data->opline++;
	execute_data->opline++;
	return ZEND_USER_OPCODE_CONTINUE; 
} /* }}} */ 

static int php_csas_binary_assign_op_helper(int (*binary_op)(zval *result, zval *op1, zval *op2 TSRMLS_DC), ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
	zend_op *opline = execute_data->opline;
	csas_free_op free_op1 = {0}, free_op2 = {0}, free_op_data2 = {0}, free_op_data1 = {0};
	zval **var_ptr = NULL, **object_ptr = NULL, *value = NULL;
	zend_bool increment_opline = 0;
	uint csased = 0;

	switch (opline->extended_value) {
		case ZEND_ASSIGN_OBJ:
			return php_csas_binary_assign_op_obj_helper(binary_op, ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
			break;
		case ZEND_ASSIGN_DIM: {
			switch (CSAS_OP1_TYPE(opline)) {
				case IS_VAR:
					#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
					object_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
					#else
					object_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
					#endif
					if (object_ptr && !(free_op1.var != NULL)) {
						Z_ADDREF_P(*object_ptr);  /* undo the effect of get_obj_zval_ptr_ptr() */
					}
					break;
				case IS_CV:
				#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
					object_ptr = php_csas_get_zval_ptr_ptr_cv(&opline->op1, execute_data->Ts, BP_VAR_W TSRMLS_CC);
				#else
					object_ptr = php_csas_get_zval_ptr_ptr_cv(opline->op1.var, BP_VAR_W TSRMLS_CC);
				#endif
					break;
				case IS_UNUSED:
					object_ptr = php_csas_get_obj_zval_ptr_ptr_unused(TSRMLS_C);
					if (object_ptr) {
						Z_ADDREF_P(*object_ptr);  /* undo the effect of get_obj_zval_ptr_ptr() */
					}
					break;
				default:
					/* do nothing */
					break;
			}
			
			if (object_ptr && Z_TYPE_PP(object_ptr) == IS_OBJECT) {
				return php_csas_binary_assign_op_obj_helper(binary_op, ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
			} else {
				zend_op *op_data = opline+1;

				zval *dim;

				switch(CSAS_OP2_TYPE(opline)) {
					case IS_TMP_VAR:
						#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
						dim = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
						#else
						dim = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
						#endif
						break;
					case IS_VAR:
						#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
						dim = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
						#else
						dim = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
						#endif
						break;
					case IS_CV:
						dim = php_csas_get_zval_ptr_cv(CSAS_OP2_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
						break;
					case IS_CONST:
						dim = CSAS_OP2_CONSTANT_PTR(opline);
						break;
					case IS_UNUSED:
						dim = NULL;
						break;
					default:
						/* do nothing */
						break;
				}
				
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				if (CSAS_OP2_TYPE(opline) == IS_TMP_VAR) {
					php_csas_fetch_dimension_address(&CSAS_T(CSAS_OP2_VAR(op_data)), object_ptr, dim, 1, BP_VAR_RW TSRMLS_CC);
				} else {
					php_csas_fetch_dimension_address(&CSAS_T(CSAS_OP2_VAR(op_data)), object_ptr, dim, 0, BP_VAR_RW TSRMLS_CC);
				}
				value = php_csas_get_zval_ptr(&op_data->op1, execute_data->Ts, &free_op_data1, BP_VAR_R TSRMLS_CC);
				var_ptr = php_csas_get_zval_ptr_ptr(&op_data->op2, execute_data->Ts, &free_op_data2, BP_VAR_RW TSRMLS_CC);
			#else
				#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
				php_csas_fetch_dimension_address(&CSAS_T((opline+1)->op2.var), object_ptr, dim, IS_TMP_VAR, BP_VAR_RW TSRMLS_CC);
				value = php_csas_get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data, &free_op_data1, BP_VAR_R TSRMLS_CC);
				var_ptr = php_csas_get_zval_ptr_ptr_var((opline+1)->op2.var, execute_data, &free_op_data2 TSRMLS_CC);
				#else
				php_csas_fetch_dimension_address(&CSAS_T(CSAS_OP2_VAR(op_data)), object_ptr, dim, CSAS_OP2_TYPE(opline), BP_VAR_RW TSRMLS_CC);
				value = php_csas_get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data->Ts, &free_op_data1, BP_VAR_R TSRMLS_CC);
				var_ptr = php_csas_get_zval_ptr_ptr_var((opline+1)->op2.var, execute_data->Ts, &free_op_data2 TSRMLS_CC);
				#endif
			#endif
				increment_opline = 1;
			}
		}
		break;
	default:
		switch(CSAS_OP2_TYPE(opline)) {
			case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
				value = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
				value = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
				break;
			case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
				value = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
				value = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
				break;
			case IS_CV:
				value = php_csas_get_zval_ptr_cv(CSAS_OP2_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
				break;
			case IS_CONST:
				value = CSAS_OP2_CONSTANT_PTR(opline);
				break;
			case IS_UNUSED:
				value = NULL;
				break;
			default:
				/* do nothing */
				break;
		}

		switch (CSAS_OP1_TYPE(opline)) {
			case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
				var_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
				var_ptr = php_csas_get_zval_ptr_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
				break;
			case IS_CV:
			#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
				var_ptr = php_csas_get_zval_ptr_ptr_cv(&opline->op1, execute_data->Ts, BP_VAR_RW TSRMLS_CC);
			#else
				var_ptr = php_csas_get_zval_ptr_ptr_cv(opline->op1.var, BP_VAR_RW TSRMLS_CC);
			#endif
				break;
			case IS_UNUSED:
				var_ptr = NULL;
				break;
			default:
				/* do nothing */
				break;
		}
		/* do nothing */
		break;
	}

	if (!var_ptr) {
		zend_error(E_ERROR, "Cannot use assign-op operators with overloaded objects nor string offsets");
		return 0;
	}

	if (*var_ptr == EG(error_zval_ptr)) {
		if (CSAS_RETURN_VALUE_USED(opline)) {
			CSAS_T(CSAS_RESULT_VAR(opline)).var.ptr_ptr = &EG(uninitialized_zval_ptr);
			Z_ADDREF_P(*CSAS_T(CSAS_RESULT_VAR(opline)).var.ptr_ptr);
			CSAS_AI_USE_PTR(CSAS_T(CSAS_RESULT_VAR(opline)).var);
		}
		
		switch(CSAS_OP2_TYPE(opline)) {
			case IS_TMP_VAR:
				zval_dtor(free_op2.var);
				break;
			case IS_VAR:
				if (free_op2.var) {zval_ptr_dtor(&free_op2.var);};
				break;
			case IS_CV:
			case IS_CONST:
			case IS_UNUSED:
			default:
				/* do nothing */
				break;
		}
		
		if (IS_VAR == CSAS_OP1_TYPE(opline) && free_op1.var) {zval_ptr_dtor(&free_op1.var);};
		if (increment_opline) {
			execute_data->opline++;
		}
		execute_data->opline++;
	}

	if ((*var_ptr && IS_STRING == Z_TYPE_PP(var_ptr) && Z_STRLEN_PP(var_ptr) && PHP_CSAS_POSSIBLE(*var_ptr))
		|| (value && IS_STRING == Z_TYPE_P(value) && Z_STRLEN_P(value) && PHP_CSAS_POSSIBLE(value))) {
		csased = 1;
	}
	
	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if(Z_TYPE_PP(var_ptr) == IS_OBJECT && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *objval = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(objval);
		if ((objval && IS_STRING == Z_TYPE_P(objval) && Z_STRLEN_P(objval) && PHP_CSAS_POSSIBLE(objval))
			|| (value && IS_STRING == Z_TYPE_P(value) && Z_STRLEN_P(value) && PHP_CSAS_POSSIBLE(value))) {
			csased = 1;
		}
		binary_op(objval, objval, value TSRMLS_CC);
		if (csased && IS_STRING == Z_TYPE_P(objval) && Z_STRLEN_P(objval)) {
			Z_STRVAL_P(objval) = erealloc(Z_STRVAL_P(objval), Z_STRLEN_P(objval) + 1 + PHP_CSAS_MAGIC_LENGTH);
			PHP_CSAS_MARK(objval, PHP_CSAS_MAGIC_POSSIBLE);
		}
		
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, objval TSRMLS_CC);
		zval_ptr_dtor(&objval);
	} else {
		binary_op(*var_ptr, *var_ptr, value TSRMLS_CC);
		if (csased && IS_STRING == Z_TYPE_PP(var_ptr) && Z_STRLEN_PP(var_ptr)) {
			Z_STRVAL_PP(var_ptr) = erealloc(Z_STRVAL_PP(var_ptr), Z_STRLEN_PP(var_ptr) + 1 + PHP_CSAS_MAGIC_LENGTH);
			PHP_CSAS_MARK(*var_ptr, PHP_CSAS_MAGIC_POSSIBLE);
		}
	}

	if (CSAS_RETURN_VALUE_USED(opline)) {
		CSAS_T(CSAS_RESULT_VAR(opline)).var.ptr_ptr = var_ptr;
		Z_ADDREF_P(*var_ptr);
		CSAS_AI_USE_PTR(CSAS_T(CSAS_RESULT_VAR(opline)).var);
	}

	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
			zval_dtor(free_op2.var);
			break;
		case IS_VAR:
			if (free_op2.var) {zval_ptr_dtor(&free_op2.var);};
			break;
		case IS_CV:
		case IS_CONST:
		case IS_UNUSED:
		default:
			/* do nothing */
			break;
	}
	
	if (increment_opline) {
		execute_data->opline++;
		CSAS_FREE_OP(free_op_data1);
		CSAS_FREE_OP_VAR_PTR(free_op_data2);
	}
	if (IS_VAR == CSAS_OP1_TYPE(opline) && free_op1.var) {zval_ptr_dtor(&free_op1.var);};
	
	execute_data->opline++;
	return ZEND_USER_OPCODE_CONTINUE; 
} /* }}} */

static int php_csas_assign_concat_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    return php_csas_binary_assign_op_helper(concat_function, ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
} /* }}} */

static int php_csas_add_string_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL, *result;
	csas_free_op free_op1 = {0};
	uint csased = 0;

	result = &CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	op1 = result;
	if (CSAS_OP1_TYPE(opline) == IS_UNUSED) {
		/* Initialize for erealloc in add_string_to_string */
		Z_STRVAL_P(op1) = NULL;
		Z_STRLEN_P(op1) = 0;
		Z_TYPE_P(op1) = IS_STRING;
		INIT_PZVAL(op1);
	} else {
#endif
	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			op1 = php_csas_get_zval_ptr_cv(CSAS_OP1_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	}
#endif

	if ((op1 && IS_STRING == Z_TYPE_P(op1) &&
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
		Z_STRVAL_P(op1) &&
#endif
		PHP_CSAS_POSSIBLE(op1))) {
		csased = 1;
	}

	add_string_to_string(result, op1, CSAS_OP2_CONSTANT_PTR(opline));

	if (csased && IS_STRING == Z_TYPE_P(result)) {
		Z_STRVAL_P(result) = erealloc(Z_STRVAL_P(result), Z_STRLEN_P(result) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(result, PHP_CSAS_MAGIC_POSSIBLE);
	}

	/* FREE_OP is missing intentionally here - we're always working on the same temporary variable */
	execute_data->opline++;

	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static int php_csas_add_char_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL, *result;
	csas_free_op free_op1 = {0};
	uint csased = 0;

	result = &CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	op1 = result;
	if (CSAS_OP1_TYPE(opline) == IS_UNUSED) {
		/* Initialize for erealloc in add_string_to_string */
		Z_STRVAL_P(op1) = NULL;
		Z_STRLEN_P(op1) = 0;
		Z_TYPE_P(op1) = IS_STRING;
		INIT_PZVAL(op1);
	} else {
#endif
	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			op1 = php_csas_get_zval_ptr_cv(CSAS_OP1_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	}
#endif

	if ((op1 && IS_STRING == Z_TYPE_P(op1)
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
				&& Z_STRVAL_P(op1)
#endif
				&& PHP_CSAS_POSSIBLE(op1))) {
		csased = 1;
	}

	add_char_to_string(result, op1, CSAS_OP2_CONSTANT_PTR(opline));

	if (csased && IS_STRING == Z_TYPE_P(result)) {
		Z_STRVAL_P(result) = erealloc(Z_STRVAL_P(result), Z_STRLEN_P(result) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(result, PHP_CSAS_MAGIC_POSSIBLE);
	}

	/* FREE_OP is missing intentionally here - we're always working on the same temporary variable */
	execute_data->opline++;

	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static int php_csas_add_var_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *op1 = NULL, *op2 = NULL, *result;
	csas_free_op free_op1 = {0}, free_op2 = {0};
	uint csased = 0;
	zval var_copy;
	int use_copy = 0;

	result = &CSAS_T(CSAS_RESULT_VAR(opline)).tmp_var;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	op1 = result;
	if (CSAS_OP1_TYPE(opline) == IS_UNUSED) {
		/* Initialize for erealloc in add_string_to_string */
		Z_STRVAL_P(op1) = NULL;
		Z_STRLEN_P(op1) = 0;
		Z_TYPE_P(op1) = IS_STRING;
		INIT_PZVAL(op1);
	} else {
#endif
	switch(CSAS_OP1_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_tmp(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data, &free_op1 TSRMLS_CC);
#else
			op1 = php_csas_get_zval_ptr_var(CSAS_OP1_NODE_PTR(opline), execute_data->Ts, &free_op1 TSRMLS_CC);
#endif

			break;
		case IS_CV:
			op1 = php_csas_get_zval_ptr_cv(CSAS_OP1_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op1 = CSAS_OP1_CONSTANT_PTR(opline);
			break;
	}
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	}
#endif

	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op2 = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			op2 = php_csas_get_zval_ptr_tmp(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_VAR:
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
			op2 = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data, &free_op2 TSRMLS_CC);
#else
			op2 = php_csas_get_zval_ptr_var(CSAS_OP2_NODE_PTR(opline), execute_data->Ts, &free_op2 TSRMLS_CC);
#endif
			break;
		case IS_CV:
			op2 = php_csas_get_zval_ptr_cv(CSAS_OP2_NODE_PTR(opline), CSAS_GET_ZVAL_PTR_CV_2ND_ARG(BP_VAR_R) TSRMLS_CC);
			break;
		case IS_CONST:
	 		op2 = CSAS_OP2_CONSTANT_PTR(opline);
			break;
	}

	if ((op1 && IS_STRING == Z_TYPE_P(op1)
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
				&& Z_STRVAL_P(op1)
#endif
				&& PHP_CSAS_POSSIBLE(op1))
			|| (op2 && IS_STRING == Z_TYPE_P(op2)
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
				&& Z_STRVAL_P(op2)
#endif
				&& PHP_CSAS_POSSIBLE(op2))) {
		csased = 1;
	}

	if (Z_TYPE_P(op2) != IS_STRING) {
		zend_make_printable_zval(op2, &var_copy, &use_copy);
		if (use_copy) {
			op2 = &var_copy;
		}
	}

	add_string_to_string(result, op1, op2);

	if (use_copy) {
		zval_dtor(op2);
	}

	if (csased && IS_STRING == Z_TYPE_P(result)) {
		Z_STRVAL_P(result) = erealloc(Z_STRVAL_P(result), Z_STRLEN_P(result) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(result, PHP_CSAS_MAGIC_POSSIBLE);
	}

	/* original comment, possibly problematic:
	 * FREE_OP is missing intentionally here - we're always working on the same temporary variable
	 * (Zeev):  I don't think it's problematic, we only use variables
	 * which aren't affected by FREE_OP(Ts, )'s anyway, unless they're
	 * string offsets or overloaded objects
	 */
	switch(CSAS_OP2_TYPE(opline)) {
		case IS_TMP_VAR:
			zval_dtor(free_op2.var);
			break;
		case IS_VAR:
			if (free_op2.var) {
				zval_ptr_dtor(&free_op2.var);
			}
			break;
	}

	execute_data->opline++;

	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static void php_csas_mcall_check(ZEND_OPCODE_HANDLER_ARGS, zend_op *opline, zend_class_entry *scope, char *fname, int len) /* {{{ */ {
	if (scope && fname) {
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
		void **p = EG(argument_stack)->top;
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	    void **p = EG(argument_stack)->top;
#else
		void **p = EG(argument_stack).top_element;
#endif
		int arg_count = opline->extended_value;
		char *class_name = (char *)scope->name;
		uint cname_len = scope->name_length;

		if (!arg_count) {
			return;
		}

		do {
			if (strncmp("mysqli", class_name, cname_len) == 0) {
				if (strncmp("query", fname, len) == 0) {
					zval *el;
					el = *((zval **) (p - arg_count));
					if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
						php_csas_error(NULL TSRMLS_CC, "SQL statement contains data that might be csased");
					}
				}
#if 0
			   	else if (strncmp("escape_string", fname, len) == 0
						|| strncmp("real_escape_string", fname, len) == 0 ) {
					zval *el;
					el = *((zval **) (p - (arg_count)));
					if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
						PHP_CSAS_MARK(el, PHP_CSAS_MAGIC_NONE);
					}
				}
#endif
				break;
			}

			if (strncmp("sqlitedatabase", class_name, cname_len) == 0) {
				if (strncmp("query", fname, len) == 0
						|| strncmp("singlequery", fname, len) == 0) {
					zval *el;
					el = *((zval **) (p - arg_count));
					if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
						php_csas_error(NULL TSRMLS_CC, "SQL statement contains data that might be csased");
					}
				}
				break;
			}

			if (strncmp("PDO", class_name, cname_len) == 0) {
				if (strncmp("query", fname, len) == 0
						|| strncmp("prepare", fname, len) == 0) {
					zval *el;
					el = *((zval **) (p - arg_count));
					if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
						php_csas_error(NULL TSRMLS_CC, "SQL statement contains data that might be csased");
					}
				}
#if 0
			   	else if (strncmp("quote", fname, len) == 0) {
					zval *el;
					el = *((zval **) (p - (arg_count)));
					if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
						PHP_CSAS_MARK(el, PHP_CSAS_MAGIC_NONE);
					}
				}
#endif
				break;
			}
		} while (0);
	}
} /* }}} */

static void php_csas_fcall_check(ZEND_OPCODE_HANDLER_ARGS, zend_op *opline, char *fname, int len) /* {{{ */ {
	if (fname) {
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
		void **p = EG(argument_stack)->top;
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
	    void **p = EG(argument_stack)->top;
#else
		void **p = EG(argument_stack).top_element;
#endif
		int arg_count = opline->extended_value;

		if (!arg_count) {
			return;
		}

		do {
			if (strncmp("print_r", fname, len) == 0
					|| strncmp("fopen", fname, len) == 0
					|| strncmp("opendir", fname, len) == 0
					|| strncmp("dirname", fname, len) == 0
					|| strncmp("basename", fname, len) == 0
					|| strncmp("pathinfo", fname, len) == 0
					|| strncmp("file", fname, len) == 0 ) {
				zval *el;
				el = *((zval **) (p - arg_count));
				if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
					php_csas_error(NULL TSRMLS_CC, "First argument contains data that might be csased");
				}
				break;
			}

			if (strncmp("printf", fname, len) == 0) {
				if (arg_count > 1) {
					zval *el;
					uint i;
					for (i=0;i<arg_count;i++) {
						el = *((zval **) (p - (arg_count - i)));
						if (el && IS_STRING == Z_TYPE_P(el) && Z_STRLEN_P(el) && PHP_CSAS_POSSIBLE(el)) {
							php_csas_error(NULL TSRMLS_CC, "%dth argument contains data that might be csased", i + 1);
							break;
						}
					}
				}
				break;
			}

			if (strncmp("vprintf", fname, len) == 0) {
				if (arg_count > 1) {
					HashTable *ht;
					zval **ppzval, *el = *((zval **) ( p - (arg_count - 1)));
					if (!el || IS_ARRAY != Z_TYPE_P(el) || zend_hash_num_elements(Z_ARRVAL_P(el))) {
						break;
					}

					ht = Z_ARRVAL_P(el);
					for(zend_hash_internal_pointer_reset(ht);
							zend_hash_has_more_elements(ht) == SUCCESS;
							zend_hash_move_forward(ht)) {
						if (zend_hash_get_current_data(ht, (void**)&ppzval) == FAILURE) {
							continue;
						}

						if (IS_STRING == Z_TYPE_PP(ppzval) && Z_STRLEN_PP(ppzval) && PHP_CSAS_POSSIBLE(*ppzval)) {
							char *key;
							long idx;
							switch (zend_hash_get_current_key(ht, &key, &idx, 0)) {
								case HASH_KEY_IS_STRING:
									php_csas_error(NULL TSRMLS_CC, "Second argument contains data(index:%s) that might be csased", key);
									break;
								case HASH_KEY_IS_LONG:
									php_csas_error(NULL TSRMLS_CC, "Second argument contains data(index:%ld) that might be csased", idx);
									break;
							}
							break;
						}
					}
				}
				break;
			}

			if (strncmp("file_put_contents", fname, len) == 0
				   || strncmp("fwrite", fname, len) == 0) {
				if (arg_count > 1) {
					zval *fp, *str;

					fp = *((zval **) (p - arg_count));
					str = *((zval **) (p - (arg_count - 1)));

					if (fp && IS_RESOURCE == Z_TYPE_P(fp)) {
						break;
					} else if (fp && IS_STRING == Z_TYPE_P(fp)) {
						if (strncasecmp("php://output", Z_STRVAL_P(fp), Z_STRLEN_P(fp))) {
							break;
						}
					}
					if (str && IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
						php_csas_error(NULL TSRMLS_CC, "Second argument contains data that might be csased");
					}
				}
				break;
			}

			if (strncmp("mysqli_query", fname, len) == 0
					|| strncmp("mysql_prepare", fname, len) == 0
					|| strncmp("mysql_query", fname, len) == 0
					|| strncmp("sqlite_query", fname, len) == 0
					|| strncmp("sqlite_single_query", fname, len) == 0 ) {
				zval *el;
				el = *((zval **) (p - arg_count));
				if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
					php_csas_error(NULL TSRMLS_CC, "SQL statement contains data that might be csased");
				}
				break;
			}

			if (strncmp("oci_parse", fname, len) == 0) {
				if (arg_count > 1) {
					zval *sql = *((zval **) (p - (arg_count - 1)));
					if (sql && IS_STRING == Z_TYPE_P(sql) && PHP_CSAS_POSSIBLE(sql)) {
						php_csas_error(NULL TSRMLS_CC, "SQL statement contains data that might be csased");
					}
				}
				break;
			}

			if (strncmp("passthru", fname, len) == 0
					|| strncmp("system", fname, len) == 0
					|| strncmp("exec", fname, len) == 0
					|| strncmp("shell_exec", fname, len) == 0
					|| strncmp("proc_open", fname, len) == 0 ) {
				zval *el;
				el = *((zval **) (p - arg_count));
				if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
					php_csas_error(NULL TSRMLS_CC, "CMD statement contains data that might be csased");
				}
				break;
			}
#if 0
			if (strncmp("escapeshellcmd", fname, len) == 0
					|| strncmp("htmlspecialchars", fname, len) == 0
					|| strncmp("escapeshellcmd", fname, len) == 0
					|| strncmp("addcslashes", fname, len) == 0
					|| strncmp("addslashes", fname, len) == 0
					|| strncmp("mysqli_escape_string", fname, len) == 0
					|| strncmp("mysql_real_escape_string", fname, len) == 0
					|| strncmp("mysql_escape_string", fname, len) == 0
					|| strncmp("sqlite_escape_string", fname, len) == 0) {
				zval *el;
				el = *((zval **) (p - (arg_count)));
				if (el && IS_STRING == Z_TYPE_P(el) && PHP_CSAS_POSSIBLE(el)) {
					PHP_CSAS_MARK(el, PHP_CSAS_MAGIC_NONE);
				}
				break;
			}
#endif
		} while (0);
	}
} /* }}} */

static int php_csas_do_fcall_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval *fname = CSAS_OP1_CONSTANT_PTR(opline);

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3)
	zend_function *old_func = EG(function_state_ptr)->function;
	if (zend_hash_find(EG(function_table), fname->value.str.val, fname->value.str.len+1, (void **)&EG(function_state_ptr)->function) == SUCCESS) {
		if (EG(function_state_ptr)->function->common.scope) {
			zend_class_entry *scope = EG(function_state_ptr)->function->common.scope;
			php_csas_mcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, scope, Z_STRVAL_P(fname), Z_STRLEN_P(fname));
		} else {
			php_csas_fcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, Z_STRVAL_P(fname), Z_STRLEN_P(fname));
		}
	}
	EG(function_state_ptr)->function = old_func;
#else
	zend_function *old_func = EG(current_execute_data)->function_state.function;
	if (zend_hash_find(EG(function_table), fname->value.str.val, fname->value.str.len+1, (void **)&EG(current_execute_data)->function_state.function) == SUCCESS) {
		if (EG(current_execute_data)->function_state.function->common.scope) {
			zend_class_entry *scope = EG(current_execute_data)->function_state.function->common.scope;
			php_csas_mcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, scope, Z_STRVAL_P(fname), Z_STRLEN_P(fname));
		} else {
			php_csas_fcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, Z_STRVAL_P(fname), Z_STRLEN_P(fname));
		}
	}
	EG(current_execute_data)->function_state.function = old_func;
#endif

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_do_fcall_by_name_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
	zend_class_entry *scope = execute_data->call->fbc->common.scope;
	char *fname = (char *)(execute_data->call->fbc->common.function_name);
#else
	zend_class_entry *scope = execute_data->fbc->common.scope;
	char *fname = (char *)(execute_data->fbc->common.function_name);
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3)
	zend_function *old_func = EG(function_state_ptr)->function;
	EG(function_state_ptr)->function = execute_data->fbc;
	if (scope) {
		php_csas_mcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, scope, fname, strlen(fname));
	} else {
		php_csas_fcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, fname, strlen(fname));
	}
	EG(function_state_ptr)->function = old_func;
#else
	zend_function *old_func = EG(current_execute_data)->function_state.function;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
	EG(current_execute_data)->function_state.function = execute_data->call->fbc;
#else
	EG(current_execute_data)->function_state.function = execute_data->fbc;
#endif

	if (scope) {
		php_csas_mcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, scope, fname, strlen(fname));
	} else {
		php_csas_fcall_check(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU, opline, fname, strlen(fname));
	}
	EG(current_execute_data)->function_state.function = old_func;
#endif

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_assign_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval **op1 = NULL, **op2 = NULL;

	switch (CSAS_OP2_TYPE(opline)) {
		case IS_VAR:
			op2 = CSAS_T(CSAS_OP2_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP2_VAR(opline));
				if (t && *t) {
					op2 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP2_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op2 = t;
					}
				}
			}
			break;
		default:
			return ZEND_USER_OPCODE_DISPATCH;
			break;
	}

	if (!op2 || *op2 == &EG(error_zval) || Z_TYPE_PP(op2) != IS_STRING || !Z_STRLEN_PP(op2) || !PHP_CSAS_POSSIBLE(*op2)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	switch (CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = t;
					}
				}
			}
			break;
	}

	if (op1 && *op1 != &EG(error_zval) && Z_TYPE_PP(op1) != IS_OBJECT 
			&& PZVAL_IS_REF(*op1) && IS_TMP_VAR != CSAS_OP2_TYPE(opline)) {
		zval garbage = **op1;
		zend_uint refcount = Z_REFCOUNT_PP(op1);

		**op1 = **op2;
		Z_SET_REFCOUNT_P(*op1, refcount);
		Z_SET_ISREF_PP(op1);
		zval_copy_ctor(*op1);
		zval_dtor(&garbage);
		Z_STRVAL_PP(op1) = erealloc(Z_STRVAL_PP(op1), Z_STRLEN_PP(op1) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(*op1, PHP_CSAS_MAGIC_POSSIBLE);

		execute_data->opline++;
		return ZEND_USER_OPCODE_CONTINUE;
	} else if (PZVAL_IS_REF(*op2) && Z_REFCOUNT_PP(op2) > 1) {
		SEPARATE_ZVAL(op2);
		Z_STRVAL_PP(op2) = erealloc(Z_STRVAL_PP(op2), Z_STRLEN_PP(op2) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(*op2, PHP_CSAS_MAGIC_POSSIBLE);
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_assign_ref_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval **op1 = NULL, **op2 = NULL;

	if (opline->extended_value == ZEND_RETURNS_FUNCTION && CSAS_OP2_TYPE(opline) == IS_VAR) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	switch (CSAS_OP2_TYPE(opline)) {
		case IS_VAR:
			op2 = CSAS_T(CSAS_OP2_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP2_VAR(opline));
				if (t && *t) {
					op2 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP2_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op2 = t;
					}
				}
			}
			break;
	}

	if (!op2 || *op2 == &EG(error_zval) || IS_STRING != Z_TYPE_PP(op2)
			|| PZVAL_IS_REF(*op2) || !Z_STRLEN_PP(op2) || !PHP_CSAS_POSSIBLE(*op2)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	switch (CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = t;
					}
				}
			}
			break;
	}

	if (op1 && *op1 == &EG(error_zval)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	if (!op1 || *op1 != *op2) {
		SEPARATE_ZVAL(op2);
		/* TODO: free the op2 if it is a var, now ignore the memleak */
		Z_ADDREF_P(*op2);
		Z_SET_ISREF_PP(op2);
		Z_STRVAL_PP(op2) = erealloc(Z_STRVAL_PP(op2), Z_STRLEN_PP(op2) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(*op2, PHP_CSAS_MAGIC_POSSIBLE);
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_csas_send_ref_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval **op1 = NULL;
	csas_free_op free_op1 = {0};
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
	if (execute_data->function_state.function->type == ZEND_INTERNAL_FUNCTION
			&& !ARG_SHOULD_BE_SENT_BY_REF(execute_data->call->fbc, CSAS_OP_LINENUM(opline->op2))) {
		return ZEND_USER_OPCODE_DISPATCH;
	}
#else
	if (execute_data->function_state.function->type == ZEND_INTERNAL_FUNCTION
				&& !ARG_SHOULD_BE_SENT_BY_REF(execute_data->fbc, CSAS_OP_LINENUM(opline->op2))) {
			return ZEND_USER_OPCODE_DISPATCH;
		}
#endif
	switch (CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = t;
					}
				}
			}
			break;
	}

	if (!op1 || *op1 == &EG(error_zval) || *op1 == &EG(uninitialized_zval) || IS_STRING != Z_TYPE_PP(op1) 
			 || PZVAL_IS_REF(*op1) || Z_REFCOUNT_PP(op1) < 2 || !Z_STRLEN_PP(op1) || !PHP_CSAS_POSSIBLE(*op1)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	SEPARATE_ZVAL_TO_MAKE_IS_REF(op1);
	Z_ADDREF_P(*op1);
	Z_STRVAL_PP(op1) = erealloc(Z_STRVAL_PP(op1), Z_STRLEN_PP(op1) + 1 + PHP_CSAS_MAGIC_LENGTH);
	PHP_CSAS_MARK(*op1, PHP_CSAS_MAGIC_POSSIBLE);
	CSAS_ARG_PUSH(*op1);

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			if (free_op1.var) {
				zval_ptr_dtor(&free_op1.var);
			}
			break;
	}

	execute_data->opline++;
	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static int php_csas_send_var_handler(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */ {
    zend_op *opline = execute_data->opline;
	zval **op1 = NULL;
	csas_free_op free_op1 = {0};
	zval *varptr;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 5)
	if ((opline->extended_value == ZEND_DO_FCALL_BY_NAME)
			&& ARG_SHOULD_BE_SENT_BY_REF(execute_data->call->fbc, CSAS_OP_LINENUM(opline->op2))) {
		return php_csas_send_ref_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	}
#else
	if ((opline->extended_value == ZEND_DO_FCALL_BY_NAME)
				&& ARG_SHOULD_BE_SENT_BY_REF(execute_data->fbc, CSAS_OP_LINENUM(opline->op2))) {
			return php_csas_send_ref_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		}
#endif
	switch (CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			op1 = CSAS_T(CSAS_OP1_VAR(opline)).var.ptr_ptr;
			break;
		case IS_CV:
			{
				zval **t = CSAS_CV_OF(CSAS_OP1_VAR(opline));
				if (t && *t) {
					op1 = t;
				} else if (EG(active_symbol_table)) {
					zend_compiled_variable *cv = &CSAS_CV_DEF_OF(CSAS_OP1_VAR(opline));
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS) {
						op1 = t;
					}
				}
			}
			break;
	}

	if (!op1 || *op1 == &EG(error_zval) || *op1 == &EG(uninitialized_zval) || IS_STRING != Z_TYPE_PP(op1) 
			|| !PZVAL_IS_REF(*op1) || Z_REFCOUNT_PP(op1) < 2 || !Z_STRLEN_PP(op1) || !PHP_CSAS_POSSIBLE(*op1)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

	MAKE_STD_ZVAL(varptr);
    *varptr = **op1;
	Z_SET_REFCOUNT_P(varptr, 0);
	zval_copy_ctor(varptr);
	Z_STRVAL_P(varptr) = erealloc(Z_STRVAL_P(varptr), Z_STRLEN_P(varptr) + 1 + PHP_CSAS_MAGIC_LENGTH);
	PHP_CSAS_MARK(varptr, PHP_CSAS_MAGIC_POSSIBLE);

	Z_ADDREF_P(varptr);
	CSAS_ARG_PUSH(varptr);

	switch(CSAS_OP1_TYPE(opline)) {
		case IS_VAR:
			if (free_op1.var) {
				zval_ptr_dtor(&free_op1.var);
			}
			break;
	}

	execute_data->opline++;
	return ZEND_USER_OPCODE_CONTINUE;
} /* }}} */

static void php_csas_register_handlers(TSRMLS_D) /* {{{ */ {
	zend_set_user_opcode_handler(ZEND_ECHO, php_csas_echo_handler);
	zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, php_csas_include_or_eval_handler);
	zend_set_user_opcode_handler(ZEND_PRINT, php_csas_echo_handler);
	zend_set_user_opcode_handler(ZEND_CONCAT, php_csas_concat_handler);
	zend_set_user_opcode_handler(ZEND_ASSIGN_CONCAT, php_csas_assign_concat_handler);
	zend_set_user_opcode_handler(ZEND_ADD_CHAR, php_csas_add_char_handler);
	zend_set_user_opcode_handler(ZEND_ADD_STRING, php_csas_add_string_handler);
	zend_set_user_opcode_handler(ZEND_ADD_VAR, php_csas_add_var_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL, php_csas_do_fcall_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, php_csas_do_fcall_by_name_handler);
	zend_set_user_opcode_handler(ZEND_ASSIGN_REF, php_csas_assign_ref_handler);
	zend_set_user_opcode_handler(ZEND_ASSIGN, php_csas_assign_handler);
	zend_set_user_opcode_handler(ZEND_SEND_VAR, php_csas_send_var_handler);
    zend_set_user_opcode_handler(ZEND_SEND_REF, php_csas_send_ref_handler);
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
	zend_set_user_opcode_handler(ZEND_QM_ASSIGN, php_csas_qm_assign_handler);
#endif
} /* }}} */

static void php_csas_override_func(char *name, uint len, php_func handler, php_func *stash TSRMLS_DC) /* {{{ */ {
	zend_function *func;
	if (zend_hash_find(CG(function_table), name, len, (void **)&func) == SUCCESS) {
		if (stash) {
			*stash = func->internal_function.handler;
		}
		func->internal_function.handler = handler;
	}
} /* }}} */

static void php_csas_override_functions(TSRMLS_D) /* {{{ */ {
	char f_join[]        = "join";
	char f_trim[]        = "trim";
	char f_split[]       = "split";
	char f_rtrim[]       = "rtrim";
	char f_ltrim[]       = "ltrim";
	char f_strval[]      = "strval";
	char f_strstr[]      = "strstr";
	char f_substr[]      = "substr";
	char f_sprintf[]     = "sprintf";
	char f_explode[]     = "explode";
	char f_implode[]     = "implode";
	char f_str_pad[]     = "str_pad";
	char f_vsprintf[]    = "vsprintf";
	char f_str_replace[] = "str_replace";
	char f_strtolower[] = "strtolower";
	char f_strtoupper[] = "strtoupper";

	php_csas_override_func(f_strval, sizeof(f_strval), PHP_FN(csas_strval), &CSAS_O_FUNC(strval) TSRMLS_CC);
	php_csas_override_func(f_sprintf, sizeof(f_sprintf), PHP_FN(csas_sprintf), &CSAS_O_FUNC(sprintf) TSRMLS_CC);
	php_csas_override_func(f_vsprintf, sizeof(f_vsprintf), PHP_FN(csas_vsprintf), &CSAS_O_FUNC(vsprintf) TSRMLS_CC);
	php_csas_override_func(f_explode, sizeof(f_explode), PHP_FN(csas_explode), &CSAS_O_FUNC(explode) TSRMLS_CC);
	php_csas_override_func(f_split, sizeof(f_split), PHP_FN(csas_explode), NULL TSRMLS_CC);
	php_csas_override_func(f_implode, sizeof(f_implode), PHP_FN(csas_implode), &CSAS_O_FUNC(implode) TSRMLS_CC);
	php_csas_override_func(f_join, sizeof(f_join), PHP_FN(csas_implode), NULL TSRMLS_CC);
	php_csas_override_func(f_trim, sizeof(f_trim), PHP_FN(csas_trim), &CSAS_O_FUNC(trim) TSRMLS_CC);
	php_csas_override_func(f_rtrim, sizeof(f_rtrim), PHP_FN(csas_rtrim), &CSAS_O_FUNC(rtrim) TSRMLS_CC);
	php_csas_override_func(f_ltrim, sizeof(f_ltrim), PHP_FN(csas_ltrim), &CSAS_O_FUNC(ltrim) TSRMLS_CC);
	php_csas_override_func(f_str_replace, sizeof(f_str_replace), PHP_FN(csas_str_replace), &CSAS_O_FUNC(str_replace) TSRMLS_CC);
	php_csas_override_func(f_str_pad, sizeof(f_str_pad), PHP_FN(csas_str_pad), &CSAS_O_FUNC(str_pad) TSRMLS_CC);
	php_csas_override_func(f_strstr, sizeof(f_strstr), PHP_FN(csas_strstr), &CSAS_O_FUNC(strstr) TSRMLS_CC);
	php_csas_override_func(f_strtolower, sizeof(f_strtolower), PHP_FN(csas_strtolower), &CSAS_O_FUNC(strtolower) TSRMLS_CC);
	php_csas_override_func(f_strtoupper, sizeof(f_strtoupper), PHP_FN(csas_strtoupper), &CSAS_O_FUNC(strtoupper) TSRMLS_CC);
	php_csas_override_func(f_substr, sizeof(f_substr), PHP_FN(csas_substr), &CSAS_O_FUNC(substr) TSRMLS_CC);

} /* }}} */

#ifdef COMPILE_DL_CSAS
ZEND_GET_MODULE(csas)
#endif

/* {{{ proto string strval(mixed $value)
 */
PHP_FUNCTION(csas_strval) {
	zval **arg;
	int csased = 0;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (Z_TYPE_PP(arg) == IS_STRING && PHP_CSAS_POSSIBLE(*arg)) {
		csased = 1;
	}

    CSAS_O_FUNC(strval)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string sprintf(string $format, ...)
 */
PHP_FUNCTION(csas_sprintf) {
	zval ***args;
	int i, argc, csased = 0;

	argc = ZEND_NUM_ARGS();

	if (argc < 1) {
		ZVAL_FALSE(return_value);
		WRONG_PARAM_COUNT;
	}

	args = (zval ***)safe_emalloc(argc, sizeof(zval *), 0);
	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		ZVAL_FALSE(return_value);
		WRONG_PARAM_COUNT;
	}

	for (i=0; i<argc; i++) {
		if (args[i] && IS_STRING == Z_TYPE_PP(args[i]) && PHP_CSAS_POSSIBLE(*args[i])) {
			csased = 1;
			break;
		}
	}
	efree(args);

	CSAS_O_FUNC(sprintf)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string vsprintf(string $format, ...)
 */
PHP_FUNCTION(csas_vsprintf) {
	zval *format, *args;
	int argc, csased = 0;

	argc = ZEND_NUM_ARGS();

	if (argc < 1) {
		ZVAL_FALSE(return_value);
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameters(argc TSRMLS_CC, "za", &format, &args) == FAILURE) {
		ZVAL_FALSE(return_value);
		WRONG_PARAM_COUNT;
	}

	do {
		if (IS_STRING == Z_TYPE_P(format) &&  PHP_CSAS_POSSIBLE(format)) {
			csased = 1;
			break;
		}

		if (IS_ARRAY == Z_TYPE_P(args)) {
			HashTable *ht = Z_ARRVAL_P(args);
			zval **ppzval;
			for(zend_hash_internal_pointer_reset(ht);
					zend_hash_has_more_elements(ht) == SUCCESS;
					zend_hash_move_forward(ht)) {
				if (zend_hash_get_current_data(ht, (void**)&ppzval) == FAILURE) {
					continue;
				}
				if (IS_STRING == Z_TYPE_PP(ppzval) && Z_STRLEN_PP(ppzval) && PHP_CSAS_POSSIBLE(*ppzval)) {
					csased = 1;
					break;
				}
			}
			break;
		}
	} while (0);

	CSAS_O_FUNC(vsprintf)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto array explode(string $separator, string $str[, int $limit])
 */
PHP_FUNCTION(csas_explode) {
	zval *separator, *str, *limit;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &separator, &str, &limit) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(explode)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_ARRAY == Z_TYPE_P(return_value) && zend_hash_num_elements(Z_ARRVAL_P(return_value))) {
		php_csas_mark_strings(return_value TSRMLS_CC);
	}
}
/* }}} */

 /* {{{ proto string implode(string $separator, array $args)
 */
PHP_FUNCTION(csas_implode) {
	zval *op1, *op2;
	zval *target = NULL;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &op1, &op2) == FAILURE) {
		ZVAL_FALSE(return_value);
		WRONG_PARAM_COUNT;
	}

	if (IS_ARRAY == Z_TYPE_P(op1)) {
		target = op1;
	} else if(IS_ARRAY == Z_TYPE_P(op2)) {
		target = op2;
	}

	if (target) {
		HashTable *ht = Z_ARRVAL_P(target);
		zval **ppzval;
		for(zend_hash_internal_pointer_reset(ht);
				zend_hash_has_more_elements(ht) == SUCCESS;
				zend_hash_move_forward(ht)) {
			if (zend_hash_get_current_data(ht, (void**)&ppzval) == FAILURE) {
				continue;
			}
			if (IS_STRING == Z_TYPE_PP(ppzval) && Z_STRLEN_PP(ppzval) && PHP_CSAS_POSSIBLE(*ppzval)) {
				csased = 1;
				break;
			}
		}
	}

	CSAS_O_FUNC(implode)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string trim(string $str)
 */
PHP_FUNCTION(csas_trim)
{
	zval *str, *charlist;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &str, &charlist) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(trim)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string rtrim(string $str)
 */
PHP_FUNCTION(csas_rtrim)
{
	zval *str, *charlist;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &str, &charlist) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(rtrim)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string ltrim(string $str)
 */
PHP_FUNCTION(csas_ltrim)
{
	zval *str, *charlist;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &str, &charlist) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(ltrim)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string str_replace(mixed $search, mixed $replace, mixed $subject [, int &$count])
 */
PHP_FUNCTION(csas_str_replace)
{
	zval *str, *from, *len, *repl;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|z", &str, &repl, &from, &len) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(repl) && PHP_CSAS_POSSIBLE(repl)) {
		csased = 1;
	} else if (IS_STRING == Z_TYPE_P(from) && PHP_CSAS_POSSIBLE(from)) {
		csased = 1;
	}

	CSAS_O_FUNC(str_replace)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string str_pad(string $input, int $pad_length[, string $pad_string = " "[, int $pad_type = STR_PAD_RIGHT]])
 */
PHP_FUNCTION(csas_str_pad)
{
	zval *input, *pad_length, *pad_string = NULL, *pad_type = NULL;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|zz", &input, &pad_length, &pad_string, &pad_type) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(input) && PHP_CSAS_POSSIBLE(input)) {
		csased = 1;
	} else if (pad_string && IS_STRING == Z_TYPE_P(pad_string) && PHP_CSAS_POSSIBLE(pad_string)) {
		csased = 1;
	}

	CSAS_O_FUNC(str_pad)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string strstr(string $haystack, mixed $needle[, bool $part = false])
 */
PHP_FUNCTION(csas_strstr)
{
	zval *haystack, *needle, *part;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &haystack, &needle, &part) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(haystack) && PHP_CSAS_POSSIBLE(haystack)) {
		csased = 1;
	}

	CSAS_O_FUNC(strstr)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string substr(string $string, int $start[, int $length])
 */
PHP_FUNCTION(csas_substr)
{
	zval *str;
	long start, length;
    int	csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl|l", &str, &start, &length) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(substr)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string strtolower(string $string)
 */
PHP_FUNCTION(csas_strtolower)
{
	zval *str;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &str) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(strtolower)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

/* {{{ proto string strtoupper(string $string)
 */
PHP_FUNCTION(csas_strtoupper)
{
	zval *str;
	int csased = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &str) == FAILURE) {
		return;
	}
	
	if (IS_STRING == Z_TYPE_P(str) && PHP_CSAS_POSSIBLE(str)) {
		csased = 1;
	}

	CSAS_O_FUNC(strtoupper)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (csased && IS_STRING == Z_TYPE_P(return_value) && Z_STRLEN_P(return_value)) {
		Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value) + 1 + PHP_CSAS_MAGIC_LENGTH);
		PHP_CSAS_MARK(return_value, PHP_CSAS_MAGIC_POSSIBLE);
	}
}
/* }}} */

static PHP_INI_MH(OnUpdateErrorLevel) /* {{{ */ {
	if (!new_value) {
		CSAS_G(error_level) = E_WARNING;
	} else {
		CSAS_G(error_level) = atoi(new_value);
	}
	return SUCCESS;
} /* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("csas.enable", "0", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_csas_globals, csas_globals)
	STD_PHP_INI_ENTRY("csas.error_level", "2", PHP_INI_ALL, OnUpdateErrorLevel, error_level, zend_csas_globals, csas_globals)
PHP_INI_END()
/* }}} */

/* {{{ proto bool csas(string $str[, string ...])
 */
PHP_FUNCTION(csas)
{
	zval ***args;
	int argc;
	int i;

	if (!CSAS_G(enable)) {
		RETURN_TRUE;
	}

	argc = ZEND_NUM_ARGS();
	args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);

	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		return;
	}

	for (i=0; i<argc; i++) {
		if (IS_STRING == Z_TYPE_PP(args[i]) && !PHP_CSAS_POSSIBLE(*args[i])) {
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
			if (IS_INTERNED(Z_STRVAL_PP(args[i]))) {
				efree(args);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%dth arg is internal string", i+1);
				RETURN_FALSE;
			}
#endif
			Z_STRVAL_PP(args[i]) = erealloc(Z_STRVAL_PP(args[i]), Z_STRLEN_PP(args[i]) + 1 + PHP_CSAS_MAGIC_LENGTH);
			PHP_CSAS_MARK(*args[i], PHP_CSAS_MAGIC_POSSIBLE);
		}
	}

	efree(args);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool uncsas(string $str[, string ...])
 */
PHP_FUNCTION(uncsas)
{
	zval ***args;
	int argc;
	int i;

	if (!CSAS_G(enable)) {
		RETURN_TRUE;
	}

	argc = ZEND_NUM_ARGS();
	args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);

	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		return;
	}

	for (i=0; i<argc; i++) {
		if (IS_STRING == Z_TYPE_PP(args[i]) && PHP_CSAS_POSSIBLE(*args[i])) {
			PHP_CSAS_MARK(*args[i], PHP_CSAS_MAGIC_UNCSAS);
		}
	}

	efree(args);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool is_csased(string $str)
 */
PHP_FUNCTION(is_csased)
{
	zval *arg;

	if (!CSAS_G(enable)) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
		return;
	}

	if (IS_STRING == Z_TYPE_P(arg) && PHP_CSAS_POSSIBLE(arg)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(csas)
{
	REGISTER_INI_ENTRIES();

	if (!CSAS_G(enable)) {
		return SUCCESS;
	}

	php_csas_register_handlers(TSRMLS_C);
	php_csas_override_functions(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(csas)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(csas)
{
	if (SG(sapi_started) || !CSAS_G(enable)) {
		return SUCCESS;
	}

    if (PG(http_globals)[TRACK_VARS_POST] && zend_hash_num_elements(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]))) {
		php_csas_mark_strings(PG(http_globals)[TRACK_VARS_POST] TSRMLS_CC);
	}

    if (PG(http_globals)[TRACK_VARS_GET] && zend_hash_num_elements(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]))) {
		php_csas_mark_strings(PG(http_globals)[TRACK_VARS_GET] TSRMLS_CC);
	}

    if (PG(http_globals)[TRACK_VARS_COOKIE] && zend_hash_num_elements(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_COOKIE]))) {
		php_csas_mark_strings(PG(http_globals)[TRACK_VARS_COOKIE] TSRMLS_CC);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(csas)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(csas)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "csas support", "enabled");
	php_info_print_table_row(2, "Version", PHP_CSAS_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
