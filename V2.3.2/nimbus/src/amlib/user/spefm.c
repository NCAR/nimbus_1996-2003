/*
-------------------------------------------------------------------------
OBJECT NAME:	spefm.c

FULL NAME:	Sail Plane Electric Field Mill

ENTRY POINTS:	EFinit()

STATIC FNS:	

DESCRIPTION:	Template for AMLIB std & user directories.

INPUT:		

OUTPUT:		

REFERENCES:	

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


static NR_TYPE	*vft0, *vft1, *vft2, *vfb0, *vfb1, *vfb2,
		*vfp0, *vfp1, *vfp2, *vfs0, *vfs1, *vfs2,
		*vnt0, *vnt1, *vnb0, *vnb1, *vnp0, *vnp1, *vns0, *vns1,
		RFZ, RFY, RNZ, RNY, CFZ, CFY, CNZ, CNY, RXZ, RXY, CXZ, CXY;

/* -------------------------------------------------------------------- */
void EFinit()
{
	vft0 = GetDefaultsValue("VFT0");
	vft1 = GetDefaultsValue("VFT1");
	vft2 = GetDefaultsValue("VFT2");

	vfb0 = GetDefaultsValue("VFB0");
	vfb1 = GetDefaultsValue("VFB1");
	vfb2 = GetDefaultsValue("VFB2");

	vfp0 = GetDefaultsValue("VFP0");
	vfp1 = GetDefaultsValue("VFP1");
	vfp2 = GetDefaultsValue("VFP2");

	vfs0 = GetDefaultsValue("VFS0");
	vfs1 = GetDefaultsValue("VFS1");
	vfs2 = GetDefaultsValue("VFS2");

	vnt0 = GetDefaultsValue("VNT0");
	vnt1 = GetDefaultsValue("VNT1");

	vnb0 = GetDefaultsValue("VNB0");
	vnb1 = GetDefaultsValue("VNB1");

	vnp0 = GetDefaultsValue("VNP0");
	vnp1 = GetDefaultsValue("VNP1");

	vns0 = GetDefaultsValue("VNS0");
	vns1 = GetDefaultsValue("VNS1");

	RFZ = (GetDefaultsValue("RFZ"))[0];
	RFY = (GetDefaultsValue("RFY"))[0];
	RNZ = (GetDefaultsValue("RNZ"))[0];
	RNY = (GetDefaultsValue("RNY"))[0];
	CFZ = (GetDefaultsValue("CFZ"))[0];
	CFY = (GetDefaultsValue("CFY"))[0];
	CNZ = (GetDefaultsValue("CNZ"))[0];
	CNY = (GetDefaultsValue("CNY"))[0];
	RXZ = (GetDefaultsValue("RXZ"))[0];
	RXY = (GetDefaultsValue("RXY"))[0];
	CXZ = (GetDefaultsValue("CXZ"))[0];
	CXY = (GetDefaultsValue("CXY"))[0];

}	/* END EFINIT */

/* -------------------------------------------------------------------- */
void sefeft(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vft, at, eft;
	int	sign;

	vft	= GetSample(varp, 0);
	sign	= (int)GetSample(varp, 1);

	sign = (sign & 0x0040) ? -1 : 1;

	if (vft > 1.43)
		at = vft0[0] + vft0[1] * vft;
	else
	if (vft > 0.093)
		at = vft1[0] + vft1[1] * vft +
		vft1[2] * pow((double)vft, (double)2.0) +
		vft1[3] * pow((double)vft, (double)3.0) +
		vft1[4] * pow((double)vft, (double)4.0) +
		vft1[5] * pow((double)vft, (double)5.0);
	else
		at = vft2[0] + vft2[1] * vft +
		vft2[2] * pow((double)vft, (double)2.0) +
		vft2[3] * pow((double)vft, (double)3.0);

	eft = pow((double)10.0, (double)at) / 1000.0 * sign;

	PutSample(varp, eft);

}	/* END SEFEFT */

/* -------------------------------------------------------------------- */
void sefefb(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vfb, at, efb;
	int	sign;

	vfb	= GetSample(varp, 0);
	sign	= (int)GetSample(varp, 1);

	sign = (sign & 0x0040) ? -1 : 1;

	if (vfb > 1.43)
		at = vfb0[0] + vfb0[1] * vfb;
	else
	if (vfb > 0.093)
		at = vfb1[0] + vfb1[1] * vfb +
		vfb1[2] * pow((double)vfb, (double)2.0) +
		vfb1[3] * pow((double)vfb, (double)3.0) +
		vfb1[4] * pow((double)vfb, (double)4.0) +
		vfb1[5] * pow((double)vfb, (double)5.0);
	else
		at = vfb2[0] + vfb2[1] * vfb +
		vfb2[2] * pow((double)vfb, (double)2.0) +
		vfb2[3] * pow((double)vfb, (double)3.0);

	efb = pow((double)10.0, (double)at) / 1000.0 * sign;

	PutSample(varp, efb);

}	/* END SEFEFB */

/* -------------------------------------------------------------------- */
void sefefp(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vfp, at, efp;
	int	sign;

	vfp	= GetSample(varp, 0);
	sign	= (int)GetSample(varp, 1);

	sign = (sign & 0x0040) ? -1 : 1;

	if (vfp > 1.43)
		at = vfp0[0] + vfp0[1] * vfp;
	else
	if (vfp > 0.093)
		at = vfp1[0] + vfp1[1] * vfp +
		vfp1[2] * pow((double)vfp, (double)2.0) +
		vfp1[3] * pow((double)vfp, (double)3.0) +
		vfp1[4] * pow((double)vfp, (double)4.0) +
		vfp1[5] * pow((double)vfp, (double)5.0);
	else
		at = vfp2[0] + vfp2[1] * vfp +
		vfp2[2] * pow((double)vfp, (double)2.0);

	efp = pow((double)10.0, (double)at) / 1000.0 * sign;

	PutSample(varp, efp);

}	/* END SEFEFP */

/* -------------------------------------------------------------------- */
void sefefs(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vfs, at, efs;
	int	sign;

	vfs	= GetSample(varp, 0);
	sign	= (int)GetSample(varp, 1);

	sign = (sign & 0x0040) ? -1 : 1;

	if (vfs > 1.43)
		at = vfs0[0] + vfs0[1] * vfs;
	else
	if (vfs > 0.093)
		at = vfs1[0] + vfs1[1] * vfs +
		vfs1[2] * pow((double)vfs, (double)2.0) +
		vfs1[3] * pow((double)vfs, (double)3.0) +
		vfs1[4] * pow((double)vfs, (double)4.0) +
		vfs1[5] * pow((double)vfs, (double)5.0);
	else
		at = vfs2[0] + vfs2[1] * vfs +
		vfs2[2] * pow((double)vfs, (double)2.0) +
		vfs2[3] * pow((double)vfs, (double)3.0);

	efs = pow((double)10.0, (double)at) / 1000.0 * sign;

	PutSample(varp, efs);

}	/* END SEFEFS */

/* -------------------------------------------------------------------- */
void sefent(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vnt, ent;
	int	Zrange;

	vnt	= GetSample(varp, 0);
	Zrange	= (int)GetSample(varp, 1) & 0x0004;

	if (Zrange)
		ent = (vnt - vnt0[0]) * vnt0[1];
	else
		ent = (vnt - vnt1[0]) * vnt1[1];

	PutSample(varp, ent);

}	/* END SEFENT */

/* -------------------------------------------------------------------- */
void sefenb(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vnb, enb;
	int	Zrange;

	vnb	= GetSample(varp, 0);
	Zrange	= (int)GetSample(varp, 1) & 0x0004;

	if (Zrange)
		enb = (vnb - vnb0[0]) * vnb0[1];
	else
		enb = (vnb - vnb1[0]) * vnb1[1];

	PutSample(varp, enb);

}	/* END SEFENB */

/* -------------------------------------------------------------------- */
void sefenp(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vnp, enp;
	int	Yrange;

	vnp	= GetSample(varp, 0);
	Yrange	= (int)GetSample(varp, 1) & 0x0002;

	if (Yrange)
		enp = (vnp - vnp0[0]) * vnp0[1];
	else
		enp = (vnp - vnp1[0]) * vnp1[1];

	PutSample(varp, enp);

}	/* END SEFENP */

/* -------------------------------------------------------------------- */
void sefens(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	vns, ens;
	int	Yrange;

	vns	= GetSample(varp, 0);
	Yrange	= (int)GetSample(varp, 1) & 0x0002;

	if (Yrange)
		ens = (vns - vns0[0]) * vns0[1];
	else
		ens = (vns - vns1[0]) * vns1[1];

	PutSample(varp, ens);

}	/* END SEFENS */

/* -------------------------------------------------------------------- */
void sefefqz(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	eft, efb;

	eft	= GetSample(varp, 0);
	efb	= GetSample(varp, 1);

	PutSample(varp, 0.5 * (eft + efb));

}	/* END SEFEFQZ */

/* -------------------------------------------------------------------- */
void sefefqy(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	efp, efs;

	efp	= GetSample(varp, 0);
	efs	= GetSample(varp, 1);

	PutSample(varp, 0.5 * (efp + efs));

}	/* END SEFEFQY */

/* -------------------------------------------------------------------- */
void sefenqz(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	ent, enb;

	ent	= GetSample(varp, 0);
	enb	= GetSample(varp, 1);

	PutSample(varp, 0.5 * (ent + enb));

}	/* END SENENQZ */

/* -------------------------------------------------------------------- */
void sefenqy(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	enp, ens;

	enp	= GetSample(varp, 0);
	ens	= GetSample(varp, 1);

	PutSample(varp, 0.5 * (enp + ens));

}	/* END SEFENQY */

/* -------------------------------------------------------------------- */
void sefexy(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	efqy, enqy;

	efqy	= GetSample(varp, 0);
	enqy	= GetSample(varp, 1);

	PutSample(varp, CXY * (efqy - RXY * enqy));

}	/* END SEFEXY */

/* -------------------------------------------------------------------- */
void sefexz(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	efqz, enqz;

	efqz	= GetSample(varp, 0);
	enqz	= GetSample(varp, 1);

	PutSample(varp, CXZ * (efqz - RXZ * enqz));

}	/* END SEFEXZ */

/* -------------------------------------------------------------------- */
static NR_TYPE	ezo, efz, efy, enz, eny;

void sefeyo(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	roll, eft, efb, efp, efs, ent, enb, enp, ens, fy, fz, ny,
		nz, sbank, cbank, eyo, vyo, vzo;

	roll	= GetSample(varp, 0) * DEG_RAD;
	vyo	= GetSample(varp, 1);
	vzo	= GetSample(varp, 2);
	eft	= GetSample(varp, 3);
	efb	= GetSample(varp, 4);
	efp	= GetSample(varp, 5);
	efs	= GetSample(varp, 6);
	ent	= GetSample(varp, 7);
	enb	= GetSample(varp, 8);
	enp	= GetSample(varp, 9);
	ens	= GetSample(varp, 10);

	fy = 0.5 * (efp - RFY * efs) * CFY;
	fz = 0.5 * (eft - RFZ * efb) * CFZ;
	ny = 0.5 * (enp - RNY * ens) * CNY;
	nz = 0.5 * (ent - RNZ * enb) * CNZ;

	sbank = sin(roll);
	cbank = cos(roll);

	efy = cbank * fy - sbank * fz;
	efz = cbank * fz + sbank * fy;

	eny = cbank * ny - sbank * nz;
	enz = cbank * nz + sbank * ny;

	eyo = cbank * vyo - sbank * vzo;
	ezo = cbank * vzo + sbank * vyo;

	PutSample(varp, eyo);

}	/* END SEFEYO */

/* -------------------------------------------------------------------- */
void sefezo(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, ezo);

}	/* END SEFEZO */

/* -------------------------------------------------------------------- */
void sefefz(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, efz);

}	/* END SEFEFZ */

/* -------------------------------------------------------------------- */
void sefefy(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, efy);

}	/* END SEFEFY */

/* -------------------------------------------------------------------- */
void sefenz(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, enz);

}	/* END SEFENZ */

/* -------------------------------------------------------------------- */
void sefeny(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, eny);

}	/* END SEFENY */

/* END SPEMF.C */
