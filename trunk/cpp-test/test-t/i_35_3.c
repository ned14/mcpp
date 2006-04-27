/* i_35_3.c:    Multi-character wide character constant.    */

/* In ASCII character set.  */
/* 35.3:    */
#if     (L'ab' != L'\x61\x62') || (L'ab' == 'ab')
#error  Bad handling of multi-character wide character constant.
#endif

/* { dg-do preprocess }
   { dg-options -w }    */

