
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "php.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "main/php_output.h"
#include "zend_API.h"
#include "SAPI.h"
#include "php_ini.h"
#include "php_slowphp.h"

#define MICRO_IN_SEC 1000000.00
#define slowphp_DEF 1


/**
copy from slowphp 's source file 
*/
double slowphp_get_utime(void)
{
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tp; 
    long sec = 0L; 
    double msec = 0.0;

    if (gettimeofday((struct timeval *) &tp, NULL) == 0) {
        sec = tp.tv_sec;
        msec = (double) (tp.tv_usec / MICRO_IN_SEC);

        if (msec >= 1.0) {
            msec -= (long) msec;
        }   
        return msec + sec;
    }   
#endif
    return 0;
}


///* If you declare any globals in php_slowphp.h uncomment this:
PHPAPI ZEND_DECLARE_MODULE_GLOBALS(slowphp)
//*/

/* True global resources - no need for thread safety here */
static int le_slowphp;
double start_time=0;
double end_time=0;
/* {{{ slowphp_functions[]
 *
 * Every user visible function must have an entry in slowphp_functions[].
 */
zend_function_entry slowphp_functions[] = {
	PHP_FE(confirm_slowphp_compiled,	NULL)
	PHP_FE(slowphp_get_long_query_time,	NULL)
	PHP_FE(slowphp_get_long_query_log,	NULL)
	PHP_FE(slowphp_get_query_log_probability,	NULL)
	PHP_FE(slowphp_get_query_log_lock_file,	NULL)
	PHP_FE(slowphp_get_magic_line,	NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ slowphp_module_entry
 */
zend_module_entry slowphp_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"slowphp",
	slowphp_functions,
	PHP_MINIT(slowphp),
	PHP_MSHUTDOWN(slowphp),
	PHP_RINIT(slowphp),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(slowphp),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(slowphp),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	PHP_MODULE_GLOBALS(slowphp),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_SLOWPHP
ZEND_GET_MODULE(slowphp)
#endif

/* {{{ PHP_INI
 */

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("slowphp.long_query_time",      "5", PHP_INI_ALL, OnUpdateLong, long_query_time, zend_slowphp_globals, slowphp_globals)
    STD_PHP_INI_ENTRY("slowphp.long_query_log", "/var/log/php_long_query.log", PHP_INI_ALL, OnUpdateString, long_query_log, zend_slowphp_globals, slowphp_globals)
    STD_PHP_INI_ENTRY("slowphp.long_query_log_probability",      "100", PHP_INI_ALL, OnUpdateLong, long_query_log_probability, zend_slowphp_globals, slowphp_globals)
    STD_PHP_INI_ENTRY("slowphp.long_query_lock_file", "/tmp/php_long_query.lock", PHP_INI_ALL, OnUpdateString, long_query_lock_file, zend_slowphp_globals, slowphp_globals)
    STD_PHP_INI_ENTRY("slowphp.magic_line", "", PHP_INI_ALL, OnUpdateString, magic_line, zend_slowphp_globals, slowphp_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_slowphp_init_globals
 */
//* Uncomment this function if you have INI entries
static void php_slowphp_init_globals(zend_slowphp_globals *slowphp_globals)
{
	slowphp_globals->long_query_time= 0;
	slowphp_globals->magic_line = NULL;
	slowphp_globals->long_query_log = NULL;
}
//*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(slowphp)
{
	//* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	//*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(slowphp)
{
	//* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	//*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(slowphp)
{
	start_time=slowphp_get_utime();
	//运行magic_line
	/**
	char buf[4096];
	sprintf(buf,"include('%s');\n",slowphp_globals.magic_line);
	printf("magic_line:%s\n",buf);
	zend_eval_string(buf,NULL,"error occured when run slowphp.magic_line;" TSRMLS_CC );
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(slowphp)
{
	char line[40960];
	//sprintf(line,"include('%s');\n",slowphp_globals.magic_line);
	sprintf(line,"%s",slowphp_globals.magic_line);
	//printf("magic_line:%s\n",line);
	zend_eval_string(line,NULL,"error occured when run slowphp.magic_line;" TSRMLS_CC );

	char buf[8192];
	end_time=slowphp_get_utime();
	double cost_time=end_time-start_time;
	int log=0;//默认不记录
	//如果满足四项条件中的任一项,就记录:
	//a:long_query_lock_file存在
	//b:cost_time < long_query_time 
	//c:在0和long_query_probability之间产生的随机数刚好是1
	//d:probability 设置的值小于等于1;
	if(slowphp_globals.long_query_log_probability<=1){
		//printf("open log because long_query_log_probability<=1");
		log=1;
	}
	else if(cost_time > slowphp_globals.long_query_time){
		//printf("open log because cost_time > long_query_time,cost_time:%f, long_query_time:%ld\n",cost_time,slowphp_globals.long_query_time);
		log=1;
	}
	else {
		//generate a random seed;
		srand((int)time(0));
		int rand_num=0;
		rand_num=rand();
		int rand_should=0;
		rand_should=(int)(RAND_MAX/slowphp_globals.long_query_log_probability);
		if(rand_num<=rand_should)
		{
			//printf("open log because rand_num<=rand_should:rand_num:%d,rand_should:%d\n",rand_num,rand_should);
			log=1;
		}
		else {
			struct stat info;
			int stat_result;
		    if( (stat_result=stat(slowphp_globals.long_query_lock_file,&info))!=-1 ){
				//printf("open log because exists file:long_query_lock_file:%s\n",slowphp_globals.long_query_lock_file);
				log =1;
			}
		}
	}
	/**
	zval * result_value;
	MAKE_STD_ZVAL( result_value);
	zend_eval_string("print('helo world');",result_value,"inner of slowph.c:197" TSRMLS_CC );
	*/
	if(log==0) return SUCCESS;
	int fd=open( slowphp_globals.long_query_log, O_WRONLY|O_APPEND|O_CREAT,S_IRWXU);
	zval ** script_file_zval;
	zval ** request_uri_zval;
    if ( 
                PG(http_globals)[TRACK_VARS_SERVER]
				 &&
                zend_hash_find(PG(http_globals)[TRACK_VARS_SERVER]->value.ht,  
"SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME"), (void **) &script_file_zval) == SUCCESS
				 &&
                zend_hash_find(PG(http_globals)[TRACK_VARS_SERVER]->value.ht,  
"REQUEST_URI", sizeof("REQUEST_URI"), (void **) &request_uri_zval) == SUCCESS
    ){
		char * script_filename=(zval ** )Z_STRVAL_PP(script_file_zval);
		char * request_uri=(zval ** )Z_STRVAL_PP(request_uri_zval);
		sprintf(buf,"%f\t%f\t%s\t%s\n",start_time,cost_time,script_filename,request_uri);
	}
	else if(
                PG(http_globals)[TRACK_VARS_SERVER] &&
                zend_hash_find(PG(http_globals)[TRACK_VARS_SERVER]->value.ht,  
"SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME"), (void **) &script_file_zval) == SUCCESS
			){
		char * script_filename=(zval ** )Z_STRVAL_PP(script_file_zval);
		sprintf(buf,"%f\t%f\t%s\n",start_time,cost_time,script_filename);
	}
	else {
		sprintf(buf,"%f\t%f\t[can't fetch SCRIPT_FILENAME]\n",start_time,cost_time);
	}



	write(fd,(void * )buf,strlen(buf));
	close(fd);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(slowphp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "slowphp support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */
/* {{{ slowphp_get_long_query_time 
*/
PHP_FUNCTION(slowphp_get_long_query_time)
{
	RETURN_LONG( (long) slowphp_globals.long_query_time);
}/*}}}*/

/* {{{ slowphp_get_long_query_log 
*/
PHP_FUNCTION(slowphp_get_long_query_log)
{
	RETURN_STRINGL( slowphp_globals.long_query_log, strlen(slowphp_globals.long_query_log),1);
}
/*}}}*/

/** {{{ slowphp_get_query_log_probability 
 * return the ini.setting: slowphp.long_query_log_probability
 */
PHP_FUNCTION(slowphp_get_query_log_probability)
{
	RETURN_LONG(slowphp_globals.long_query_log_probability);
}
/*}}}*/

/** {{{ slowphp_get_query_log_lock_file
 * */
PHP_FUNCTION(slowphp_get_query_log_lock_file)
{
	RETURN_STRINGL( slowphp_globals.long_query_lock_file, strlen(slowphp_globals.long_query_lock_file),1);
}
/*}}}*/
/** {{{ slowphp_get_magic_line
 * */
PHP_FUNCTION(slowphp_get_magic_line)
{
	RETURN_STRINGL( slowphp_globals.magic_line, strlen(slowphp_globals.magic_line),1);
}
/*}}}*/

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_slowphp_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_slowphp_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "slowphp", arg);

	RETURN_STRINGL(strg, len, 0);
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
