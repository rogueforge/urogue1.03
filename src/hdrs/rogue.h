/*
 * Last Modified: 05/25/86
 */
/*
 * Rogue definitions and variable declarations
 *
 */
#include "defs.h"
#include "tunable.h"
/*
 * Maximum number of different things
 */
#define	MAXROOMS	9
#define	MAXTHINGS	9
#define	MAXOBJ		8
#define	MAXPACK		23
#define	MAXTREAS	30	/* number monsters/treasure in treasure room */
#define	MAXTRAPS	50	/* max traps per level */
#define	MAXTRPTRY	16	/* attempts/level allowed for setting traps */
#define	MAXDOORS	4	/* Maximum doors to a room */
#define	MAXPRAYERS_C    12	/* Maximum number of prayers for clerics */
#define MAXPRAYERS_D    12      /* Maximum number of prayers for druids */
#define	MAXSPELLS_M	21	/* Maximum number of spells (for magician) */
#define	MAXSPELLS_I	21	/* Maximum number of spells (for illusionist */
#define	NUMMONST	385	/* Current number of monsters */
#define NUMSUMMON	48	/* number of creatures that can summon hero */
#define	NLEVMONS	4	/* Number of new monsters per level */
#define MAXPURCH	8	/* max purchases per trading post visit */
#define LINELEN		80	/* characters in a buffer */

/* These defines are used by get_play.c */
#define	I_STR		0
#define	I_INTEL		1
#define	I_WIS		2
#define	I_DEX		3
#define	I_WELL		4
#define	I_APPEAL	5
#define	I_HITS		6
#define	I_ARM		7
#define	I_WEAP		8
#define	I_CHAR		9
#define I_WEAPENCH	10
#define	MAXPATT		11	/* Total Number of above defines. */
#define	MAXPDEF		4	/* Maximum number of pre-defined chars */

/* Movement penalties */
#define BACKPENALTY 3
#define SHOTPENALTY 2		/* In line of sight of missile */
#define DOORPENALTY 1		/* Moving out of current room */

/*
 * stuff to do with encumberance
 */
#define NORMENCB	1500	/* normal encumberance */
#define F_OK		 0	/* have plenty of food in stomach */
#define F_HUNGRY	 1	/* player is hungry */
#define F_WEAK		 2	/* weak from lack of food */
#define F_FAINT		 3	/* fainting from lack of food */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/* 
 * The character types
 */
#define	C_FIGHTER	0
#define C_PALADIN	1
#define C_RANGER	2
#define	C_CLERIC	3
#define C_DRUID		4
#define	C_MAGICIAN	5
#define C_ILLUSION	6
#define	C_THIEF		7
#define C_ASSASIN	8
#define C_NINJA		9
#define	C_MONSTER	10
#define	C_NOTSET	11	/* Must not be a value from above */

/*
 * values for games end
 */
#define SCOREIT -1
#define KILLED 	 0
#define CHICKEN  1
#define WINNER   2
#define TOTAL    3

/*
 * definitions for function step_ok:
 *	MONSTOK indicates it is OK to step on a monster -- it
 *	is only OK when stepping diagonally AROUND a monster
 */
#define MONSTOK 1
#define NOMONST 2

/*
 * used for ring stuff
 */
#define LEFT_1	 0
#define LEFT_2	 1
#define LEFT_3	 2
#define LEFT_4	 3
#define RIGHT_1	 4
#define RIGHT_2	 5
#define RIGHT_3	 6
#define RIGHT_4	 7

/*
 * All the fun defines
 */
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define winat(y, x) (mvwinch(mw, y, x)==' '?mvwinch(stdscr, y, x):winch(mw))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)	(struct object *)((*what).l_data)
#define THINGPTR(what)	(struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define on(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) != 0)
#define off(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) == 0)
#define turn_on(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag & ~FLAGMASK))
#define turn_off(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~flag)
#if !defined(CTRL)
#define CTRL(ch) ('ch' & 037)
#endif
#define ALLOC(x) malloc((unsigned int) x)
#define FREE(x) cfree((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT_1, r) || ISRING(LEFT_2, r) ||\
			 ISRING(LEFT_3, r) || ISRING(LEFT_4, r) ||\
			 ISRING(RIGHT_1, r) || ISRING(RIGHT_2, r) ||\
			 ISRING(RIGHT_3, r) || ISRING(RIGHT_4, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == FOOD)
#define isrock(ch) ((ch == WALL) || (ch == '-') || (ch == '|'))
#define is_stealth(tp) \
    (rnd(25) < (tp)->t_stats.s_dext || (tp == &player && ISWEARING(R_STEALTH)))
#if	defined(TCFLSH)
#define flushout() ioctl(_tty_ch,TCFLSH,0)
#endif
#define mi_wght mi_worth

/*
 * Ways to die
 */
#define	D_PETRIFY	-1
#define	D_ARROW		-2
#define	D_DART		-3
#define	D_POISON	-4
#define	D_BOLT		-5
#define	D_SUFFOCATION	-6
#define	D_POTION	-7
#define	D_INFESTATION	-8
#define D_DROWN		-9
#define D_FALL		-10
#define D_FIRE		-11

/*
 * Things that appear on the screens
 */
#define	WALL		' '
#define	PASSAGE		'#'
#define	DOOR		'+'
#define	FLOOR		'.'
#define VPLAYER 	'@'
#define IPLAYER 	'_'
#define	POST		'^'
#define LAIR		'('
#define RUSTTRAP	';'
#define	TRAPDOOR	'>'
#define	ARROWTRAP	'{'
#define	SLEEPTRAP	'$'
#define	BEARTRAP	'}'
#define	TELTRAP		'~'
#define	DARTTRAP	'`'
#define POOL		'"'
#define MAZETRAP	'\\'
#define FIRETRAP	'<'
#define POISONTRAP	'['
#define ARTIFACT	','
#define	SECRETDOOR	'&'
#define	STAIRS		'%'
#define	GOLD		'*'
#define	POTION		'!'
#define	SCROLL		'?'
#define	MAGIC		'$'
#define	BMAGIC		'>'	/*	Blessed	magic	*/
#define	CMAGIC		'<'	/*	Cursed	magic	*/
#define	FOOD		':'
#define	WEAPON		')'
#define	ARMOR		']'
#define	RING		'='
#define	STICK		'/'
#define	CALLABLE	-1
#define	MARKABLE	-2

/*
 * Various constants
 */
#define	MAXAUTH		10		/* Let's be realistic! */
#define	PASSWD		rpass.rp_pass
#define PASSWDSEED	rpass.rp_pkey
#define	BEARTIME	3
#define	SLEEPTIME	4
#define	FREEZETIME	6
#define	HEALTIME	30
#define	HOLDTIME	2
#define	CHILLTIME	(roll(2, 4))
#define	SMELLTIME	20
#define	STONETIME	8
#define	SICKTIME	10
#define	STPOS		0
#define	WANDERTIME	60
#define HEROTIME	20
#define	BEFORE		1
#define	AFTER		2
#define	HUHDURATION	20
#define	SEEDURATION	850
#define	CLRDURATION	15
#define GONETIME	200
#define	PHASEDURATION	300
#define	HUNGERTIME	1300
#define	MORETIME	150
#define	STINKTIME	6
#define	STOMACHSIZE	2000
#define	ESCAPE		27
#define LINEFEED	10
#define CARRIAGE_RETURN 13
#define	BOLT_LENGTH	10
#define	MARKLEN		20

/*
 * Save against things
 */
#define	VS_POISON		0
#define	VS_PARALYZATION		0
#define	VS_DEATH		0
#define	VS_PETRIFICATION	1
#define	VS_WAND			2
#define	VS_BREATH		3
#define	VS_MAGIC		4

/*
 * attributes for treasures in dungeon
 */
#define ISCURSED	0x00000001
#define ISKNOW		0x00000002
#define ISPOST		0x00000004	/* object is in a trading post */
#define	ISMETAL     	0x00000008
#define ISPROT		0x00000010	/* object is protected */
#define ISBLESSED	0x00000020
#define ISZAPPED	0x00000040	/* weapon has been charged by dragon */
#define ISVORPED	0x00000080	/* vorpalized weapon */
#define ISSILVER	0x00000100	/* silver weapon */
#define ISPOISON	0x00000200	/* poisoned weapon */
#define CANRETURN	0x00000400	/* weapon returns if misses */
#define ISOWNED		0x00000800	/* weapon returns always */
#define ISLOST		0x00001000	/* weapon always disappears */
#define ISMISL		0x00002000
#define ISMANY		0x00004000
#define CANBURN		0x00008000	/* burns monsters */
#define ISSHARP		0x00010000	/* cutting edge */
#define ISBIG		0x00020000
#define ISLITTLE	0x00040000

/*
 * Various flag bits
 */
#define ISDARK		0x00000001
#define ISGONE		0x00000002
#define	ISTREAS		0x00000004
#define ISFOUND		0x00000008
#define ISTHIEFSET	0x00000010
#define WASDARK		0x00000020

/*
 * thing flags (this might include player) for monster attributes
 */

#define ISBLIND     	0x00000001
#define	ISINWALL     	0x00000002
#define ISRUN	    	0x00000004
#define	ISFLEE	    	0x00000008
#define ISINVIS     	0x00000010
#define ISMEAN      	0x00000020
#define ISGREED     	0x00000040
#define CANSHOOT     	0x00000080
#define ISHELD      	0x00000100
#define ISHUH       	0x00000200
#define ISREGEN     	0x00000400
#define CANHUH      	0x00000800
#define CANSEE      	0x00001000
#define HASFIRE	    	0x00002000
#define ISSLOW	    	0x00004000
#define ISHASTE     	0x00008000
#define ISCLEAR     	0x00010000
#define CANINWALL   	0x00020000
#define ISDISGUISE 	0x00040000
#define CANBLINK   	0x00080000
#define CANSNORE   	0x00100000
#define HALFDAMAGE	0x00200000
#define	CANSUCK		0x00400000
#define	CANRUST		0x00800000
#define	CANPOISON	0x01000000
#define	CANDRAIN	0x02000000
#define ISUNIQUE	0x04000000
#define	STEALGOLD	0x08000000
#define	STEALMAGIC	0x10000001
#define	CANDISEASE	0x10000002
#define HASDISEASE	0x10000004
#define CANSUFFOCATE	0x10000008
#define DIDSUFFOCATE	0x10000010
#define BOLTDIVIDE	0x10000020
#define BLOWDIVIDE	0x10000040
#define NOCOLD		0x10000080
#define	TOUCHFEAR	0x10000100
#define BMAGICHIT	0x10000200
#define NOFIRE		0x10000400
#define NOBOLT		0x10000800
#define CARRYGOLD	0x10001000
#define CANITCH		0x10002000
#define HASITCH		0x10004000
#define DIDDRAIN	0x10008000
#define WASTURNED	0x10010000
#define CANSELL		0x10020000
#define CANBLIND	0x10040000
#define CANBBURN	0x10080000
#define ISCHARMED	0x10100000
#define CANSPEAK	0x10200000
#define CANFLY		0x10400000
#define ISFRIENDLY	0x10800000
#define CANHEAR		0x11000000
#define ISDEAF		0x12000000
#define CANSCENT	0x14000000
#define ISUNSMELL	0x18000000
#define WILLRUST	0x20000001
#define WILLROT		0x20000002
#define SUPEREAT	0x20000004
#define PERMBLIND	0x20000008
#define MAGICHIT	0x20000010
#define CANINFEST	0x20000020
#define HASINFEST	0x20000040
#define NOMOVE		0x20000080
#define CANSHRIEK	0x20000100
#define CANDRAW		0x20000200
#define CANSMELL	0x20000400
#define CANPARALYZE	0x20000800
#define CANROT		0x20001000
#define ISSCAVENGE	0x20002000
#define DOROT		0x20004000
#define CANSTINK	0x20008000
#define HASSTINK	0x20010000
#define ISSHADOW	0x20020000
#define CANCHILL	0x20040000
#define	CANHUG		0x20080000
#define CANSURPRISE	0x20100000
#define CANFRIGHTEN	0x20200000
#define CANSUMMON	0x20400000
#define TOUCHSTONE	0x20800000
#define LOOKSTONE	0x21000000
#define CANHOLD		0x22000000
#define DIDHOLD		0x24000000
#define DOUBLEDRAIN	0x28000000
#define ISUNDEAD	0x30000001
#define BLESSMAP	0x30000002
#define BLESSGOLD	0x30000004
#define BLESSMONS	0x30000008
#define BLESSMAGIC	0x30000010
#define BLESSFOOD	0x30000020
#define CANBRANDOM	0x30000040	/* Types of breath */
#define CANBACID	0x30000080
#define CANBFIRE	0x30000100
#define CANBBOLT	0x30000200
#define CANBGAS		0x30000400
#define CANBICE		0x30000800
#define CANBPGAS	0x30001000	/* Paralyze gas */
#define CANBSGAS	0x30002000	/* Sleeping gas */
#define CANBSLGAS	0x30004000	/* Slow gas */
#define CANBFGAS	0x30008000	/* Fear gas */
#define CANBREATHE	0x3000ffc0	/* Can it breathe at all? */
#define STUMBLER	0x30010000
#define POWEREAT	0x30020000
#define ISELECTRIC	0x30040000
#define HASOXYGEN	0x30080000
#define POWERDEXT	0x30100000
#define POWERSTR	0x30200000
#define POWERWISDOM	0x30400000
#define POWERINTEL	0x30800000
#define POWERCONST	0x31000000
#define SUPERHERO	0x32000000
#define ISUNHERO	0x34000000
#define CANCAST		0x38000000
#define CANTRAMPLE	0x40000001
#define CANSWIM		0x40000002
#define LOOKSLOW	0x40000004
#define CANWIELD	0x40000008
#define CANDARKEN	0x40000010
#define ISFAST		0x40000020
#define CANBARGAIN	0x40000040
#define NOMETAL		0x40000080
#define CANSPORE	0x40000100
#define NOSHARP		0x40000200
#define DRAINWISDOM	0x40000400
#define DRAINBRAIN	0x40000800
#define ISLARGE		0x40001000
#define ISSMALL		0x40002000
#define CANSTAB		0x40004000
#define ISFLOCK		0x40008000
#define ISSWARM		0x40010000
#define CANSTICK	0x40020000
#define CANTANGLE	0x40040000
#define DRAINMAGIC	0x40080000
#define SHOOTNEEDLE	0x40100000
#define CANZAP		0x40200000
#define HASARMOR	0x40400000
#define CANTELEPORT	0x40800000
#define ISBERSERK	0x41000000
#define ISFAMILIAR      0x42000000
#define HASFAMILIAR     0x44000000
#define SUMMONING       0x48000000
#define CANREFLECT      0x50000001
  

/* Masks for choosing the right flag */
#define FLAGMASK	0xf0000000
#define FLAGINDEX	0x0000000f
#define FLAGSHIFT	28

/* 
 * Mask for cancelling special abilities 
 * The flags listed here will be the ones left on after the
 * cancellation takes place
 */
#define CANC0MASK (	ISBLIND		| ISINWALL	| ISRUN		| \
			ISFLEE		| ISMEAN	| ISGREED	| \
			CANSHOOT	| ISHELD	| ISHUH		| \
			ISSLOW		| ISHASTE	| ISCLEAR	| \
			ISUNIQUE)
#define CANC1MASK (	HASDISEASE	| DIDSUFFOCATE	| CARRYGOLD 	| \
			HASITCH		| CANSELL 	| CANBBURN	| \
			CANSPEAK	| CANFLY	| ISFRIENDLY)
#define CANC2MASK (	HASINFEST	| NOMOVE	| ISSCAVENGE	| \
			DOROT		| HASSTINK	| DIDHOLD)
#define CANC3MASK (	ISUNDEAD	| CANBREATHE	| CANCAST)
#define CANC4MASK (	CANTRAMPLE	| CANSWIM	| CANWIELD	| \
			ISFAST		| CANBARGAIN	| CANSPORE	| \
			ISLARGE		| ISSMALL	| ISFLOCK	| \
			ISSWARM		| CANSTICK	| CANTANGLE	| \
			SHOOTNEEDLE	| CANZAP	| HASARMOR	| \
			CANTELEPORT	| ISBERSERK	| ISFAMILIAR	| \
			HASFAMILIAR	| SUMMONING)
#define CANC5MASK (	CANREFLECT)
#define CANC6MASK (	0 )
#define CANC7MASK (	0 )
#define CANC8MASK (	0 )
#define CANC9MASK (	0 )
#define CANCAMASK (	0 )
#define CANCBMASK (	0 )
#define CANCCMASK (	0 )
#define CANCDMASK (	0 )
#define CANCEMASK (	0 )
#define CANCFMASK (	0 )

/* types of things */
#define TYP_POTION	0
#define TYP_SCROLL	1
#define TYP_FOOD	2
#define TYP_WEAPON	3
#define TYP_ARMOR	4
#define TYP_RING	5
#define TYP_STICK	6
#define TYP_ARTIFACT	7
#define	NUMTHINGS	8

/*
 * Potion types
 */
#define	P_CLEAR		0
#define	P_ABIL		1
#define	P_SEEINVIS	2
#define	P_HEALING	3
#define	P_MFIND		4
#define	P_TFIND		5
#define	P_RAISE		6
#define	P_HASTE		7
#define	P_RESTORE	8
#define	P_PHASE		9
#define P_INVIS		10
#define P_SMELL		11
#define P_HEAR		12
#define P_SHERO		13
#define P_DISGUISE	14
#define	MAXPOTIONS	15

/*
 * Scroll types
 */
#define	S_CONFUSE	0
#define	S_MAP		1
#define	S_LIGHT		2
#define	S_HOLD		3
#define	S_SLEEP		4
#define	S_ALLENCH	5
#define	S_IDENT		6
#define	S_SCARE		7
#define	S_GFIND		8
#define	S_TELEP		9
#define	S_CREATE	10
#define	S_REMOVE	11
#define	S_PETRIFY	12
#define	S_GENOCIDE	13
#define	S_CURING	14
#define S_MAKEIT	15
#define S_PROTECT	16
#define S_NOTHING	17
#define S_SILVER	18
#define S_OWNERSHIP	19
#define S_FOODFIND	20
#define S_ELECTRIFY     21
#define S_CHARM         22
#define S_SUMMON        23
#define S_REFLECT       24
#define	MAXSCROLLS	25

/*
 * Weapon types
 */
#define SLING		0		/* sling */
#define ROCK		1		/* rocks */
#define BULLET		2		/* sling bullet */
#define BOW		3		/* short bow */
#define ARROW		4		/* arrow */
#define SILVERARROW	5		/* silver arrows */
#define FLAMEARROW	6		/* flaming arrows */
#define FOOTBOW		7		/* footbow */
#define FBBOLT		8		/* footbow bolt */
#define CROSSBOW	9		/* crossbow */
#define BOLT		10		/* crossbow bolt */

#define DART		11		/* darts */
#define DAGGER		12		/* dagger */
#define HAMMER		13		/* hammer */
#define LEUKU		14		/* leuku */
#define JAVELIN		15		/* javelin */
#define TOMAHAWK	16		/* tomahawk */
#define MACHETE		17		/* machete */
#define THROW_AXE	18		/* throwing axe */
#define SHORT_SPEAR	19		/* spear */
#define BOOMERANG	20		/* boomerangs */
#define LONG_SPEAR	21		/* spear */
#define SHIRIKEN	22		/* shurikens */
#define MOLOTOV		23		/* molotov cocktails */
#define GRENADE		24		/* grenade for explosions */
#define CLUB		25		/* club */
#define PITCHFORK	26		/* pitchfork */
#define SHORT_SWORD	27		/* short sword */
#define HAND_AXE	28		/* hand axe */
#define PARTISAN	29		/* partisan */
#define GRAIN_FLAIL	30		/* grain flail */
#define SINGLESTICK	31		/* singlestick*/
#define RAPIER		32		/* rapier */
#define SICKLE		33		/* sickle */
#define HATCHET		34		/* hatchet */
#define SCIMITAR	35		/* scimitar */
#define LIGHT_MACE	36		/* mace */
#define MORNINGSTAR	37		/* morning star */
#define BROAD_SWORD	38		/* broad sword */
#define MINER_PICK	39		/* miner's pick */
#define GUISARME	40		/* guisarme */
#define WAR_FLAIL	41		/* war flail */
#define CRYSKNIFE	42		/* crysknife */
#define BATTLE_AXE	43		/* battle axe */
#define CUTLASS		44		/* cutlass sword */
#define GLAIVE		45		/* glaive */
#define PERTUSKA	46		/* pertuska */
#define LONG_SWORD	47		/* long sword */
#define LANCE		48		/* lance */
#define SHORT_PIKE	49		/* short pike */
#define SABRE		50		/* sabre */
#define SPETUM		51		/* spetum */
#define HALBERD		52		/* halberd */
#define TRIDENT		53		/* trident */
#define WAR_PICK	54		/* war pick */
#define BARDICHE	55 		/* bardiche */
#define HEAVY_MACE	56		/* mace */
#define SCYTHE		57		/* great scythe */
#define QUARTERSTAFF	58		/* quarter staff */
#define BAST_SWORD	59		/* bastard sword */
#define LONG_PIKE	60		/* long pike */
#define TWO_FLAIL	61		/* two-handed flail */
#define TWO_MAUL	62		/* two-handed maul */
#define TWO_PICK	63		/* two-handed pick */
#define TWO_SWORD	64		/* two-handed sword */
#define CLAYMORE	65		/* claymore sword */
#define MAXWEAPONS	66		/* types of weapons */
#define NONE		100		/* no weapon */

/*
 * Armor types
 */
#define	SOFT_LEATHER	0
#define	CUIRBOLILLI	1
#define	HEAVY_LEATHER	2
#define	RING_MAIL	3
#define	STUDDED_LEATHER	4
#define	SCALE_MAIL	5
#define	PADDED_ARMOR	6
#define	CHAIN_MAIL	7
#define	BRIGANDINE	8
#define	SPLINT_MAIL	9
#define	BANDED_MAIL	10
#define	GOOD_CHAIN	11
#define	PLATE_MAIL	12
#define	PLATE_ARMOR	13
#define	MITHRIL		14
#define	CRYSTAL_ARMOR   15
#define	MAXARMORS	16

/*
 * Ring types
 */
#define	R_PROTECT	0
#define	R_ADDSTR	1
#define	R_SUSABILITY	2
#define	R_SEARCH	3
#define	R_SEEINVIS	4
#define	R_ALERT		5
#define	R_AGGR		6
#define	R_ADDHIT	7
#define	R_ADDDAM	8
#define	R_REGEN		9
#define	R_DIGEST	10
#define	R_TELEPORT	11
#define	R_STEALTH	12
#define	R_ADDINTEL	13
#define	R_ADDWISDOM	14
#define	R_HEALTH	15
#define R_VREGEN	16
#define R_LIGHT		17
#define R_DELUSION	18
#define R_CARRYING	19
#define R_ADORNMENT	20
#define R_LEVITATION	21
#define R_FIRERESIST	22
#define R_COLDRESIST	23
#define R_ELECTRESIST	24
#define R_RESURRECT	25
#define R_BREATHE	26
#define R_FREEDOM	27
#define R_WIZARD	28
#define R_TELCONTROL	29
#define	MAXRINGS	30

/*
 * Rod/Wand/Staff types
 */

#define	WS_LIGHT	0
#define	WS_HIT		1
#define	WS_ELECT	2
#define	WS_FIRE		3
#define	WS_COLD		4
#define	WS_POLYMORPH	5
#define	WS_MISSILE	6
#define	WS_SLOW_M	7
#define	WS_DRAIN	8
#define	WS_CHARGE	9
#define	WS_TELMON	10
#define	WS_CANCEL	11
#define WS_CONFMON	12
#define WS_ANNIH	13
#define WS_ANTIM	14
#define WS_PARALYZE	15
#define WS_MDEG		16
#define WS_NOTHING	17
#define WS_INVIS	18
#define WS_BLAST	19
#define	MAXSTICKS	20

/*
 * Food types
 */

#define FD_RATION	0
#define FD_FRUIT	1
#define FD_CRAM		2
#define FD_CAKES	3
#define FD_LEMBA	4
#define FD_MIRUVOR	5
#define	MAXFOODS	6

/*
 * Artifact types
 */

#define TR_PURSE	0
#define TR_PHIAL	1
#define TR_AMULET	2
#define TR_PALANTIR	3
#define TR_CROWN	4
#define TR_SCEPTRE	5
#define TR_SILMARIL	6
#define TR_WAND		7
#define MAXARTIFACT	8

/*
 * Artifact flags
 */

#define ISUSED	       01
#define ISACTIVE       02

/*
 * These are the types of inventory styles.
 */
#define INV_SLOW	0
#define INV_OVER	1
#define INV_CLEAR	2

/* 
 * The following determine if you can cast various spells
 */
#define MAGICIAN_ABILITY	player.t_ctype == C_MAGICIAN || \
				pstats.s_intel > 15 

#define ILLUSION_ABILITY	player.t_ctype == C_ILLUSION || \
				pstats.s_intel > 15 

#define CLERIC_ABILITY          player.t_ctype == C_CLERIC || \
		                (player.t_ctype == C_PALADIN && \
				 pstats.s_lvl > 8) || pstats.s_wisdom > 16

#define DRUID_ABILITY           player.t_ctype == C_DRUID || \
		                (player.t_ctype == C_RANGER && \
			   	pstats.s_lvl > 8) || pstats.s_wisdom > 16

#define FAMILIAR_ABILITY    	off(player,HASFAMILIAR) && \
				on(player,CANSUMMON)

#define PRAYER_ABILITY          player.t_ctype == C_DRUID || \
				player.t_ctype == C_CLERIC || \
				(player.t_ctype == C_PALADIN && \
				pstats.s_lvl > 8) || \
				(player.t_ctype == C_RANGER && \
				pstats.s_lvl > 8)

/*
 * Miscellaneous defines
 */
#define good_monster(m) on(m, ISCHARMED) || \
			on(m, ISFRIENDLY) || \
			on(m, ISFAMILIAR)

/*
 * Now we define the structures and types
 */

/*
 * level types
 */
typedef enum {
	NORMLEV,	/* normal level */
	POSTLEV,	/* trading post level */
	MAZELEV,	/* maze level */
	THRONE		/* unique monster's throne room */
} LEVTYPE;

/*
 * Help list
 */

struct h_list {
    char h_ch;
    char *h_desc;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
    char l_letter;			/* Letter for inventory */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
    int mi_worth;
    int mi_curse;
    int mi_bless;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    long r_flags;			/* Info about the room */
    short r_fires;			/* Number of fires in room */
    int r_nexits;			/* Number of exits */
    coord r_exit[MAXDOORS];		/* Where the exits are */
};

/*
 * Initial artifact stats
 */
struct init_artifact {
	char *ar_name;		/* name of the artifact */
	int ar_level;		/* first level where it appears */
	int ar_rings;		/* number of ring effects */
	int ar_potions;		/* number of potion effects */
	int ar_scrolls;		/* number of scroll effects */
	int ar_wands;		/* number of wand effects */
	int ar_worth;		/* gold pieces */
	int ar_weight;		/* weight of object */
};

/*
 * Artifact attributes
 */
struct artifact {
	long ar_flags;		/* general flags */
	long ar_rings;		/* ring effects flags */
	long ar_potions;	/* potion effects flags */
	long ar_scrolls;	/* scroll effects flags */
	long ar_wands;		/* wand effects flags */
	struct linked_list *t_art; /* linked list pointer */
};

/*
 * Array of all traps on this level
 */
struct trap {
    char tr_type;			/* What kind of trap */
    char tr_show;			/* Where disguised trap looks like */
    coord tr_pos;			/* Where trap is */
    long tr_flags;			/* Info about trap (i.e. ISFOUND) */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    short s_str;			/* Strength */
    short s_intel;			/* Intelligence */
    short s_wisdom;			/* Wisdom */
    short s_dext;			/* Dexterity */
    short s_const;			/* Constitution */
    short s_charisma;			/* Charisma */
    unsigned long s_exp;		/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    int s_pack;				/* current weight of his pack */
    int s_carry;			/* max weight he can carry */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure describing a fighting being (monster at initialization)
 */
struct mstats {
    short s_str;			/* Strength */
    long s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    char *s_hpt;			/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    bool t_turn;			/* If slowed, is it a turn to move */
    bool t_wasshot;			/* Was character shot last round? */
    char t_type;			/* What it is */
    char t_disguise;			/* What mimic looks like */
    char t_oldch;			/* Character that was where it was */
    short t_ctype;			/* Character type */
    short t_index;			/* Index into monster table */
    short t_no_move;			/* How long the thing can't move */
    short t_quiet;			/* used in healing */
    short t_doorgoal;			/* What door are we heading to? */
    coord t_pos;			/* Position */
    coord t_oldpos;			/* Last position */
    coord *t_dest;			/* Where it is running to */
    unsigned long t_flags[16];		/* State word */
    struct linked_list *t_pack;		/* What the thing is carrying */
    struct stats t_stats;		/* Physical description */
    struct stats maxstats;		/* maximum(or initial) stats */
};

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    short m_carry;			/* Probability of carrying something */
    bool m_normal;			/* Does monster exist? */
    bool m_wander;			/* Does monster wander? */
    char m_appear;			/* What does monster look like? */
    char *m_intel;			/* Intelligence range */
    long m_flags[16];			/* Things about the monster */
    char *m_typesum;			/* type of creature can he summon */
    short m_numsum;			/* how many creatures can he summon */
    short m_add_exp;			/* Added experience per hit point */
    struct mstats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char *o_text;			/* What it says if you read it */
    char o_launch;			/* What you need to launch it */
    char *o_damage;			/* Damage if used like sword */
    char *o_hurldmg;			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    long o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
    int o_weight;			/* weight of this object */
    char o_mark[MARKLEN];		/* Mark the specific object */
    struct artifact art_stats;		/* substructure for artifacts */
};
/*
 * weapon structure
 */
struct init_weps {
    char *w_name;		/* name of weapon */
    char *w_dam;		/* hit damage */
    char *w_hrl;		/* hurl damage */
    char w_launch;		/* need to launch it */
    int  w_flags;		/* flags */
    int  w_wght;		/* weight of weapon */
    int  w_worth;		/* worth of this weapon */
};

/*
 * armor structure 
 */
struct init_armor {
	char *a_name;		/* name of armor */
	int  a_prob;		/* chance of getting armor */
	int  a_class;		/* normal armor class */
	int  a_worth;		/* worth of armor */
	int  a_wght;		/* weight of armor */
};

struct matrix {
    int base;			/* Base to-hit value (AC 10) */
    int max_lvl;		/* Maximum level for changing value */
    int factor;			/* Amount base changes each time */
    int offset;			/* What to offset level */
    int range;			/* Range of levels for each offset */
};

struct spells {
    short s_which;		/* which scroll or potion */
    short s_cost;		/* cost of casting spell */
    short s_type;		/* scroll or potion */
    bool  s_blessed;		/* is the spell blessed? */
};

struct  real_pass {
	char	rp_pass[20];
	char	rp_pkey[2];
} ;

struct linked_list 
    *find_mons(), *find_obj(), *get_item(), *new_item(), *creat_mons(),
    *new_thing(), *wake_monster(), *get_hurl(), *spec_item(), 
    *find_mons_around(),*summon_monster();

struct object *wield_weap();

#if !defined(unctrl)
char *malloc(), *getenv(), *unctrl(), *tr_name(), *new(), *sprintf();
#else
char *malloc(), *getenv(), *tr_name(), *new(), *sprintf();
#endif
char *vowelstr(), *inv_name(), *strcpy(), *strcat(), *sbrk(), *brk();
char *ctime(), *num(), *ring_num(), *blesscurse(), *typ_name();

struct room *roomin();

coord *rndmove(), *can_shoot(), *find_shoot();

short randmonster();

int	auto_save(), quit(), endit(), nohaste(), res_strength(), doctor(), 
	runners(), swander(), tstp(), unconfuse(), unsee(), unclrhead(), 
	unphase(), noslow(), rollwand(), stomach(), sight(), unstink(), 
	suffocate(), cure_disease(), un_itch(), appear(), 
	unscent(), scent(), hear(), unhear(),
	unelectrify(), unshero(), unbhero(), unxray(),
	undisguise(), shero(), listen(), unsummon(), ungaze();

bool	blue_light(), can_blink(), shoot_bolt(), add_pack(),
	straight_shot(), maze_view(), is_carrying(), possessed();

void	byebye(), id_monst();

int checkout();

long lseek();

struct trap *trap_at();

extern struct h_list helpstr[];

#if defined(USGV5)
extern int _tty_ch;
#if !defined(attron)
extern SGTTY _tty;
#endif NOT attron
#endif

/*
 * Now all the global variables
 */

extern struct trap traps[];
extern struct room rooms[];		/* One for each room -- A level */
extern struct room *oldrp;		/* Roomin(&oldpos) */
extern struct linked_list *mlist;	/* List of monsters on the level */
extern struct thing player;		/* The rogue */
extern struct thing *beast;		/* The last monster attacking */
extern struct monster monsters[];	/* The initial monster states */
extern struct linked_list *lvl_obj;	/* List of objects on this level */
extern struct linked_list *familiar_ptr;/* A ptr to the familiar */
extern struct object *cur_weapon;	/* Which weapon he is weilding */
extern struct object *cur_armor;	/* What a well dresssed rogue wears */
extern struct object *cur_ring[];	/* Which rings are being worn */
extern struct magic_item things[];	/* Chances for each type of item */
extern struct magic_item s_magic[];	/* Names and chances for scrolls */
extern struct magic_item p_magic[];	/* Names and chances for potions */
extern struct magic_item r_magic[];	/* Names and chances for rings */
extern struct magic_item ws_magic[];	/* Names and chances for sticks */
extern struct magic_item fd_data[];	/* Names and chances for food */
extern struct spells magic_spells[];	/* spells for magicians */
extern struct spells cleric_spells[];	/* spells for clerics */
extern struct spells illusion_spells[];	/* spells for illusionists */
extern struct spells druid_spells[];	/* spells for druids */
extern struct real_pass rpass;		/* For protection's sake! */
extern char *cnames[][15];		/* Character level names */
extern char curpurch[];			/* name of item ready to buy */
extern char PLAYER;			/* what the player looks like */
extern int def_array[MAXPDEF][MAXPATT];	/* Pre-def'd chars */
extern int resurrect;			/* resurrection counter */
extern bool char_type;			/* what type of character is player */
extern int foodlev;			/* how fast he eats food */
extern int see_dist;			/* how far he can see^2 */
extern int level;			/* What level rogue is on */
extern int trader;			/* number of purchases */
extern int curprice;			/* price of an item */
extern int purse;			/* How much gold the rogue has */
extern int mpos;			/* Where cursor is on top line */
extern int ntraps;			/* Number of traps on this level */
extern int no_command;			/* Number of turns asleep */
extern int inpack;			/* Number of things in pack */
extern int total;			/* Total dynamic memory bytes */
extern int lastscore;			/* Score before this turn */
extern int no_food;			/* Number of levels without food */
extern int seed;			/* Random number seed */
extern int count;			/* Number of times to repeat command */
extern int dnum;			/* Dungeon number */
extern int max_level;			/* Deepest player has gone */
extern int food_left;			/* Amount of food in hero's stomach */
extern int group;			/* Current group number */
extern int hungry_state;		/* How hungry is he */
extern int infest_dam;			/* Damage from parasites */
extern int lost_str;			/* Amount of strength lost */
extern int lost_dext;			/* amount of dexterity lost */
extern int hold_count;			/* Number of monsters holding player */
extern int trap_tries;			/* Number of attempts to set traps */
extern int spell_power;			/* Spell power left at this level */
extern int auth_or[MAXAUTH];		/* MAXAUTH priviledged players */
extern int has_artifact;		/* set for possesion of artifacts */
extern int picked_artifact;		/* set for any artifacts picked up */
extern int msg_index;			/* pointer to current message buffer */
extern int luck;			/* how expensive things to buy thing */
extern int familiar_type;		/* Type of familiar */
extern int times_prayed;		/* The number of time prayed */
extern char take;			/* Thing the rogue is taking */
extern char prbuf[];			/* Buffer for sprintfs */
extern char outbuf[];			/* Output buffer for stdout */
extern char runch;			/* Direction player is running */
extern char *s_names[];			/* Names of the scrolls */
extern char *p_colors[];		/* Colors of the potions */
extern char *r_stones[];		/* Stone settings of the rings */
extern struct init_weps weaps[];	/* weapons and attributes */
extern struct init_armor armors[];	/* armors and attributes */
extern struct init_artifact arts[];	/* artifacts and attributes */
extern char *ws_made[];			/* What sticks are made of */
extern char *release;			/* Release number of rogue */
extern char whoami[];			/* Name of player */
extern char fruit[];			/* Favorite fruit */
extern char msgbuf[10][2*BUFSIZ];	/* message buffer */
extern char *s_guess[];			/* Players guess at what scroll is */
extern char *p_guess[];			/* Players guess at what potion is */
extern char *r_guess[];			/* Players guess at what ring is */
extern char *ws_guess[];		/* Players guess at what wand is */
extern char *ws_type[];			/* Is it a wand or a staff */
extern char file_name[];		/* Save file name */
extern char score_file[];		/* Score file name */
extern char home[];			/* User's home directory */
extern WINDOW *cw;			/* Window that the player sees */
extern WINDOW *hw;			/* Used for the help command */
extern WINDOW *mw;			/* Used to store mosnters */
extern bool pool_teleport;		/* just teleported from a pool */
extern bool inwhgt;			/* true if from wghtchk() */
extern bool running;			/* True if player is running */
extern bool fighting;			/* True if player is fighting */
extern bool playing;			/* True until he quits */
extern bool wizard;			/* True if allows wizard commands */
extern bool after;			/* True if we want after daemons */
extern bool fight_flush;		/* True if toilet input */
extern bool terse;			/* True if we should be short */
extern bool doorstop;			/* Stop running when we pass a door */
extern bool jump;			/* Show running as series of jumps */
extern bool door_stop;			/* Current status of doorstop */
extern bool firstmove;			/* First move after setting door_stop */
extern bool waswizard;			/* Was a wizard sometime */
extern bool canwizard;			/* Will be permitted to do this */
extern bool askme;			/* Ask about unidentified things */
extern bool moving;			/* move using 'm' command */
extern bool s_know[];			/* Does he know what a scroll does */
extern bool p_know[];			/* Does he know what a potion does */
extern bool r_know[];			/* Does he know what a ring does */
extern bool ws_know[];			/* Does he know what a stick does */
extern bool in_shell;			/* True if executing a shell */
extern bool monst_dead;			/* Indicates if monster got killed */
extern coord delta;			/* Change indicated to get_dir() */
extern bool inv_type;			/* Inven style. Bool so options works */
extern int line_cnt;			/* Counter for inventory style */
extern int newpage;
extern char *lastfmt;
extern char *lastarg;
extern char *spacemsg;
extern char *morestr;
extern char *retstr;
extern LEVTYPE levtype;
