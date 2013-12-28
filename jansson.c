/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <jansson.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_jansson.h"

static inline void* jsson_malloc(size_t len) {
	return emalloc(len);
}

static inline void jsson_free(void *block) {
	efree(block);
}

static inline json_t* json_encode_zend(zval **zvalue TSRMLS_DC) {
	json_t *json = NULL;
	HashTable *hash = NULL;
	HashPosition position;
	zval **zivalue;
	
	switch (Z_TYPE_PP(zvalue)) {
		case IS_LONG:
			return json_integer(Z_LVAL_PP(zvalue));
			
		case IS_DOUBLE:
			return json_real(Z_DVAL_PP(zvalue));
			
		case IS_NULL:
			return json_null();
		
		case IS_STRING:
			return json_string(Z_STRVAL_PP(zvalue));
		
		case IS_ARRAY:
			hash = Z_ARRVAL_PP(zvalue);
			
		case IS_OBJECT: {
			char *key = NULL;
			zend_uint klen = 0;
			zend_ulong kdx = 0L;
			
			if (!hash) {
				hash = Z_OBJPROP_PP(zvalue);
			}
			
			json = json_object();
			
			for (zend_hash_internal_pointer_reset_ex(hash, &position);
				zend_hash_get_current_data_ex(hash, (void**)&zivalue, &position) == SUCCESS;
				zend_hash_move_forward_ex(hash, &position)) {
				if (zend_hash_get_current_key_ex(hash, &key, &klen, &kdx, 0, &position) == HASH_KEY_IS_STRING) {
					json_obect_set(json, key, json_encode_zend(zivalue TSRMLS_CC));
				}
			}
		}
	}
	
	return json;
}

static inline zval* json_decode_zend(json_t **json TSRMLS_DC) {
	zval* retval = EG(uninitialized_zval_ptr);
	
	if (json && *json) {
		switch (json_typeof(*json)) {
			case JSON_NULL:
				Z_ADDREF_P(retval);
			break;
			
			case JSON_STRING:
				ALLOC_INIT_ZVAL(retval);
				ZVAL_STRING(retval, json_string_value(*json), 1);

			break;
			
			case JSON_INTEGER:
				ALLOC_INIT_ZVAL(retval);
				ZVAL_LONG(retval, json_integer_value(*json));
			break;
			
			case JSON_REAL:
				ALLOC_INIT_ZVAL(retval);
				ZVAL_DOUBLE(retval, json_real_value(*json));
			break;	
			
			case JSON_TRUE:
			case JSON_FALSE:
				ALLOC_INIT_ZVAL(retval);
				ZVAL_BOOL(retval, json_boolean_value(*json));
			break;
			
			case JSON_OBJECT: {
				const char *jkey;
				json_t *jvalue;
				
				ALLOC_INIT_ZVAL(retval);
				array_init(retval);
				
				json_object_foreach(*json, jkey, jvalue) {
					add_assoc_zval(
						retval, jkey, json_decode_zend(&jvalue TSRMLS_CC));
				}
			} break;
			
			case JSON_ARRAY: {
				size_t jkey;
				json_t *jvalue;
				
				ALLOC_INIT_ZVAL(retval);
				array_init(retval);
				
				json_array_foreach(*json, jkey, jvalue) {
					add_index_zval(
						retval, jkey, json_decode_zend(&jvalue TSRMLS_CC));
				}
			} break;
		
			default:
				zend_error(E_ERROR, "unknown tyep : %d\n", json_typeof(*json));
		}
	} else {
		Z_ADDREF_P(retval);
	}
	
	return retval;
}

/* {{{ proto string jsson_encode(mixed variable)
   Encode the variable in JSON */
PHP_FUNCTION(jsson_encode)
{
	zval *variable;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &variable) == FAILURE) {
		return;
	}

	{
		json_t *json;
		json_error_t error;
		HashTable *hash = NULL;
		HashPosition position;
		zval **zvalue;
		
		switch (Z_TYPE_P(variable)) {
			case IS_NULL:
				json = json_null();
			break;
			
			case IS_LONG:
				json = json_integer(Z_LVAL_P(variable));
			break;
			
			case IS_STRING:
				json = json_string(Z_STRVAL_P(variable));
			break;
			
			case IS_ARRAY:
				hash = Z_ARRVAL_P(variable);
			
			case IS_OBJECT: {
				char *key = NULL;
				zend_uint klen = 0;
				zend_ulong kdx = 0L;
				
				if (!hash) {
					hash = Z_OBJPROP_P(variable);
				}
				
				json = json_object();
				
				for (zend_hash_internal_pointer_reset_ex(hash, &position);
					zend_hash_get_current_data_ex(hash, (void**)&zvalue, &position) == SUCCESS;
					zend_hash_move_forward_ex(hash, &position)) {
					json_t *next = NULL;
					if (zend_hash_get_current_key_ex(
						hash, &key, &klen, &kdx, 0, &position) == HASH_KEY_IS_STRING) {
						next = json_encode_zend(zvalue TSRMLS_CC);
						if (next) {
							json_object_set(json, key, next);
							json_decref(next);
						}
					}
				}
			}	
		}
		
		if (json) {
			ZVAL_STRING(return_value, json_dumps(json, 0), 0);
			json_decref(json);
		}
	}
} /* }}} */

/* {{{ proto mixed jsson_decode(string string)
   Decode the JSON string */
PHP_FUNCTION(jsson_decode)
{
	char *str;
	zend_uint slen;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &slen) == FAILURE) {
		return;
	}

	{
		json_error_t error;
		json_t *json = json_loads(str, 0, &error);
		
		if (json) {
			zval *retval = json_decode_zend(&json TSRMLS_CC);
			if (retval) {
				ZVAL_ZVAL(return_value, retval, 0, 1);
			}
			json_decref(json);
		} else {
			zend_error(E_WARNING, 	
				"JSON parsing error, %s on line %d at offset %d: \"%s\"",
				error.text, error.line, error.column, error.source);
		}
		
	}
} /* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(jansson)
{
	json_set_alloc_funcs(jsson_malloc, jsson_free);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(jansson)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "jansson support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(arginfo_jsson_encode, 0, 0, 1)
	ZEND_ARG_INFO(0, variable)
ZEND_END_ARG_INFO() /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(arginfo_jsson_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
ZEND_END_ARG_INFO() /* }}} */

/* {{{ jansson_functions[]
 */
const zend_function_entry jansson_functions[] = {
	PHP_FE(jsson_encode,	arginfo_jsson_encode)
	PHP_FE(jsson_decode,	arginfo_jsson_decode)
	PHP_FE_END
};
/* }}} */

/* {{{ jansson_module_entry
 */
zend_module_entry jansson_module_entry = {
	STANDARD_MODULE_HEADER,
	"jansson",
	jansson_functions,
	PHP_MINIT(jansson),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(jansson),
	PHP_JANSSON_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_JANSSON
ZEND_GET_MODULE(jansson)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
