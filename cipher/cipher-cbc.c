/* cipher-cbc.c  - Generic CBC mode implementation
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
 *               2005, 2007, 2008, 2009, 2011 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "cipher.h"
#include "ath.h"
#include "./cipher-internal.h"
#include "bufhelp.h"



gcry_err_code_t
_gcry_cipher_cbc_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, unsigned int outbuflen,
                          const unsigned char *inbuf, unsigned int inbuflen)
{
  unsigned int n;
  unsigned char *ivp;
  int i;
  size_t blocksize = c->spec->blocksize;
  unsigned nblocks = inbuflen / blocksize;
  unsigned int burn, nburn;

  if (outbuflen < ((c->flags & GCRY_CIPHER_CBC_MAC)? blocksize : inbuflen))
    return GPG_ERR_BUFFER_TOO_SHORT;

  if ((inbuflen % c->spec->blocksize)
      && !(inbuflen > c->spec->blocksize
           && (c->flags & GCRY_CIPHER_CBC_CTS)))
    return GPG_ERR_INV_LENGTH;

  burn = 0;

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      if ((inbuflen % blocksize) == 0)
	nblocks--;
    }

  if (c->bulk.cbc_enc)
    {
      c->bulk.cbc_enc (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks,
                       (c->flags & GCRY_CIPHER_CBC_MAC));
      inbuf  += nblocks * blocksize;
      if (!(c->flags & GCRY_CIPHER_CBC_MAC))
        outbuf += nblocks * blocksize;
    }
  else
    {
      for (n=0; n < nblocks; n++ )
        {
          buf_xor(outbuf, inbuf, c->u_iv.iv, blocksize);
          nburn = c->spec->encrypt ( &c->context.c, outbuf, outbuf );
          burn = nburn > burn ? nburn : burn;
          memcpy (c->u_iv.iv, outbuf, blocksize );
          inbuf  += blocksize;
          if (!(c->flags & GCRY_CIPHER_CBC_MAC))
            outbuf += blocksize;
        }
    }

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      /* We have to be careful here, since outbuf might be equal to
         inbuf.  */
      int restbytes;
      unsigned char b;

      if ((inbuflen % blocksize) == 0)
        restbytes = blocksize;
      else
        restbytes = inbuflen % blocksize;

      outbuf -= blocksize;
      for (ivp = c->u_iv.iv, i = 0; i < restbytes; i++)
        {
          b = inbuf[i];
          outbuf[blocksize + i] = outbuf[i];
          outbuf[i] = b ^ *ivp++;
        }
      for (; i < blocksize; i++)
        outbuf[i] = 0 ^ *ivp++;

      nburn = c->spec->encrypt (&c->context.c, outbuf, outbuf);
      burn = nburn > burn ? nburn : burn;
      memcpy (c->u_iv.iv, outbuf, blocksize);
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}


gcry_err_code_t
_gcry_cipher_cbc_decrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, unsigned int outbuflen,
                          const unsigned char *inbuf, unsigned int inbuflen)
{
  unsigned int n;
  int i;
  size_t blocksize = c->spec->blocksize;
  unsigned int nblocks = inbuflen / blocksize;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  if ((inbuflen % c->spec->blocksize)
      && !(inbuflen > c->spec->blocksize
           && (c->flags & GCRY_CIPHER_CBC_CTS)))
    return GPG_ERR_INV_LENGTH;

  burn = 0;

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      nblocks--;
      if ((inbuflen % blocksize) == 0)
	nblocks--;
      memcpy (c->lastiv, c->u_iv.iv, blocksize);
    }

  if (c->bulk.cbc_dec)
    {
      c->bulk.cbc_dec (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks);
      inbuf  += nblocks * blocksize;
      outbuf += nblocks * blocksize;
    }
  else
    {
      for (n=0; n < nblocks; n++ )
        {
          /* Because outbuf and inbuf might be the same, we have to
           * save the original ciphertext block.  We use LASTIV for
           * this here because it is not used otherwise. */
          memcpy (c->lastiv, inbuf, blocksize);
          nburn = c->spec->decrypt ( &c->context.c, outbuf, inbuf );
          burn = nburn > burn ? nburn : burn;
          buf_xor(outbuf, outbuf, c->u_iv.iv, blocksize);
          memcpy(c->u_iv.iv, c->lastiv, blocksize );
          inbuf  += c->spec->blocksize;
          outbuf += c->spec->blocksize;
        }
    }

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      int restbytes;

      if ((inbuflen % blocksize) == 0)
        restbytes = blocksize;
      else
        restbytes = inbuflen % blocksize;

      memcpy (c->lastiv, c->u_iv.iv, blocksize );         /* Save Cn-2. */
      memcpy (c->u_iv.iv, inbuf + blocksize, restbytes ); /* Save Cn. */

      nburn = c->spec->decrypt ( &c->context.c, outbuf, inbuf );
      burn = nburn > burn ? nburn : burn;
      buf_xor(outbuf, outbuf, c->u_iv.iv, restbytes);

      memcpy(outbuf + blocksize, outbuf, restbytes);
      for(i=restbytes; i < blocksize; i++)
        c->u_iv.iv[i] = outbuf[i];
      nburn = c->spec->decrypt (&c->context.c, outbuf, c->u_iv.iv);
      burn = nburn > burn ? nburn : burn;
      buf_xor(outbuf, outbuf, c->lastiv, blocksize);
      /* c->lastiv is now really lastlastiv, does this matter? */
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}
