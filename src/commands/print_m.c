/*
 * Last Modified: 05/25/86
 */
#include "/usr/include/curses.h"
#include "rogue.h"


		/*
		CARRY_PROB,	NORMAL,	WANDER,	SYMBOL,	INTELLIGENCE,
		{ATTRIBUTE_FLAGS},
		CREATURE_SUMMONED, NUMBER_SUMMONED,
		ADDED_EXPERIENCE/HIT_POINT,
		{str,	exp_pts, exp_level, armor_class, hit_points,
		"damage"}},
		*/

extern struct monster monsters[NUMMONST+2];

main()
{
    register int i;
    register int flag, j, k;

    printf("Monster                  Letter  Int Str  Lvl Exp(X)   Tres AC Flags\n\
\t\t\t\t\t\tHP      Damage\n");

    for (i=1; i<NUMMONST+2; i++) {
	printf("%25.25s '%c' %5s %2d %c",
	    monsters[i].m_name,	
	    monsters[i].m_appear,
	    monsters[i].m_intel,
	    monsters[i].m_stats.s_str,	
	    (monsters[i].m_wander ? 'W' : ' ')
	    );

	printf(" %2d ",
	    monsters[i].m_stats.s_lvl
	    );

	printf(" %5d",
	    monsters[i].m_stats.s_exp
	    );
	if (monsters[i].m_add_exp != 0)
	    printf("(%2d)",
		monsters[i].m_add_exp
	    );
	else printf("    ");

	if (monsters[i].m_carry != 0)
	    printf("%3d%%",
		monsters[i].m_carry
	    );
	else printf("    ");

	printf(" %2d ",
	    monsters[i].m_stats.s_arm
	    );

	find_flags(i);

	/* Second line */
	printf("\n\t\t\t\t\t\t");
	printf("%-5s %-10s",
	    monsters[i].m_stats.s_hpt,	
	    monsters[i].m_stats.s_dmg
	    );

	if (monsters[i].m_typesum != 0)
	    printf("\t%d(%s)",
	    monsters[i].m_numsum,
	    monsters[i].m_typesum
	    );
	printf("\n");
    }
}

find_flags(i)
register int i;
{
    register int k, flag;
    for (k=0; k<10; k++) {
	if ( (flag=monsters[i].m_flags[k]) != 0)
	{
	switch(flag) {
	case ISBLIND:	printf(" BLIND");	break;
	case ISINWALL:	printf(" INWALL");	break;
	case ISRUN:	printf(" RUN");	break;
	case ISFLEE:	printf(" FLEE");	break;
	case ISINVIS:	printf(" INVIS");	break;
	case ISMEAN:	printf(" M");	break;
	case ISGREED:	printf(" GREEDY");	break;
	case CANSHOOT:	printf(" SHOOT");	break;
	case ISHELD:	printf(" HELD");	break;
	case ISHUH:	printf(" HUH");	break;
	case ISREGEN:	printf(" REGEN");	break;
	case CANHUH:	printf(" HUH");	break;
	case CANSEE:	printf(" SEE");	break;
	case HASFIRE:	printf(" FIRE");	break;
	case ISSLOW:	printf(" SLOW");	break;
	case ISHASTE:	printf(" TE");	break;
	case ISCLEAR:	printf(" CLEAR");	break;
	case CANINWALL:	printf(" INWALL");	break;
	case ISDISGUISE:	printf(" DISGUISE");	break;
	case CANBLINK:	printf(" BLINK");	break;
	case CANSNORE:	printf(" SNORE");	break;
	case HALFDAMAGE:	printf(" HALFDAMAGE");	break;
	case CANSUCK:	printf(" SUCK");	break;
	case CANRUST:	printf(" RUST");	break;
	case CANPOISON:	printf(" POISON");	break;
	case CANDRAIN:	printf(" DRAIN");	break;
	case ISUNIQUE:	printf(" UNIQUE");	break;
	case STEALGOLD:	printf(" STEALGOLD");	break;
	case STEALMAGIC:	printf(" STEALMAGIC");	break;
	case CANDISEASE:	printf(" DISEASE");	break;
	case HASDISEASE:	printf(" DISEASE");	break;
	case CANSUFFOCATE:	printf(" SUFFOCATE");	break;
	case DIDSUFFOCATE:	printf(" DIDSUFFOCATE");	break;
	case BOLTDIVIDE:	printf(" BOLTDIVIDE");	break;
	case BLOWDIVIDE:	printf(" BLOWDIVIDE");	break;
	case NOCOLD:	printf(" NOCOLD");	break;
	case TOUCHFEAR:	printf(" TOUCHFEAR");	break;
	case BMAGICHIT:	printf(" M_+_HIT");	break;
	case NOFIRE:	printf(" NOFIRE");	break;
	case NOBOLT:	printf(" NOBOLT");	break;
	case CARRYGOLD:	printf(" CARRYGOLD");	break;
	case CANITCH:	printf(" ITCH");	break;
	case HASITCH:	printf(" ITCH");	break;
	case DIDDRAIN:	printf(" DIDDRAIN");	break;
	case WASTURNED:	printf(" WASTURNED");	break;
	case CANSELL:	printf(" SELL");	break;
	case CANBLIND:	printf(" BLIND");	break;
	case CANBBURN:	printf(" BBURN");	break;
	case ISCHARMED:	printf(" CHARMED");	break;
	case CANSPEAK:	printf(" SPEAK");	break;
	case CANFLY:	printf(" FLY");	break;
	case ISFRIENDLY:	printf(" FRIENDLY");	break;
	case CANHEAR:	printf(" HEAR");	break;
	case ISDEAF:	printf(" DEAF");	break;
	case CANSCENT:	printf(" SCENT");	break;
	case ISUNSMELL:	printf(" UNSMELL");	break;
	case WILLRUST:	printf(" RUST");	break;
	case WILLROT:	printf(" ROT");	break;
	case SUPEREAT:	printf(" SUPEREAT");	break;
	case PERMBLIND:	printf(" PERMBLIND");	break;
	case MAGICHIT:	printf(" M_HIT");	break;
	case CANINFEST:	printf(" INFEST");	break;
	case HASINFEST:	printf(" INFEST");	break;
	case NOMOVE:	printf(" NOMOVE");	break;
	case CANSHRIEK:	printf(" SHRIEK");	break;
	case CANDRAW:	printf(" DRAW");	break;
	case CANSMELL:	printf(" SMELL");	break;
	case CANPARALYZE:	printf(" PARALYZE");	break;
	case CANROT:	printf(" ROT");	break;
	case ISSCAVENGE:	printf(" SCAVENGE");	break;
	case DOROT:	printf(" ROT");	break;
	case CANSTINK:	printf(" STINK");	break;
	case HASSTINK:	printf(" STINK");	break;
	case ISSHADOW:	printf(" SHADOW");	break;
	case CANCHILL:	printf(" CHILL");	break;
	case CANHUG:	printf(" HUG");	break;
	case CANSURPRISE:	printf(" SURPRISE");	break;
	case CANFRIGHTEN:	printf(" FRIGHTEN");	break;
	case CANSUMMON:	printf("");	break;
	case TOUCHSTONE:	printf(" TOUCHSTONE");	break;
	case LOOKSTONE:	printf(" LOOKSTONE");	break;
	case CANHOLD:	printf(" HOLD");	break;
	case DIDHOLD:	printf(" DIDHOLD");	break;
	case DOUBLEDRAIN:	printf(" DOUBLEDRAIN");	break;
	case ISUNDEAD:	printf(" UNDEAD");	break;
	case BLESSMAP:	printf(" B_MAP");	break;
	case BLESSGOLD:	printf(" B_GOLD");	break;
	case BLESSMONS:	printf(" B_MONS");	break;
	case BLESSMAGIC:	printf(" B_MAGIC");	break;
	case BLESSFOOD:	printf(" B_FOOD");	break;
	case CANBRANDOM:	printf(" B_RANDOM");	break;
	case CANBACID:	printf(" B_ACID");	break;
	case CANBFIRE:	printf(" B_FIRE");	break;
	case CANBBOLT:	printf(" B_BOLT");	break;
	case CANBGAS:	printf(" B_GAS");	break;
	case CANBICE:	printf(" B_ICE");	break;
	case CANBPGAS:	printf(" B_PGAS");	break;
	case CANBSGAS:	printf(" B_SGAS");	break;
	case CANBSLGAS:	printf(" B_SLGAS");	break;
	case CANBFGAS:	printf(" B_FGAS");	break;
	case CANBREATHE:	printf(" BREATHE");	break;
	case STUMBLER:	printf(" STUMBLER");	break;
	case POWEREAT:	printf(" P_EAT");	break;
	case ISELECTRIC:	printf(" ELECTRIC");	break;
	case HASOXYGEN:	printf(" OXYGEN");	break;
	case POWERDEXT:	printf(" P_DEXT");	break;
	case POWERSTR:	printf(" P_STR");	break;
	case POWERWISDOM:	printf(" P_WISDOM");	break;
	case POWERINTEL:	printf(" P_INTEL");	break;
	case POWERCONST:	printf(" P_CONST");	break;
	case SUPERHERO:	printf(" SUPERHERO");	break;
	case ISUNHERO:	printf(" UNHERO");	break;
	case CANCAST:	printf(" CAST");	break;
	case CANTRAMPLE:	printf(" TRAMPLE");	break;
	case CANSWIM:	printf(" SWIM");	break;
	case LOOKSLOW:	printf(" LOOKSLOW");	break;
	case CANWIELD:	printf(" WIELD");	break;
	case CANDARKEN:	printf(" DARKEN");	break;
	case ISFAST:	printf(" FAST");	break;
	case CANBARGAIN:	printf(" BARGAIN");	break;
	case NOMETAL:	printf(" NOMETAL");	break;
	case CANSPORE:	printf(" SPORE");	break;
	case NOSHARP:	printf(" NOSHARP");	break;
	case DRAINWISDOM:	printf(" D_WISDOM");	break;
	case DRAINBRAIN:	printf(" D_BRAIN");	break;
	case ISLARGE:	printf(" LARGE");	break;
	case ISSMALL:	printf(" SMALL");	break;
	case CANSTAB:	printf(" STAB");	break;
	case ISFLOCK:	printf(" FLOCK");	break;
	case ISSWARM:	printf(" SWARM");	break;
	case CANSTICK:	printf(" STICK");	break;
	case CANTANGLE:	printf(" TANGLE");	break;
	case DRAINMAGIC:	printf(" D_ENCHANT");	break;
	case SHOOTNEEDLE:	printf(" S_NEEDLE");	break;
	case CANZAP:	printf(" ELECTRIC_ZAP");	break;
	case HASARMOR:	printf(" HASARMOR");	break;
	case CANTELEPORT:	printf(" TELEPORT");	break;
	default:
		printf(" ?????");	break;
	    }
	}
    }
}
