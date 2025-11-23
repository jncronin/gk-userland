#include <unistd.h>
#include <string.h>

static char crypt_val[14];

extern "C" char *crypt(const char *key, const char *salt)
{
    /* For now, do no hashing - just needs to be implemented for dropbear support */
    if(!salt)
        return nullptr;
    if(!key)
        return nullptr;

    /* Ensure the return value is initialized */
    memset(crypt_val, ' ', 13);
    crypt_val[13] = 0;

    size_t salt_len = strlen(salt);
    if(salt_len > 2)
        salt_len = 2;
    memcpy(crypt_val, salt, salt_len);

    size_t key_len = strlen(key);
    if(key_len > 11)
        key_len = 11;
    memcpy(&crypt_val[2], key, key_len);

    return crypt_val;
}
