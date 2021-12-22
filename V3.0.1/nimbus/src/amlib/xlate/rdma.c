/*
-------------------------------------------------------------------------
OBJECT NAME:	rdma.c

FULL NAME:	Translate RDMA houskeeping variables to Nimbus record
		format

ENTRY POINTS:	xlrdma*()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 2001
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

extern char	*ADSrecord;


/* -------------------------------------------------------------------- */
void xlrdmavd(RAWTBL *varp, void *input, NR_TYPE *np)
{
int i;
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->vd);
/*
if ( ((Rdma_blk *)input)->item_type[0] != 'h')
{
printf("%s @ %02d:%02d:%02d\n", ((Rdma_blk *)input)->item_type,
	ntohs(((short *)ADSrecord)[1]), ntohs(((short *)ADSrecord)[2]),
	ntohs(((short *)ADSrecord)[3]));

for (i = 0; i < 200; ++i)
  printf("%.2f ", (NR_TYPE)ntohf(((Rdma_blk *)input)->scan[i]));
printf("\n");
}
*/
}

/* -------------------------------------------------------------------- */
void xlrdmavsh(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->vsh);

}

/* -------------------------------------------------------------------- */
void xlrdmaqa(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->qa);

}

/* -------------------------------------------------------------------- */
void xlrdmaqsh(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->qsh);

}

/* -------------------------------------------------------------------- */
void xlrdmaqs(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->qs);

}

/* -------------------------------------------------------------------- */
void xlrdmaqex(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->qex);

}

/* -------------------------------------------------------------------- */
void xlrdmapabs(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->pabs);

}

/* -------------------------------------------------------------------- */
void xlrdmarh(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->rh);

}

/* -------------------------------------------------------------------- */
void xlrdmatemp(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->temp);

}

/* -------------------------------------------------------------------- */
void xlrdmahvps(RAWTBL *varp, void *input, NR_TYPE *np)
{
  *np = (NR_TYPE)ntohf(((Rdma_blk *)input)->hvps);

}

/* END RDMA.C */
