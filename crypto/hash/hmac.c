/*
 * hmac.c
 *
 * implementation of hmac auth_type_t
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */
/*
 *	
 * Copyright(c) 2001-2006 Cisco Systems, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * 
 *   Neither the name of the Cisco Systems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <openssl/hmac.h>
#include <openssl/sha.h>

#include "hmac.h" 
#include "alloc.h"

/* the debug module for authentiation */

debug_module_t mod_hmac = {
  0,                  /* debugging is off by default */
  "hmac sha-1"        /* printable name for module   */
};


err_status_t
hmac_alloc(auth_t **a, int key_len, int out_len) {
  extern auth_type_t hmac;
  uint8_t *pointer;

  debug_print(mod_hmac, "allocating auth func with key length %d", key_len);
  debug_print(mod_hmac, "                          tag length %d", out_len);

  /*
   * check key length - note that we don't support keys larger
   * than SHA_DIGEST_LENGTH bytes yet
   */
  if (key_len > SHA_DIGEST_LENGTH)
    return err_status_bad_param;

  /* check output length - should be less than SHA_DIGEST_LENGTH bytes */
  if (out_len > SHA_DIGEST_LENGTH)
    return err_status_bad_param;

  /* allocate memory for auth and HMAC_CTX structures */
  pointer = (uint8_t*)crypto_alloc(sizeof(HMAC_CTX) + sizeof(auth_t));
  if (pointer == NULL)
    return err_status_alloc_fail;

  /* set pointers */
  *a = (auth_t *)pointer;
  (*a)->type = &hmac;
  (*a)->state = pointer + sizeof(auth_t);  
  (*a)->out_len = out_len;
  (*a)->key_len = key_len;
  (*a)->prefix_len = 0;

  /* increment global count of all hmac uses */
  hmac.ref_count++;

  return err_status_ok;
}

err_status_t
hmac_dealloc(auth_t *a) {
  extern auth_type_t hmac;
  
  /* zeroize entire state*/
  octet_string_set_to_zero((uint8_t *)a, 
			   sizeof(HMAC_CTX) + sizeof(auth_t));

  /* free memory */
  crypto_free(a);
  
  /* decrement global count of all hmac uses */
  hmac.ref_count--;

  return err_status_ok;
}

err_status_t
hmac_init(HMAC_CTX *ctx, const uint8_t *key, int key_len) {
  /*
   * check key length - note that we don't support keys larger
   * than SHA_DIGEST_LENGTH bytes yet
   */
  if (key_len > SHA_DIGEST_LENGTH)
    return err_status_bad_param;

  HMAC_Init(ctx, key, key_len, EVP_sha1());

  return err_status_ok;
}

err_status_t
hmac_start(HMAC_CTX *ctx) {
  return err_status_ok;
}

err_status_t
hmac_update(HMAC_CTX *ctx, const uint8_t *message, int msg_octets) {

  debug_print(mod_hmac, "input: %s", 
	      octet_string_hex_string(message, msg_octets));
  
  HMAC_Update(ctx, message, msg_octets);

  return err_status_ok;
}

err_status_t
hmac_compute(HMAC_CTX *ctx, const void *message,
	     int msg_octets, int tag_len, uint8_t *result) {
  unsigned char hash_value[SHA_DIGEST_LENGTH];
  int i;

  /* check tag length, return error if we can't provide the value expected */
  if (tag_len > SHA_DIGEST_LENGTH)
    return err_status_bad_param;
  
  /* hash message, copy output into hash_value */
  HMAC_Update(ctx, message, msg_octets);
  HMAC_Final(ctx, &hash_value, NULL);

  debug_print(mod_hmac, "output: %s",
	      octet_string_hex_string((uint8_t *)hash_value, tag_len));

  /* copy hash_value to *result */
  memcpy(result, &hash_value, tag_len);

  return err_status_ok;
}


/* begin test case 0 */

uint8_t
hmac_test_case_0_key[20] = {
  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 
  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 
  0x0b, 0x0b, 0x0b, 0x0b
};

uint8_t 
hmac_test_case_0_data[8] = {
  0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65   /* "Hi There" */
};

uint8_t
hmac_test_case_0_tag[20] = {
  0xb6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64, 
  0xe2, 0x8b, 0xc0, 0xb6, 0xfb, 0x37, 0x8c, 0x8e, 
  0xf1, 0x46, 0xbe, 0x00
};

auth_test_case_t
hmac_test_case_0 = {
  20,                        /* octets in key            */
  hmac_test_case_0_key,      /* key                      */
  8,                         /* octets in data           */ 
  hmac_test_case_0_data,     /* data                     */
  20,                        /* octets in tag            */
  hmac_test_case_0_tag,      /* tag                      */
  NULL                       /* pointer to next testcase */
};

/* end test case 0 */

char hmac_description[] = "hmac sha-1 authentication function";

/*
 * auth_type_t hmac is the hmac metaobject
 */

auth_type_t
hmac  = {
  (auth_alloc_func)      hmac_alloc,
  (auth_dealloc_func)    hmac_dealloc,
  (auth_init_func)       hmac_init,
  (auth_compute_func)    hmac_compute,
  (auth_update_func)     hmac_update,
  (auth_start_func)      hmac_start,
  (char *)               hmac_description,
  (int)                  0,  /* instance count */
  (auth_test_case_t *)  &hmac_test_case_0,
  (debug_module_t *)    &mod_hmac,
  (auth_type_id_t)       HMAC_SHA1
};
