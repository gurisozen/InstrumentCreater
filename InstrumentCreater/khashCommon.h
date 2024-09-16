/*
 * khashCommon.h
 *
 *  Created on: Sep 24, 2018
 *      Author: Osman
 */



#define klib_unused __attribute__ ((__unused__))

#define LIB_DICTIONARY_MAX_STRING_SIZE		64

static const double __string__ac_HASH_UPPER = 0.77;
#define __string__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __string__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __string__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __string__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __string__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __string__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __string__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __string__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

static const double __stringmt__ac_HASH_UPPER = 0.77;
#define __stringmt__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __stringmt__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __stringmt__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __stringmt__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __stringmt__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __stringmt__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __stringmt__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __stringmt__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

static const double __int__ac_HASH_UPPER = 0.77;
#define __int__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __int__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __int__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __int__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __int__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __int__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __int__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __int__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

static const double __intmt__ac_HASH_UPPER = 0.77;
#define __intmt__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __intmt__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __intmt__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __intmt__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __intmt__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __intmt__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __intmt__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __intmt__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

static const double __long__ac_HASH_UPPER = 0.77;
#define __long__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __long__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __long__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __long__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __long__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __long__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __long__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __long__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

static const double __longmt__ac_HASH_UPPER = 0.77;
#define __longmt__ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __longmt__ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __longmt__ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __longmt__ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __longmt__ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __longmt__ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __longmt__ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define __longmt__ac_fsize(m) ((m) < 16? 1 : (m)>>4)

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

