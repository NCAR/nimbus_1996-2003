/*
-------------------------------------------------------------------------
OBJECT NAME:	memalloc.c

FULL NAME:	Memory Allocation

ENTRY POINTS:	AllocateDataArrays()
		FreeDataArrays()

STATIC FNS:	none

DESCRIPTION:	Allocate storage for pointers & data array.  This is
		analogous to shmem.c for winput.c

INPUT:		none

OUTPUT:		none

REFERENCES:	none

REFERENCED BY:	cb_main.c
-------------------------------------------------------------------------
*/

#include "define.h"
#include "decode.h"
#include "ctape.h"

static bool	ArraysInitialized = FALSE;


/* -------------------------------------------------------------------- */
void AllocateDataArrays()
{
  int		i;
  long	lrlen;

  get_lrlen(&lrlen);
  ADSrecord = GetMemory((unsigned)lrlen);

  nFloats = 0;

  for (i = 0; i < nsdi; ++i)
    {
    sdi[i]->SRstart = nFloats;
    nFloats += sdi[i]->SampleRate;
    }

  for (i = 0; i < nraw; ++i)
    {
    raw[i]->SRstart = nFloats;
    nFloats += (raw[i]->SampleRate * raw[i]->Length);
    }


  bits = (NR_TYPE *)GetMemory((unsigned)sizeof(long) * nFloats);
  volts = (NR_TYPE *)GetMemory((unsigned)sizeof(NR_TYPE) * nFloats);
  SampledData = (NR_TYPE *)GetMemory((unsigned)sizeof(NR_TYPE) * nFloats);

  memset((void *)bits, 0, sizeof(long) * nFloats);
  memset((void *)volts, 0, sizeof(NR_TYPE) * nFloats);

  ArraysInitialized = TRUE;

}	/* END ALLOCATEDATAARRAYS */

/* -------------------------------------------------------------------- */
void FreeDataArrays()
{
  if (ArraysInitialized)
    {
    free((char *)ADSrecord);
    free((char *)SampledData);
    }

  ArraysInitialized = FALSE;

}	/* END FREEDATAARRAYS */

/* END MEMALLOC.C */
