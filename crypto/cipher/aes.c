/*
 * aes.c
 *
 * An implemnetation of the AES block cipher.
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */

/*
 *	
 * Copyright (c) 2001-2006, Cisco Systems, Inc.
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


#include <openssl/aes.h>

#include "aes.h"
#include "err.h"


extern debug_module_t mod_aes_icm;

err_status_t
aes_expand_encryption_key(const uint8_t *key, 
			  int key_len,
			  aes_expanded_key_t *expanded_key) {
  if (key_len == 16) {
    AES_set_encrypt_key(key, 128, expanded_key);
    return err_status_ok;
  }
  else if (key_len == 24) {
    /* AES-192 not yet supported */
    return err_status_bad_param;
  }
  else if (key_len == 32) {
    AES_set_encrypt_key(key, 256, expanded_key);
    return err_status_ok;
  }
  else
    return err_status_bad_param;
}

err_status_t
aes_expand_decryption_key(const uint8_t *key, 
			  int key_len,
			  aes_expanded_key_t *expanded_key) {
  AES_set_decrypt_key(key, key_len * 8, expanded_key);
  return err_status_ok;
}
