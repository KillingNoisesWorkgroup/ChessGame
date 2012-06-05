#include "encryption.h"

// Does the ecryption and returns encrypted binary data
// FIXME
// Be very carefull with leaks here!!!
encrypted_data_t encrypt(encryption_key_t private_key, binary_data_t raw_data) {
	encrypted_data_t result;
	result.length = raw_data.length;
	result.content = raw_data.content;
	return result;
}

// Does decryption and returns decrypted binary data
// FIXME
// The same thing as in encryption - no leaks please!
binary_data_t decrypt(encryption_key_t public_key, encrypted_data_t encrypted_data) {
	binary_data_t result;
	result.length = encrypted_data.length;
	result.content = encrypted_data.content;
	return result;
}
