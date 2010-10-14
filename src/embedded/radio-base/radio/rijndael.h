#ifndef RIJNDAEL_H
#define RIJNDAEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**

	\file rijndael.h
	\brief 128-bit AES - Advanced Encryption Standard (Rijndael) library

*/

typedef enum {
	AES_KEY_SRAM = 0,
	AES_KEY_FLASH = 1
} key_mem_t;

/**

	\brief Expand AES key to (16*11=176 bytes) of SRAM

	\param[out] buf target SRAM buffer, 176 bytes
	\param[in] key source key pointer
	\param[in] key_mem memory type, AES_KEY_SRAM or AES_KEY_FLASH

*/
void aes_key_expand(uint8_t *buf, const uint8_t *key, key_mem_t key_mem);

/**

	\brief Encrypt a block of data with AES

	Data may be encrypted in-place, i.e. data may equal buf.

	\param[in] expanded_key result of aes_key_expand()
	\param[in] data plaintext to be encrypted, 16 bytes
	\param[out] buf target buffer, 16 bytes
	\param[out] state state buffer / IV (first round), 16 bytes

*/
void aes_encrypt(const uint8_t *expanded_key, const uint8_t *data, uint8_t *buf, uint8_t *state);

/**

	\brief Decrypt a block of data with AES

	The target buffer MUST NOT equal data.

	\param[in] expanded_key result of aes_key_expand()
	\param[in] data ciphertext to be decrypted, 16 bytes
	\param[out] buf target buffer, 16 bytes
	\param[out] state state buffer / IV (first round), 16 bytes

*/
void aes_decrypt(const uint8_t *expanded_key, const uint8_t *data, uint8_t *buf, uint8_t *state);


#ifdef __cplusplus
}
#endif

#endif /* RIJNDAEL_H */

