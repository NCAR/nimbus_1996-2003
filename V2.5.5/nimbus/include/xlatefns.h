#ifndef DERIVEDEF_H
#define DERIVEDEF_H

struct _dnfn deriveftns[] =
		{
		"2D-C",	NULL,		xlAsync,	NULL,
		"2D-G",	NULL,		xlAsync,	NULL,
		"2D-H",	NULL,		xlAsync,	NULL,
		"2D-P",	NULL,		xlAsync,	NULL,
		"2DEE1",	NULL,		xl2d_hskp4,	NULL,
		"2DEE32",	NULL,		xl2d_hskp5,	NULL,
		"2DMTMP",	NULL,		xl2d_hskp1,	NULL,
		"2DV15",	NULL,		xl2d_hskp0,	NULL,
		"2DV5",	NULL,		xl2d_hskp7,	NULL,
		"2DV5",	NULL,		xl2d_hskp7,	NULL,
		"2DV5",	NULL,		xl2d_hskp7,	NULL,
		"2DVN15",	NULL,		xl2d_hskp6,	NULL,
		"A200X",	NULL,		xlP16v2bin,	NULL,
		"A200X_OLD",	NULL,		xlP16bin,	NULL,
		"A200Y",	NULL,		xlP16v2bin,	NULL,
		"A200Y_OLD",	NULL,		xlP16bin,	NULL,
		"A260X",	NULL,		xlP64v2bin,	NULL,
		"A260X_OLD",	NULL,		xlP260bin,	NULL,
		"AACT_OLD",	NULL,		xlaact,	NULL,
		"AASAS_OLD",	NULL,		xlP16bin,	NULL,
		"ACINS_IRS",	NULL,		xlvacc,	NULL,
		"AF300",	NULL,		xlP32v2bin,	NULL,
		"AF300_OLD",	NULL,		xlP300bin,	NULL,
		"AFSSP",	NULL,		xlP16v2bin,	NULL,
		"AFSSP_OLD",	NULL,		xlP16bin,	NULL,
		"ALPHA_INS",	NULL,		xlalpha,	NULL,
		"ALT_IRS",	NULL,		xlialt,	NULL,
		"AMASP",	NULL,		xlP32v2bin,	NULL,
		"APCAS",	NULL,		xlP16v2bin,	NULL,
		"ATMNS",	NULL,		xlatmns,	NULL,
		"BLATA_IRS",	NULL,		xlblata,	NULL,
		"BLONGA_IRS",	NULL,		xlblona,	NULL,
		"BNORMA_IRS",	NULL,		xlbnorma,	NULL,
		"BPITCHR_IRS",	NULL,		xlbpitchr,	NULL,
		"BROLLR_IRS",	NULL,		xlbrollr,	NULL,
		"BYAWR_IRS",	NULL,		xlbyawr,	NULL,
		"CCEP",	NULL,		xlccep,	NULL,
		"CGS",	NULL,		xlccgs,	NULL,
		"CHGME",	NULL,		xlchgme,	NULL,
		"CLAT",	NULL,		xlclat,	NULL,
		"CLON",	NULL,		xlclon,	NULL,
		"CLORTM",	NULL,		xlclortm,	NULL,
		"CMAGVR",	NULL,		xlcmagvr,	NULL,
		"CNAVTM",	NULL,		xlcnavtm,	NULL,
		"CROLL",	NULL,		xlcroll51,	NULL,
		"CSTAT",	NULL,		xlcstat,	NULL,
		"CTK",	NULL,		xlcctk,	NULL,
		"CTMLAG",	NULL,		xlctmlag,	NULL,
		"CWARN",	NULL,		xlcwarn,	NULL,
		"DAY",	NULL,		xlday,	NULL,
		"DMECH",	NULL,		xldmech,	NULL,
		"DMEDI",	NULL,		xldmedi,	NULL,
		"DMEFL",	NULL,		xldmefl,	NULL,
		"DRFTA_IRS",	NULL,		xlidrift,	NULL,
		"DSAT",	NULL,		xldsat,	NULL,
		"DSGALT",	NULL,		xldsgalt,	NULL,
		"DSLAT",	NULL,		xldslat,	NULL,
		"DSLON",	NULL,		xldslon,	NULL,
		"DSPS",	NULL,		xldsps,	NULL,
		"DSRH",	NULL,		xldsmr,	NULL,
		"DSTIME",	NULL,		xldstime,	NULL,
		"DSVSPD",	NULL,		xldsvspd,	NULL,
		"DSWD",	NULL,		xldswd,	NULL,
		"DSWS",	NULL,		xldsws,	NULL,
		"ETORQ",	NULL,		xletorq,	NULL,
		"EVENT",	NULL,		xlevent,	NULL,
		"FANV",	NULL,		xlpms16_hskp5,	NULL,
		"FREF",	NULL,		xlpms16_hskp7,	NULL,
		"FRNG",	NULL,		xlfrange2,	NULL,
		"FRNG_OLD",	NULL,		xlfrange,	NULL,
		"FRST",	NULL,		xlpms16_hskp1,	NULL,
		"FRST_OLD",	NULL,		xlfreset,	NULL,
		"FSIG",	NULL,		xlpms16_hskp6,	NULL,
		"FSTB",	NULL,		xlpms16_hskp0,	NULL,
		"FSTB_OLD",	NULL,		xlfstrob,	NULL,
		"FSTT",	NULL,		xlpms16_hskp3,	NULL,
		"FTMP",	NULL,		xlpms16_hskp4,	NULL,
		"GALT",	NULL,		xlgalt,	NULL,
		"GLAT",	NULL,		xlglat,	NULL,
		"GLON",	NULL,		xlglon,	NULL,
		"GMODE",	NULL,		xlgmode,	NULL,
		"GREYEE1",	NULL,		xlGrey_hskp0,	NULL,
		"GREYEE64",	NULL,		xlGrey_hskp1,	NULL,
		"GSF_IRS",	NULL,		xligspd,	NULL,
		"GSI_INS",	NULL,		xlgsi,	NULL,
		"GSTAT",	NULL,		xlgstat,	NULL,
		"GTIMP",	NULL,		xlgtimp,	NULL,
		"GTIMV",	NULL,		xlgtimv,	NULL,
		"GVEW",	NULL,		xlgvew,	NULL,
		"GVNS",	NULL,		xlgvns,	NULL,
		"GVZI",	NULL,		xlgvzi,	NULL,
		"HGM",	NULL,		xlhgm,	NULL,
		"HGM232",	NULL,		xlhgm232,	NULL,
		"HGM232S",	NULL,		xlhgm232s,	NULL,
		"HGME",	NULL,		xlhgme,	NULL,
		"HOUR",	NULL,		xlhour,	NULL,
		"INSSEC_INS",	NULL,		xlinssec,	NULL,
		"IWD_IRS",	NULL,		xliwd,	NULL,
		"IWS_IRS",	NULL,		xliws,	NULL,
		"LAG10_IRS",	NULL,		xllag10,	NULL,
		"LAG25_IRS",	NULL,		xllag25,	NULL,
		"LAG50_IRS",	NULL,		xllag50,	NULL,
		"LAG5_IRS",	NULL,		xllag5,	NULL,
		"LAT_INS",	NULL,		xlalat,	NULL,
		"LAT_IRS",	NULL,		xlilat,	NULL,
		"LON_INS",	NULL,		xlalon,	NULL,
		"LON_IRS",	NULL,		xlilon,	NULL,
		"MINUTE",	NULL,		xlmin,	NULL,
		"MONTH",	NULL,		xlmonth,	NULL,
		"MRUV",	NULL,		xlmruv,	NULL,
		"OBBCOD",	NULL,		xlobbcod,	NULL,
		"ODETDC",	NULL,		xlodetdc,	NULL,
		"ODETSG",	NULL,		xlodetsg,	NULL,
		"OGAIN",	NULL,		xlogain,	NULL,
		"OSMOTR",	NULL,		xlosmotr,	NULL,
		"OTBBC",	NULL,		xlotbbc,	NULL,
		"OTBBF",	NULL,		xlotbbf,	NULL,
		"OTBNCH",	NULL,		xlotbnch,	NULL,
		"OTBOX",	NULL,		xlotbox,	NULL,
		"OTDET",	NULL,		xlotdet,	NULL,
		"OTSNT",	NULL,		xlotsnt,	NULL,
		"P200X",	NULL,		xlpartspace,	NULL,
		"P200Y",	NULL,		xlpartspace,	NULL,
		"P260X",	NULL,		xlpartspace,	NULL,
		"P6E01",	NULL,		xlpms64_hskp5,	NULL,
		"P6E64",	NULL,		xlpms64_hskp6,	NULL,
		"P6RST",	NULL,		xlpms64_hskp0,	NULL,
		"P6TMP",	NULL,		xlpms64_hskp4,	NULL,
		"PACT",	NULL,		xlpms16_hskp0,	NULL,
		"PBRS",	NULL,		xlpms16_hskp7,	NULL,
		"PCACTV",	NULL,		xlpms16_hskp0,	NULL,
		"PCFLOW",	NULL,		xlpms16_hskp6,	NULL,
		"PF300",	NULL,		xlpartspace,	NULL,
		"PFLW",	NULL,		xlpms16_hskp6,	NULL,
		"PFSSP",	NULL,		xlpartspace,	NULL,
		"PHDG",	NULL,		xlphdg51,	NULL,
		"PITCH",	NULL,		xlpitch51,	NULL,
		"PITCH_IRS",	NULL,		xlipitch,	NULL,
		"PPCAS",	NULL,		xlpartspace,	NULL,
		"PREF",	NULL,		xlpms16_hskp5,	NULL,
		"PSFD",	NULL,		xlpsfd,	NULL,
		"PSFD1",	NULL,		xlpsfd_ADS2,	NULL,
		"PSTT",	NULL,		xlpms16_hskp1,	NULL,
		"PTMP",	NULL,		xlpms16_hskp4,	NULL,
		"ROLL",	NULL,		xlroll51,	NULL,
		"ROLL_IRS",	NULL,		xliroll,	NULL,
		"SECOND",	NULL,		xlsec,	NULL,
		"SERIAL",	NULL,		xlserial,	NULL,
		"SHDORC",	NULL,		xl2d_shadow,	NULL,
		"SHDORP",	NULL,		xl2d_shadow,	NULL,
		"THDG_IRS",	NULL,		xlithdg,	NULL,
		"THI_INS",	NULL,		xlthi,	NULL,
		"TKAR_IRS",	NULL,		xlitrkart,	NULL,
		"TKAT_IRS",	NULL,		xlittrka,	NULL,
		"TRKI_INS",	NULL,		xltrki,	NULL,
		"UVCHRF",	NULL,		xluvchrf,	NULL,
		"UVCHSP",	NULL,		xluvchsp,	NULL,
		"UVHREF",	NULL,		xluvhref,	NULL,
		"UVHSMP",	NULL,		xluvhsmp,	NULL,
		"UVKRRF",	NULL,		xluvkrrf,	NULL,
		"UVKRSP",	NULL,		xluvkrsp,	NULL,
		"UVPVOL",	NULL,		xluvpvol,	NULL,
		"UVTVOL",	NULL,		xluvtvol,	NULL,
		"VEW_IRS",	NULL,		xlivew,	NULL,
		"VNS_IRS",	NULL,		xlivns,	NULL,
		"VSPD_IRS",	NULL,		xlivspd,	NULL,
		"VZI",	NULL,		xlvzi51,	NULL,
		"XBABS",	NULL,		ogren2,	NULL,
		"XBCUT",	NULL,		ogren1,	NULL,
		"XBSPD",	NULL,		ogren3,	NULL,
		"XBSPW",	NULL,		ogren4,	NULL,
		"XBSPWRH",	NULL,		ogren5,	NULL,
		"XMASS1",	NULL,		mass1,	NULL,
		"XMASS2",	NULL,		mass2,	NULL,
		"XMASS3",	NULL,		mass3,	NULL,
		"XMASS4",	NULL,		mass4,	NULL,
		"XPSFD",	NULL,		xlxpsfd,	NULL,
		"XVI_INS",	NULL,		xlxvi,	NULL,
		"YEAR",	NULL,		xlyear,	NULL,
		"YVI_INS",	NULL,		xlyvi,	NULL,
		NULL,		NULL,	NULL,	NULL
		};

#endif
