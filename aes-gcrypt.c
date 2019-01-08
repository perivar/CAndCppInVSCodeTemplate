/*  aes-gcrypt - a sample program for libgcrypt usage
    Copyright (C) 2004  Tobias Bayer

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>

int main()
{
    const int BLOCKSIZE = 16;
    const int TEXTLEN = 25;
    const char* plaintext = "ERLOSCHEN IST LEUCHTTONNE";
    
	//Initialize
	printf("Initializing... Version: %s\n", gcry_check_version(NULL));

	//Symmetric key generation (128 bit)
	printf("Generating symmetric key (128 bit)...\n");
	
	//Allocate a byte-array for holding the key
	unsigned char* key = (unsigned char*) malloc(BLOCKSIZE*sizeof(char)); 
	
	//Use secure random number generation
	gcry_randomize(key, BLOCKSIZE, GCRY_STRONG_RANDOM);
	
	//Print the key
	printf("Key is: ");
	int i;
	for(i=0;i<BLOCKSIZE;i++)
		printf("%X ", key[i]);	
	printf("\n");

	//Symmetric encryption with AES
	gcry_error_t err;
	gcry_cipher_hd_t hd;
	//Initialization vector
	const char* iv = "1234567812345678";	
	//Allocate memory for ciphertext
	unsigned char* ciphertext = (unsigned char*) malloc(TEXTLEN*sizeof(char));

    //Open cipher
	printf("Opening AES cipher...\n");	
	err = gcry_cipher_open(&hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CFB, 0);
	if(err)
	{
		printf("Failed opening AES cipher: %s\n", gpg_strerror(err));
		return 1;
	}

    //Set key for cipher
	printf("Setting the key...\n");
	err = gcry_cipher_setkey(hd, key, BLOCKSIZE);
    if(err)
	{
        printf("Failed setting the key: %s\n", gpg_strerror(err));
        return 1;
    }
	
	//Set iv
	printf("Setting the initialization vector %s...\n", iv);
	err = gcry_cipher_setiv(hd, iv, BLOCKSIZE);
    if(err)
    {
        printf("Failed setting the initialization vector: %s\n", gpg_strerror(err));
        return 1;
    }

    //Encrypt
	printf("Encrypting plaintext: %s...\n", plaintext);
	err = gcry_cipher_encrypt(hd, ciphertext, TEXTLEN, plaintext, TEXTLEN);
    if(err)
    {
        printf("Encryption failed: %s\n", gpg_strerror(err));
        return 1;
    }
	
	printf("Resulting ciphertext (hex): ");
	for(i=0;i<TEXTLEN;i++)
		printf("%X ", ciphertext[i]);
	printf("\n");
	
	//Decrypt again
	printf("Resetting initialization vector...\n");
	err = gcry_cipher_setiv(hd, iv, BLOCKSIZE);        
	if(err)        
	{                
		printf("Failed setting the initialization vector: %s\n", gpg_strerror(err));
        return 1;
    }

	printf("Decrypting...\n");
	//Memory for storing the decryption result
    unsigned char* decryptedtext = (unsigned char*) malloc(TEXTLEN*sizeof(char));
	err = gcry_cipher_decrypt(hd, decryptedtext, TEXTLEN, ciphertext, TEXTLEN);
    if(err)                
    {  
        printf("Decryption failed: %s\n", gpg_strerror(err));
        return 1;
    }
	
	printf("Decrypted text: %s\n", decryptedtext); 
	
	//Cleanup
	gcry_cipher_close(hd);
	free(key);
	free(ciphertext);
	free(decryptedtext);
	
	return 0;
}