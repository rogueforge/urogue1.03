/*
 * Last Modified: 08/12/86
 */
/*
 * This file contains functions for casting magic spells
 */

#include "rogue.h"
#include "death.h"
#include <ctype.h>

/* fumble_table() return values */
#define CAST_CURSED 1		/* cast cursed version of spell */
#define CAST_NOT 2		/* do not cast any spell */
#define CAST_OK 3		/* go ahead, cast the spell */
#define CAST_DEAD 4		/* caster died during fumble */

#define MIN_FUMBLE_CHANCE	5
#define MAX_FUMBLE_CHANCE	95

struct spells {
    int sp_level;		/* level of casting spell */
    int sp_which;		/* which scroll or potion */
    int sp_flags;		/* scroll, blessed, known */
    int sp_cost;		/* generated in incant()  */
};

char *spell_name();
char *spell_abrev();
struct spells *pick_monster_spell();

/*
 * Cost for each level of spells
 * level:	     0  1  2   3   4   5   7    7
 */
int spell_cost[] = { 1, 3, 5, 11, 17, 29, 53, 101, 1000, 10000};

/*
 * Spells that a monster can cast
 */
struct spells monst_spells[] = {
#define		M_SELFTELEP	0
	{ 5,	S_SELFTELEP,	SCR_MAGIC},
#define		M_HEALING2	1
	{ 4,	P_HEALING,	POT_MAGIC|_TWO_},
#define		M_REGENERATE	2
	{ 3,	P_REGENERATE,	POT_MAGIC},
#define		M_HEALING	3
	{ 2,	P_HEALING,	POT_MAGIC},
#define 	NUM_RUN 	4

#define		M_HASTE 	4
	{ 4,	P_HASTE,	POT_MAGIC},
#define		M_SEEINVIS	5
	{ 2,	P_SEEINVIS,	POT_MAGIC},
#define		M_SHERO 	6
	{ 3,	P_SHERO,	POT_MAGIC},
#define		M_PHASE 	7
	{ 5,	P_PHASE,	POT_MAGIC},

#define		M_INVIS 	8
	{ 4,	P_INVIS,	POT_MAGIC},
#define		 M_CANCEL	9
	{ 4,	WS_CANCEL,	ZAP_MAGIC},

	/* In reverse order of damage ability */
#define		M_OFFENSE	10
	{ 6,	WS_ELECT,	ZAP_MAGIC},
	{ 6,	WS_FIRE,	ZAP_MAGIC},
	{ 6,	WS_COLD,	ZAP_MAGIC},
	{ 5,	WS_ELECT,	ZAP_MAGIC},
	{ 5,	WS_FIRE,	ZAP_MAGIC},
	{ 5,	WS_COLD,	ZAP_MAGIC},
	{ 5,	WS_MISSILE,	ZAP_MAGIC},
	{ 4,	WS_ELECT,	ZAP_MAGIC},
	{ 4,	WS_FIRE,	ZAP_MAGIC},
	{ 4,	WS_COLD,	ZAP_MAGIC},
	{ 3,	WS_MISSILE,	ZAP_MAGIC},
	{ 1,	WS_MISSILE,	ZAP_MAGIC},

	{ -1,	-1,		-1		},
};


/*
 * Spells that a player can cast
 * Non-mus only know ISKNOW spells until found in the dungeon
 * Special classes know their spells one level lower, and blessed one above
 */
struct spells player_spells[] = {
	{ 0,	S_SUMFAMILIAR,	SCR_MAGIC	|SP_ALL},

	{ 1,	P_MONSTDET,	POT_MAGIC	},
	{ 1,	S_GFIND,	SCR_MAGIC	},
	{ 1,	P_TREASDET,	POT_MAGIC	|ISKNOW},
	{ 1,	S_FOODDET,	SCR_MAGIC	|ISKNOW},
	{ 1,	S_LIGHT,	SCR_MAGIC	|SP_ILLUSION},

	{ 2,	S_IDENTIFY,	SCR_MAGIC	|ISKNOW},
	{ 2,	WS_CLOSE,	ZAP_MAGIC	|ISKNOW},
	{ 2,	WS_MISSILE,	ZAP_MAGIC	|ISKNOW|SP_MAGIC},
	{ 2,	P_SHIELD,	POT_MAGIC	|SP_MAGIC},
	{ 2,	P_SMELL,	POT_MAGIC	|SP_WIZARD|SP_DRUID},
	{ 2,	P_COLDRESIST,	POT_MAGIC	|SP_WIZARD|SP_DRUID},
	{ 2,	P_HEAR,		POT_MAGIC	|SP_ILLUSION|SP_CLERIC},
	{ 2,	S_CONFUSE,	SCR_MAGIC	|SP_ILLUSION},
	{ 2,	WS_HIT,		ZAP_MAGIC	|SP_PRAYER},

	{ 3,	WS_KNOCK,	ZAP_MAGIC	|ISKNOW},
	{ 3,	P_RESTORE,	POT_MAGIC	|ISKNOW|SP_PRAYER},
	{ 3,	P_CLEAR,	POT_MAGIC	|ISKNOW|SP_PRAYER},
	{ 3,	S_REMOVECURSE,	SCR_MAGIC	|ISKNOW|SP_PRAYER},
	{ 3,	WS_MONSTELEP,	ZAP_MAGIC	|SP_MAGIC},
	{ 3,	WS_CANCEL,	ZAP_MAGIC	|SP_WIZARD},
	{ 3,	P_FIRERESIST,	POT_MAGIC	|SP_WIZARD|SP_DRUID},
	{ 3,	S_SELFTELEP,	SCR_MAGIC	|SP_WIZARD},
	{ 3,	P_SEEINVIS,	POT_MAGIC	|SP_ILLUSION},
	{ 3,	S_MAP,		SCR_MAGIC	|SP_ILLUSION|SP_DRUID},
	{ 3,	S_HOLD,		SCR_MAGIC	|SP_CLERIC},
	{ 3,	S_SLEEP,	SCR_MAGIC	|SP_DRUID},
	{ 3,	P_HASOXYGEN,	POT_MAGIC	|SP_DRUID},
	{ 3,	WS_XENOHEALING,	ZAP_MAGIC	|SP_DRUID},
	{ 3,	P_HEALING,	POT_MAGIC	|SP_PRAYER},

	{ 4,	S_MSHIELD,	SCR_MAGIC	|ISKNOW|SP_ILLUSION},
	{ 4,	S_PROTECT,	SCR_MAGIC	|SP_MAGIC},
	{ 4,	P_INVIS,	POT_MAGIC	|SP_ILLUSION},
	{ 4,	S_ELECTRIFY,	SCR_MAGIC	|SP_ILLUSION},
	{ 4,	S_REFLECT,	SCR_MAGIC	|SP_ILLUSION},
	{ 4,	P_REGENERATE,	POT_MAGIC	|SP_PRAYER},
	{ 4,	WS_DRAIN,	ZAP_MAGIC	|SP_PRAYER},
	{ 4,	S_CURING,	SCR_MAGIC	|SP_PRAYER},
	{ 4,	P_HASTE,	POT_MAGIC	|SP_ILLUSION|SP_CLERIC},
	{ 4,	P_LEVITATION,	POT_MAGIC	|SP_WIZARD|SP_DRUID},
	{ 4,	WS_WEB,		ZAP_MAGIC	|SP_MAGIC},
	{ 4,	P_PHASE,	POT_MAGIC	},

	{ 5,	P_GAINABIL,	POT_MAGIC	|ISKNOW|SP_PRAYER},
	{ 5,	S_SCARE,	SCR_MAGIC|_TWO_	|SP_PRAYER},
	{ 5,	S_SILVER,	SCR_MAGIC	|SP_PRAYER},
	{ 5,	S_PETRIFY,	SCR_MAGIC	|SP_MAGIC},
	{ 5,	P_SHERO,	POT_MAGIC	|ISKNOW|SP_MAGIC},
	{ 5,	WS_COLD,	ZAP_MAGIC	|SP_DRUID},
	{ 5,	WS_FIRE,	ZAP_MAGIC	|SP_CLERIC},
	{ 5,	WS_ELECT,	ZAP_MAGIC	|SP_WIZARD},

	{ 6,	WS_DISINTEGRATE,ZAP_MAGIC	|ISKNOW},
	{ 6,	P_RAISELEVEL,	POT_MAGIC	|ISKNOW|SP_CLERIC},
	{ 6,	S_OWNERSHIP,	SCR_MAGIC	|SP_ALL},
	{ 6,	S_GENOCIDE,	SCR_MAGIC	|SP_DRUID},
	{ 6,	S_ENCHANT,	SCR_MAGIC	|SP_MAGIC},
	{ 6,	S_MAKEITEMEM,	SCR_MAGIC	|SP_WIZARD},
	{ 6,	WS_CHARGE,	ZAP_MAGIC	|SP_WIZARD},
	{ 6,	WS_ANTIMATTER,	ZAP_MAGIC	|SP_ILLUSION},

	{ -1,	-1,		-1		},
};

/*
 * Incantation:
 *   Cast a spell
 */
incant(caster, shoot_dir)
struct thing *caster;
coord shoot_dir;
{
    register int i;
    char ch;

    register struct spells *sp;
    register char *cast_name;			/* = spell_name(sp) */
    char *spell_type;				/* spell or prayer */
    int casting_cost;				/* from spell_cost[] */
    int spell_roll;				/* sucess/fail 1D100 die roll */
    int fumble_chance;				/* Spell fumble chance */
    int bless_or_curse = ISNORMAL;		/* blessed or cursed? */

    struct stats *curp = &(caster->t_stats);
    struct stats *maxp = &(caster->maxstats);
    bool is_player = (caster == &player);

    int casters_class;				/* For determining ISKNOW */
    int casters_stat;				/* s_intel or s_wisdom */
    int casters_level;				/* spellcasting level */
    int casters_points = curp->s_power;
    char *casters_name = (on(player, ISBLIND)) ? "it" : monsters[caster->t_index].m_name;

    char buf[2*LINELEN];

    if (casters_points <= 0) {
	if (is_player) {
	    msg("You don't have any spell points.");
	    after = FALSE;
	}
	return;
    }

    /*
     * Paladins, Rangers, ringwearers, and monsters cast at 4 levels below.
     * Other non-specialists at 8 below
     */
    casters_level = curp->s_lvl;
    switch (caster->t_ctype) {
    when C_PALADIN:
	casters_level -= 4;
    case C_CLERIC:
	casters_class = SP_CLERIC;
	casters_stat = curp->s_wisdom;
    when C_RANGER:
	casters_level -= 4;
    case C_DRUID:
	casters_class = SP_DRUID;
	casters_stat = curp->s_wisdom;
    when C_MAGICIAN:
	casters_class = SP_WIZARD;
	casters_stat = curp->s_intel;
    when C_ILLUSION:
	casters_class = SP_ILLUSION;
	casters_stat = curp->s_intel;
    when C_MONSTER:
	if (off(*caster, ISUNIQUE))
	    casters_level -= 4;
	casters_class = 0x0;
	casters_stat = curp->s_intel;
    otherwise:
	if (is_wearing(R_WIZARD)) {
	    casters_level -= 4;
	    casters_class = (rnd(4) ? SP_WIZARD : SP_ILLUSION);
	    casters_stat = curp->s_intel;
	}
	else if (is_wearing(R_PIETY)) {
	    casters_level -= 4;
	    casters_class = (rnd(4) ? SP_CLERIC : SP_DRUID);
	    casters_stat = curp->s_wisdom;
	}
	else  {
	    casters_level -= 8;
	    casters_class = 0x0;
	    casters_stat = (rnd(2) ? curp->s_wisdom : curp->s_intel);
	}
    }
    /* Bug - What about when WIS == INT? */
    spell_type = (casters_stat == curp->s_intel) ? "spell" : "prayer";

    if (!is_player) {
	if ( (sp = pick_monster_spell(caster)) == NULL)
	    return;
    }
    else {
	int num_spells = -1;			/* num of spells cheap enough */
	int num_blessed = -1;
	struct spells *castable[100];
	struct spells blessed_spells[50];

	castable[0] = castable[1] = NULL;

	for (sp=player_spells; sp->sp_level != -1; sp++) {
	    if (sp->sp_flags & casters_class) {	/* Does class know spell? */
		int spell_level = sp->sp_level - 1;
		int rnd_number = rnd(2*spell_level) - spell_level;

		/* Knows normal spell one level below others */
		casting_cost = spell_cost[spell_level] + rnd_number;
		if (casters_points >= casting_cost) {
		    castable[++num_spells] = sp;
		    sp->sp_cost = casting_cost;
		}

		/* Knows blessed spell one level above others */
		spell_level = sp->sp_level + 1;
		rnd_number = rnd(2*spell_level) - spell_level;
		casting_cost = spell_cost[spell_level] + rnd_number;
		if (casters_points >= casting_cost) {
		    sp->sp_cost = casting_cost;
		    blessed_spells[++num_blessed] = *sp;
		    blessed_spells[num_blessed].sp_level += 2;
		    blessed_spells[num_blessed].sp_flags |= ISBLESSED;
		}
	    }
	    /*
	     * If class doesn't know spell, see if its a ISKNOW
	     */
	    else if (sp->sp_flags & ISKNOW ||
		(is_wearing(R_WIZARD) && rnd(2)) ||
		(is_wearing(R_PIETY) && rnd(2)) ) {
		int spell_level = sp->sp_level;
		int rnd_number = rnd(2*spell_level) - spell_level;
		casting_cost = spell_cost[spell_level] + rnd_number;
		if (casters_points >= casting_cost) {
		    castable[++num_spells] = sp;
		    sp->sp_cost = casting_cost;
		}
	    }
	    /* else this spell is unknown */
	}

	/* castable[] and blessed_spells point to all cheap enough spells */
	if (castable[1] == NULL) {
	    msg("You don't have enough %s points.", spell_type);
	    after = FALSE;
	}

	do {					/* Prompt for spells */
	    struct spells *which_spell = NULL;
	    buf[0] = '\0';
	    msg("");				/* Get rid of --More-- */
	    msg("Which %s are you casting [%d points left] (* for list)? ",
		spell_type, casters_points);

	    switch (get_str(buf, cw)) {
	    when NORM:
	    when QUIT:				/* ESC */
		after = FALSE;
		return;
	    otherwise:
		continue;
	    }
	    if (buf[0] == '*') {		/* print list */
		add_line("Cost  Abbreviation	Full Name");
		for (i=0; i<=num_blessed; i++) {
		    sp = &blessed_spells[i];
		    sprintf(buf, "[%3d] %-12s\t%s",
			sp->sp_cost, spell_abrev(sp), spell_name(sp));
		    add_line(buf);
		}
		for (i=0; i<=num_spells; i++) {
		    sp = castable[i];
		    sprintf(buf, "[%3d] %-12s\t%s",
			sp->sp_cost, spell_abrev(sp), spell_name(sp));
		    add_line(buf);
		}
		end_line();
		sp = NULL;
		continue;
	    }
	    if (isupper( (int) buf[0])) {	/* Uppercase Abbreviation */
		for (i=0; i<=num_blessed; i++) {
		    sp = &blessed_spells[i];
		    if ((strcmp(spell_abrev(sp), buf) == 0)) {
			debug("matched #%d: '%s'.", i, spell_name(sp));
			which_spell = sp;
			goto found_one;
		    }
		}
		for (i=0; i<=num_spells; i++) {
		    sp = castable[i];
		    if ((strcmp(spell_abrev(sp), buf) == 0)) {
			debug("matched #%d: '%s'.", i, spell_name(sp));
			which_spell = sp;
			goto found_one;
		    }
		}
	    }
	    else {				/* Full Spell Name */
		for (i=0; i<=num_blessed; i++) {
		    sp = &blessed_spells[i];
		    if ((strcmp(spell_name(sp), buf) == 0)) {
			debug("matched #%d: '%s'.", i, spell_name(sp));
			which_spell = sp;
			goto found_one;
		    }
		}
		for (i=0; i<=num_spells; i++) {
		    sp = castable[i];
		    if ((strcmp(spell_name(sp), buf) == 0)) {
			debug("matched #%d: '%s'.", i, spell_name(sp));
			which_spell = sp;
			goto found_one;
		    }
		}
	    }
found_one:
	    sp = which_spell;
	}
	while (sp == NULL);

	/*
	 * Safely cast detects in Trading Posts
	 */ 
	if (levtype == POSTLEV && sp->sp_level != 1)
	    aggravate();
    }
    /*
     * Common monster and player code
     */
    cast_name = spell_name(sp);

    fumble_chance = (10*sp->sp_level/4 - 10*casters_level/13) * 5;
    if (cur_weapon!=NULL && wield_ok(&player, cur_weapon, FALSE) == FALSE) {
	switch(caster->t_ctype) {
	when C_MAGICIAN:
	case C_ILLUSION:
	    msg("You should have both hands free.");
	    fumble_chance += rnd(casters_level) * 5;
	when C_CLERIC:
	case C_DRUID:
	case C_PALADIN:
	    msg("Your god looks askance at the weapon you wield.");
	    fumble_chance += rnd(casters_level) * 5;
	otherwise:
	    break;
	}
    }

    if (fumble_chance >= MAX_FUMBLE_CHANCE)
	fumble_chance = MAX_FUMBLE_CHANCE;
    else if (fumble_chance <= MIN_FUMBLE_CHANCE + sp->sp_level)
	fumble_chance = MIN_FUMBLE_CHANCE + sp->sp_level;

    if (fumble_chance > (30 + rnd(50)) ) {
	if (is_player) {
	    char answer;
	    msg("Are you sure you want to try for that hard a %s? [n]",
			spell_type);
	    answer = readchar();
	    if (tolower(answer) != 'y') {
	        after = FALSE;
	        return;
	    }
	    else msg("Here goes...");
	}
	else {			/* Only if the monster is desperate */
	    if (curp->s_hpt > maxp->s_hpt/2)
		return;
	}
    }

    spell_roll = rnd(100);
    
    debug("%s(%d) cast '%s' fumble %%%d (rolled %d) ",
	    monsters[caster->t_index].m_name, curp->s_power, cast_name,
	    fumble_chance, spell_roll);
    caster->t_rest_hpt	= caster->t_rest_pow	= 0;
    if (!is_player) {			/* Stop running. */
	running = FALSE;
    	msg("The %s is casting '%s'.", casters_name, cast_name);
    }

    /* The Crown of Might insures that your spells never fumble */
    if (spell_roll < fumble_chance) {
	if (is_player && is_carrying(TR_CROWN))
	    msg("The Crown guides your %s.", spell_type);
	else {
	    if (is_player)
		msg("You botched your '%s' %s.", cast_name, spell_type);
	    else debug("Monster fumble '%s' spell.", cast_name);

	    curp->s_power -= min(curp->s_power, (2*sp->sp_cost) ); /* 2x cost */
	    switch(fumble_spell(caster, sp, fumble_chance - spell_roll)) {
	    when CAST_CURSED:
		bless_or_curse = ISCURSED;
	    when CAST_NOT:
		if (is_player)
		    msg("Your %s fails.", spell_type);
		return;
	    when CAST_OK:
		/* Bad things already happened in fumble_spell() */
		break;
	    when CAST_DEAD:
		return;
	    }
	}
    }
    else if (spell_roll > MAX_FUMBLE_CHANCE) {
	if (is_player) {
	    msg("Your '%s' %s goes superbly.", cast_name, spell_type);
	    pstats.s_exp += 3 * sp->sp_cost;
	    check_level();
	}
	maxp->s_power += sp->sp_cost;
	bless_or_curse = ISBLESSED;
    }
    else {
	if (is_player) {
	    msg("You sucessfully cast your '%s' %s.", cast_name, spell_type);
	    /* Spellcasters get bonus for sucessful spells */
	    if (player.t_ctype==C_MAGICIAN || player.t_ctype==C_ILLUSION) {
		pstats.s_exp += sp->sp_cost;
		check_level();
	    }

	}
	bless_or_curse = sp->sp_flags & ISBLESSED;
	curp->s_power -= sp->sp_cost;
   }

    /* The Septre of Might blesses all your spells */
    if (is_player && (bless_or_curse&ISBLESSED == 0) && is_carrying(TR_SCEPTRE)) {
	msg("The Septre sanctifies your %s.", spell_type);
	bless_or_curse = ISBLESSED;
    }

    switch (sp->sp_flags & TYP_MAGIC_MASK) {
    case POT_MAGIC:
	quaff(caster, sp->sp_which, bless_or_curse);
	break;
    case SCR_MAGIC:
	read_scroll(caster, sp->sp_which, bless_or_curse);
	break;
    case ZAP_MAGIC:
	if (is_player) {
	    do {				/* Must pick a direction */
		msg("Which direction?");
	    } while (get_dir() == FALSE);
	}
	else {
	    delta.x = shoot_dir.x;
	    delta.y = shoot_dir.y;
	}
	do_zap(caster, TRUE, sp->sp_which, bless_or_curse);
    otherwise:
	msg("What a strange %s!", spell_type);
    }
}

/*
 * spell_name - returns pointer to spell name
 */
char *
spell_name(sp)
struct spells *sp;
{
    static char spellname[2*BUFSIZ];
    char *ret_val;

    switch (sp->sp_flags & TYP_MAGIC_MASK) {
    when POT_MAGIC:
	ret_val = strcpy(spellname, p_magic[sp->sp_which].mi_name);
    when SCR_MAGIC:
	ret_val = strcpy(spellname, s_magic[sp->sp_which].mi_name);
    when ZAP_MAGIC:
	ret_val = strcpy(spellname, ws_magic[sp->sp_which].mi_name);
    otherwise:
	ret_val = strcpy(spellname, "unknown spell type");
    }
    if (sp->sp_flags & ISBLESSED)
	strcat(spellname, " 2");

    return(spellname);
}

/*
 * spell_abrev - returns pointer to capital letter spell abbreviation
 */
char *
spell_abrev(sp)
struct spells *sp;
{
    static char spellname[2*BUFSIZ];
    char *ret_val;

    switch (sp->sp_flags & TYP_MAGIC_MASK) {
    when POT_MAGIC:
	ret_val = strcpy(spellname, p_magic[sp->sp_which].mi_abrev);
    when SCR_MAGIC:
	ret_val = strcpy(spellname, s_magic[sp->sp_which].mi_abrev);
    when ZAP_MAGIC:
	ret_val = strcpy(spellname, ws_magic[sp->sp_which].mi_abrev);
    otherwise:
	ret_val = strcpy(spellname, "?????");
    }
    if (sp->sp_flags & ISBLESSED)
	strcat(spellname, " 2");

    return(spellname);
}

/*
 * fumble_spell - he blew it.  Make him pay
 */
fumble_spell(caster, sp, spell_roll)
register struct thing *caster;		/* the fumblee */
register struct spells *sp;		/* the botched spell */
int spell_roll;				/* high means you fumbled real bad */
{
    register struct stats *curp = &(caster->t_stats);
    register struct stats *maxp = &(caster->maxstats);
    int ret_val = CAST_OK;
    bool is_player = (caster == &player);
    int num_fumbles;

    
    num_fumbles = rnd((spell_roll/10) + 1) + rnd(sp->sp_level) + rnd(curp->s_lvl);
    num_fumbles = min(10, max(0, num_fumbles));
    debug("taking %d fumbles.", num_fumbles);

    switch(num_fumbles) {
    case 10:
	/* Lose ability */
	if (rnd(5) == 0)
	    quaff(caster, P_GAINABIL, ISCURSED);
    case 9:
	/* Lose max spell points */
	if (rnd(4) == 0) {
	    maxp->s_power -= rnd(10);
	    if (maxp->s_power <= 5)
		maxp->s_power = 5;
	}
    case 8:
	/* Lose all current spell points */
	if (rnd(3) == 0)
	     curp->s_power = 0;
	else curp->s_power /= 2;
    case 7:
	/* Freeze */
	if (rnd(2) == 0) {
	    if (is_player)
		 no_command++;
	    else caster->t_no_move++;
	}
    case 6:
	/* Cast a cursed spell - see below */
    case 5:
	/* Become dazed and confused */
	if (rnd(5) == 0)
	    quaff(caster, P_CLEAR, ISCURSED);
    case 4:
	/* Lose hit points */
	if (is_player)
	    feel_message();
	if ((curp->s_hpt -= rnd(2*sp->sp_level)) <= 0) {
	    if (is_player)
		 death(D_SPELLFUMBLE);
	    else killed(caster, find_mons(caster->t_pos.y, caster->t_pos.x),
			NOMESSAGE, NOPOINTS);
	    return(CAST_DEAD);
	}
    case 3:
	/* Spell fails */
	ret_val = CAST_NOT;
    case 2:
	/* Freeze */
	if (is_player)
	     no_command++;
	else caster->t_no_move++;
    default:
    case 1:
	/* Take double spell points - handed in incant() */
	break;
    }
    if (num_fumbles>=6 && rnd(1)==0)
	ret_val = CAST_CURSED;

    return(ret_val);
}

/*
 * learn_new_spells - go through player_spells and ISKNOW identified
 * potions, scrolls, and sticks
 */
learn_new_spells()
{
    register struct spells *sp;
    register bool know_spell;

    for (sp=player_spells; sp->sp_level != -1; sp++) {
	switch (sp->sp_flags & TYP_MAGIC_MASK) {
	when POT_MAGIC:
	    know_spell = p_know[sp->sp_which];
	when SCR_MAGIC:
	    know_spell = s_know[sp->sp_which];
	when ZAP_MAGIC:
	    know_spell = ws_know[sp->sp_which];
	}
	if (know_spell) {
	    if ((sp->sp_flags & ISKNOW) == FALSE)
		debug("Learned new spell '%s'", spell_name(sp));
	    sp->sp_flags |= ISKNOW;
	}
    }
}

/*
 * pick_monster_spell - decide which spell from monst_spells will be cast
 * returns pointer to spell in monst_spells
 */
struct spells *
pick_monster_spell(caster)
struct thing *caster;
{
    register int i;
    struct spells *sp = NULL;
    struct stats *curp = &(caster->t_stats);
    int casters_level = curp->s_lvl;
    int casters_stat = curp->s_intel;
    int casters_points = curp->s_power;

    /* Discover castable spells */
    for (sp=monst_spells; sp->sp_level != -1; sp++) {
	int spell_level = sp->sp_level;
	int rnd_number = rnd(2*spell_level) - spell_level;
	int casting_cost = spell_cost[spell_level] + rnd_number;

	if (casters_points >= casting_cost)
	     sp->sp_flags |= ISKNOW;
	else sp->sp_flags &= ~ISKNOW;
    }

    /*
     *  Decide which spell to cast
     */
    if (curp->s_hpt < rnd(caster->maxstats.s_hpt)) {	/* think defense */
	register int i;
	static int run_or_heal[NUM_RUN] =
		{ M_SELFTELEP, M_HEALING2, M_HEALING, M_REGENERATE };

	for (i=0; i<NUM_RUN; i++) {
	    sp = &monst_spells[run_or_heal[i]];
	    if ( (sp->sp_flags&ISKNOW) && rnd(1) )
		return(sp);
	}
    }

    if (on(*caster, ISSLOW)) {				/* cancel a slow */
	sp = &monst_spells[M_HASTE];
	if (sp->sp_flags & ISKNOW)
	    return(sp);
    }

    if (on(*caster, ISFLEE)) {				/* stop running away */
	sp = &monst_spells[M_SHERO];
	if (sp->sp_flags & ISKNOW)
	    return(sp);
    }

    if (on(player, ISINVIS) || on(player, ISDISGUISE)) {
	if (off(*caster, CANSEE)) {			/* look for him */
	    sp = &monst_spells[M_SEEINVIS];
	    if (sp->sp_flags & ISKNOW)
		return(sp);
	}
	else if (off(*caster, ISINVIS)) {		/* go invisible */
	    sp = &monst_spells[M_INVIS];
	    if (sp->sp_flags & ISKNOW)
		return(sp);
	}
    }

    if (on(player, CANINWALL) && (off(*caster, CANINWALL)) && rnd(2)) {
	sp = &monst_spells[M_PHASE];
	if (sp->sp_flags & ISKNOW)
	    return(sp);
    }

    /* Any flags added here must also be in player_powers[] */
    if (rnd(3)==0		&& on(player, HASOXYGEN) ||
	on(player, CANFLY)	|| on(player, CANINWALL) ||
	on(player, CANREFLECT)	|| on(player, CANSEE) ||
	on(player, HASMSHIELD)	|| on(player, HASSHIELD) ||
	on(player, ISHASTE)	|| on(player, ISREGEN) ||
	on(player, ISDISGUISE)	|| on(player, ISINVIS) ||
	on(player, NOCOLD)	|| on(player, NOFIRE) ||
	on(player, ISELECTRIC) ) {
	    sp = &monst_spells[M_CANCEL];
	    if (sp->sp_flags & ISKNOW)
		return(sp);
    }

    /* Cast an offensive spell */
    for (sp = &monst_spells[M_OFFENSE]; sp->sp_level != 1; sp++) {
	if ( (rnd(3) == 0) && (sp->sp_flags & ISKNOW) )
	    return(sp);
    }

    return(NULL);
}
