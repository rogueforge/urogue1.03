/*
 * Last Modified: 05/25/86
 */
#include "curses.h"
#include "rogue.h"
/*
 * Now all the global variables
 */
struct trap traps[MAXTRAPS+MAXTRAPS];	/* twice as much for special effects */
struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room *oldrp;			/* Roomin(&player.t_oldpos) */
struct thing player;			/* The rogue */
struct thing *beast;			/* The last beast that attacked us */
struct object *cur_armor;		/* What a well dresssed rogue wears */
struct object *cur_ring[8];		/* Which rings are being worn */
struct linked_list *familiar_ptr = NULL;/* A ptr to the familiar */
struct linked_list *lvl_obj = NULL; 
struct linked_list *mlist = NULL;
struct object *cur_weapon = NULL;
struct real_pass rpass;			/* For protection's sake! */

#if defined(USGV5)
int _tty_ch;
#if !defined(attron)
SGTTY _tty;
#endif NOT attron
#endif

int def_array[MAXPDEF][MAXPATT];	/* Pre-def'd chars */
bool char_type = C_NOTSET;		/* what type of character is player */
int foodlev = 1;			/* how fast he eats food */
int ntraps;				/* Number of traps on this level */
int trader = 0;				/* no. of purchases */
int curprice = -1;			/* current price of item */
int seed;				/* Random number seed */
int dnum;				/* Dungeon number */
int max_level;				/* Deepest player has gone */
int lost_dext;				/* amount of lost dexterity */
int mpos = 0;
int no_command = 0;
int level = 1;
int purse = 0;
int inpack = 0;
int total = 0;
int see_dist = 3;
int no_food = 0;
int count = 0;
int food_left = HUNGERTIME;
int group = 1;
int hungry_state = F_OK;
int infest_dam=0;
int lost_str=0;
int lastscore = -1;
int hold_count = 0;
int trap_tries = 0;
int spell_power = 0;
int has_artifact = 0;
int picked_artifact = 0;
int msg_index = 0;
int luck = 0;
int resurrect = 0;
int auth_or[MAXAUTH];
bool inv_type = INV_OVER;		/* Overwrite style of inventory */
int line_cnt;				/* Inventory stuff */
int newpage;				/* Inventory stuff */
int familiar_type = 0;			/* The type of familiar */
int times_prayed = 0;			/* The number of times prayed */
char *lastfmt;
char *lastarg;
char curpurch[15];			/* name of item ready to buy */
char PLAYER = VPLAYER;			/* what the player looks like */
char take;				/* Thing the rogue is taking */
char prbuf[2*LINELEN];			/* Buffer for sprintfs */
char outbuf[BUFSIZ];			/* Output buffer for stdout */
char runch;				/* Direction player is running */
char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char *r_stones[MAXRINGS];		/* Stone settings of the rings */
char *ws_made[MAXSTICKS];		/* What sticks are made of */
char whoami[LINELEN];			/* Name of player */
char fruit[LINELEN];			/* Favorite fruit */
char msgbuf[10][2*BUFSIZ];		/* message buffer */
char *s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* Players guess at what potion is */
char *r_guess[MAXRINGS];		/* Players guess at what ring is */
char *ws_guess[MAXSTICKS];		/* Players guess at what wand is */
char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */
char file_name[LINELEN];		/* Save file name */
char score_file[LINELEN];		/* Score file name */
char home[LINELEN];			/* User's home directory */
WINDOW *cw;				/* Window that the player sees */
WINDOW *hw;				/* Used for the help command */
WINDOW *mw;				/* Used to store mosnters */
bool pool_teleport = FALSE;		/* just teleported from a pool */
bool inwhgt = FALSE;			/* true if from wghtchk() */
bool after;				/* True if we want after daemons */
bool waswizard;				/* Was a wizard sometime */
bool canwizard;				/* Will be permitted to do this */
bool s_know[MAXSCROLLS];		/* Does he know what a scroll does */
bool p_know[MAXPOTIONS];		/* Does he know what a potion does */
bool r_know[MAXRINGS];			/* Does he know what a ring does */
bool ws_know[MAXSTICKS];		/* Does he know what a stick does */
bool playing = TRUE; 
bool running = FALSE; 
bool fighting = FALSE; 
bool wizard = FALSE;
bool fight_flush = FALSE; 
bool terse = FALSE; 
bool door_stop = FALSE;
bool jump = FALSE; 
bool doorstop = TRUE;
bool firstmove = FALSE; 
bool askme = FALSE;
bool moving = FALSE;
bool in_shell = FALSE; 
bool monst_dead = FALSE;
coord delta;				/* Change indicated to get_dir() */
LEVTYPE levtype;			/* type of level i'm on */

char *spacemsg =	"--Press SPACE to continue--";
char *morestr  =	"--More--";
char *retstr   =	"[Press RETURN to continue]";

/*
 * weapons and their attributes
 * Average Damage = (min_damage + max_damage) / 2)
 * AD of 2D5+3 = (5 + 13) / 2 = 9
 * AD of 3D6   = (3 + 18) / 2 = 10.5
 */
struct init_weps weaps[MAXWEAPONS] = {
    /* Missile weapons */
    { "sling",		"0d0",  "0d0", NONE,  		ISLITTLE, 5, 1 },
    {   "rock",		"1d2",  "1d4", SLING,		ISMANY|ISMISL|ISLITTLE, 5, 1 },
    {   "sling bullet",	"1d1",	"1d8", SLING,		ISSHARP|ISMANY|ISMISL|ISMETAL|ISLITTLE, 3, 1 },
    { "short bow",	"1d1",  "1d1", NONE,		0, 40, 75 },
    {   "arrow",	"1d1",  "1d6", BOW,		ISSHARP|ISMANY|ISMISL|ISLITTLE, 5, 1 },
    {   "arrow",	"1d2",	"2d8", BOW,		ISSHARP|ISSILVER|ISMANY|ISMISL|ISLITTLE, 10, 3 },
    {   "fire arrow",	"1d2",	"2d8", BOW,		ISSHARP|CANBURN|ISMANY|ISMISL|ISLITTLE, 10, 3 },
    { "footbow",	"1d1",  "1d1", NONE,  		0, 90, 10 },
    {   "footbow bolt",	"1d2", "1d10", FOOTBOW,		ISSHARP|ISMANY|ISMISL|ISLITTLE, 5, 1 },
    { "crossbow",	"1d1",  "1d1", NONE,  		0, 100, 15 },
    {   "crossbow bolt","1d2", "1d12", CROSSBOW,	ISSHARP|ISMANY|ISMISL|ISLITTLE, 7, 1 },

    /* Useful throwing weapons */
    { "dart",		"1d1",  "1d3", NONE,  		ISSHARP|ISMANY|ISMISL|ISLITTLE, 5, 1 },
    { "dagger",		"1d6",  "1d4", NONE,		ISSHARP|ISMETAL|ISMANY|ISMISL|ISLITTLE, 10, 2 },
    { "hammer", 	"1d3",	"1d5", NONE,		ISMETAL|ISMISL, 50, 4 },
    { "leuku", 		"1d6",	"1d5", NONE,		ISSHARP|ISMETAL|ISBIG, 40, 3 },
    { "javelin", 	"1d4",	"1d6", NONE,		ISSHARP|ISMISL|ISBIG, 10, 3 },
    { "tomahawk", 	"1d6",	"1d6", NONE,		ISSHARP|ISMISL, 45, 7 },
    { "machete", 	"1d7",	"1d6", NONE,		ISSHARP|ISMETAL|ISMISL, 45, 4 },
    { "throwing axe",	"1d3",	"1d6+2", NONE,		ISSHARP|ISMETAL|ISMISL, 50, 8 },
    { "short spear",	"1d6",  "1d8", NONE,		ISSHARP|ISMISL, 50, 2 },
    { "boomerang", 	"1d1",	"1d8", NONE,		CANRETURN|ISMANY|ISMISL|ISLITTLE, 10, 3 },
    { "long spear",	"1d8", "1d10", NONE,		ISSHARP|ISMETAL|ISMISL|ISBIG, 50, 2 },
    { "shuriken", 	"1d1",	"2d5", NONE,		ISSHARP|ISMETAL|ISMANY|ISMISL|ISLITTLE, 4, 1 },
    { "burning oil",	"0d0",  "2d10+5",NONE,		CANBURN|ISMANY|ISMISL|ISLITTLE, 20, 3},
    { "grenade",	"1d1", "1d2/4d8", NONE,		ISMANY|ISSMALL, 10, 3 },
    { "club",	 	"1d4",	"1d2", NONE,		0, 30, 2 },
    { "pitchfork",	"1d5",  "2d2", NONE,		ISSHARP|ISMETAL, 15, 5 },
    { "short sword", 	"1d6",	"1d2", NONE,		ISSHARP|ISMETAL, 35, 8 },
    { "hand axe", 	"1d6",	"1d2", NONE,		ISSHARP|ISMETAL, 50, 8 },
    { "partisan", 	"1d6",	"1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 80, 4 },
    { "grain flail", 	"1d6",	"1d4", NONE,		ISSHARP|ISMETAL, 100, 10 },
    { "singlestick", 	"1d4+2","1d2", NONE,		0, 30, 2 },
    { "rapier", 	"1d6+1","1d2", NONE,		ISSHARP|ISMETAL, 7, 75 },
    { "sickle", 	"1d6+1","1d2", NONE,		ISSHARP|ISMETAL, 30, 15 },
    { "hatchet", 	"1d6+1","1d4", NONE,		ISSHARP|ISMETAL, 50, 10 },
    { "scimitar", 	"1d8",	"1d2", NONE,		ISSHARP|ISMETAL, 40, 8 },
    { "light mace",	"2d4",  "1d3", NONE,		0, 100, 8 },
    { "morning star", 	"2d4",	"1d3", NONE,		ISMETAL, 125, 8 },
    { "broad sword", 	"2d4",	"1d3", NONE,		ISSHARP|ISMETAL, 75, 15 },
    { "miner's pick",	"2d4",	"1d2", NONE,		ISSHARP|ISMETAL, 60, 4 },
    { "guisarme", 	"2d4",	"1d3", NONE,		ISSHARP|ISMETAL|ISBIG, 80, 6 },
    { "war flail", 	"1d6+2","1d4", NONE,		ISSHARP|ISMETAL|ISBIG, 150, 10 },
    { "crysknife",	"3d3",  "1d3", NONE,		ISSHARP|ISPOISON|ISMANY|ISLITTLE, 12, 2 },
    { "battle axe", 	"1d8+2","1d3", NONE,		ISSHARP|ISMETAL, 80, 10 },
    { "cutlass",	"1d10", "1d2", NONE,		ISSHARP|ISMETAL, 55, 12 },
    { "glaive", 	"1d10",	"1d3", NONE,		ISSHARP|ISMETAL|ISBIG, 80, 8 },
    { "pertuska",	"2d5",  "1d3", NONE,		ISSHARP|ISMETAL|ISBIG, 160, 10 },
    { "long sword",	"1d12", "1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 60, 15 },
    { "lance",		"1d12", "1d8", NONE,		ISSHARP|ISBIG, 80, 18 },
    { "short pike",	"1d12", "1d8", NONE,		ISSHARP|ISMETAL|ISBIG, 80, 18 },
    { "sabre",		"2d6",	"1d3", NONE,		ISSHARP|ISMETAL, 120, 20},
    { "spetum",		"2d6",  "1d3", NONE,		ISSHARP|ISMETAL|ISBIG, 50, 40 },
    { "halberd",	"2d6",  "1d3", NONE,		ISSHARP|ISMETAL|ISBIG, 175, 10 },
    { "trident",	"3d4",  "1d4", NONE,		ISSHARP|ISMETAL|ISBIG, 50, 40 },
    { "war pick",	"3d4",  "1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 75, 25 },
    { "bardiche",	"3d4",  "1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 125, 6 },
    { "heavy mace",	"3d4",  "1d3", NONE,		ISBIG, 100, 8 },
    { "great scythe", 	"2d6+2","1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 100, 8 },
    { "quarter staff", 	"3d5",	"1d2", NONE,		ISBIG, 70, 8 },
    { "bastard sword",	"2d8",  "1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 100, 20 },
    { "long pike", 	"2d8",	"2d6", NONE,		ISSHARP|ISMETAL|ISBIG, 80, 6 },
    { "two-handed flail","2d6+2","1d4", NONE,		ISSHARP|ISMETAL|ISBIG, 200, 30 },
    { "two-handed maul","4d4",  "1d3", NONE,		ISBIG, 200, 8 },
    { "two-handed pick","2d9",	"1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 175, 30 },
    { "two-handed sword","3d6",	"1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 250, 30 },
    { "claymore",	"3d7",  "1d2", NONE,		ISSHARP|ISMETAL|ISBIG, 200, 27 },
};
struct init_armor armors[MAXARMORS] = {
	{ "soft leather",		03,  9,   35,  50 },
	{ "cuirboilli",			06,  8,   50, 130 },
	{ "heavy leather",		10,  8,   70, 100 },
	{ "ring mail",			20,  7,   50, 250 },
	{ "studded leather",		31,  7,   50, 200 },
	{ "scale mail",			43,  6,   70, 250 },
	{ "padded armor",		55,  6,  150, 150 },
	{ "chain mail",			67,  5,  100, 300 },
	{ "brigandine",			75,  5,  200, 280 },
	{ "splint mail",		77,  4,  150, 350 },
	{ "banded mail",		87,  4,  150, 350 },
	{ "superior chain",	 	90,  3,  500, 350 },
	{ "plate mail",		 	93,  3,  400, 400 },
	{ "plate armor",		97,  2,  650, 450 },
	{ "mithril",			99,  2, 10000, 100 },
	{ "crystalline armor",		100, 0, 5500, 300 },
};

struct magic_item things[NUMTHINGS] = {
    { "potion",			250,	5 },	/* potion */
    { "scroll",			260,   30 },	/* scroll */
    { "food",			210,	7 },	/* food */
    { "weapon",			 60,	0 },	/* weapon */
    { "armor",			 90,	0 },	/* armor */
    { "ring",			 70,	5 },	/* ring */
    { "stick",			 60,	0 },	/* stick */
    { "artifact",		  0,    0 },    /* special artifacts */
};

struct magic_item s_magic[MAXSCROLLS] = {
    { "monster confusion",	 55, 125, 0, 0 },
    { "magic mapping",		 45, 150, 20, 20 },
    { "light",			 75, 100, 25, 15 },
    { "hold monster",		 25, 200, 33, 20 },
    { "sleep",			 25,  50, 20, 10 },
    { "enchantment",		110, 130, 14, 9 },
    { "identify",		150, 100, 0, 15 },
    { "scare monster",		 35, 190, 27, 21 },
    { "gold detection",		 15, 110, 20, 20 },
    { "teleportation",		 50, 165, 10, 20 },
    { "create monster",		 25,  75, 30, 0 },
    { "remove curse",		 60, 120, 9, 15 },
    { "petrification",		 10, 185, 0, 0 },
    { "genocide",		 10, 1200, 0, 0 },
    { "cure disease",		 70, 160, 0, 0 },
    { "acquirement",		  5, 2500, 50, 15 },
    { "protection",		 40, 150, 0, 0 },
    { "nothing",		 60, 50, 0, 0 },
    { "magic hitting",		 10, 875, 25, 15 },
    { "ownership",		 10, 550, 25, 15 },
    { "food detection",		 40, 150, 25, 15 },
    { "electrification",	 20, 450, 0, 0 },
    { "charm monster",		 10, 1500, 25, 15},
    { "summon monster",          10, 1500, 25, 15},
    { "gaze reflection",         35, 200, 25, 15}, 
};

struct magic_item p_magic[MAXPOTIONS] = {
    { "clear thought",		 90, 380, 27, 15 },
    { "gain ability",		 40, 250, 15, 15 },
    { "see invisible",		 64, 170, 25, 15 },
    { "healing",		140, 130, 27, 27 },
    { "monster detection",	 60, 120, 15, 20 },
    { "magic detection",	 60, 105, 20, 15 },
    { "raise level",		  1, 900, 11, 10 },
    { "haste self",		160, 200, 30, 5 },
    { "restore abilities",	200, 140, 0, 0 },
    { "phasing",		 45, 240, 21, 20 },
    { "invisibility",		 30, 200, 0, 15 },
    { "acute scent",		 30, 200, 20, 15 },
    { "acute hearing",		 30, 200, 20, 15 },
    { "super heroism",		 10, 800, 20, 15 },
    { "disguise",		 40, 500, 0, 15 },
};

struct magic_item r_magic[MAXRINGS] = {
    { "protection",		 70, 200, 33, 25 },
    { "add strength",		 65, 200, 33, 25 },
    { "sustain ability",	 40, 480, 0, 0 },
    { "searching",		 65, 250, 0, 0 },
    { "extra sight",		 40, 175, 0, 0 },
    { "alertness",		 40, 190, 0, 0 },
    { "aggravate monster",	 35, 100, 100, 0 },
    { "dexterity",		 65, 220, 33, 25 },
    { "increase damage",	 65, 220, 33, 25 },
    { "regeneration",		 35, 260, 0, 0 },
    { "slow digestion",		 40, 340, 15, 10 },
    { "teleportation",		 35, 100, 100, 0 },
    { "stealth",		 50, 100, 0, 0 },
    { "add intelligence",	 60, 540, 33, 25 },
    { "increase wisdom",	 60, 220, 33, 25 },
    { "sustain health",		 60, 250, 0,  0 },
    { "vampiric regeneration",	 20, 800, 25, 10 },
    { "illumination",		 20, 300, 0, 0 },
    { "delusion",		 20, 100, 75, 0 },
    { "carrying",		 20, 200, 30, 0 },
    { "adornment",		 15, 10000, 0, 0 },
    { "levitation",		 20, 450, 0, 0 },
    { "fire resistance",	 10, 250, 0, 0 },
    { "cold resistance",	 10, 250, 0, 0 },
    { "lightning resistance",	 10, 250, 0, 0 },
    { "resurrection",		  1, 8000, 0, 0 },
    { "breathing",		 10, 250, 0, 0 },
    { "free action",		 10, 225, 0, 0 },
    { "wizardry",		  4, 950, 0, 0 },
    { "teleport control",	  5, 850, 0, 0 },
};

struct magic_item ws_magic[MAXSTICKS] = {
    { "light",			 90, 120, 20, 20 },
    { "striking",		 58, 115, 0, 0 },
    { "lightning",		 25, 200, 0, 0 },
    { "fire",			 25, 200, 0, 0 },
    { "cold",			 30, 200, 0, 0 },
    { "polymorph",		 90, 210, 0, 0 },
    { "magic missile",		 90, 170, 0, 0 },
    { "slow monster",		 76, 320, 25, 20 },
    { "drain life",		 90, 210, 20, 0 },
    { "charging",		 70, 100, 0, 0 },
    { "teleport monster",	 90, 140, 25, 20 },
    { "cancellation",		 38, 130, 0, 0 },
    { "confuse monster",   	 50, 100, 0, 0},
    { "disintegration",	  	 10, 550, 33, 0},
    { "anti-matter",		 30, 180, 0, 0},
    { "paralyze monster",	 38, 300, 0, 0},
    { "degenerate monster",	 30, 200, 30, 0},
    { "nothing",		 30, 100, 0, 0},
    { "invisibility",		 30, 150, 30, 5},
    { "blasting",		 10, 220, 0, 0},
};

struct magic_item fd_data[MAXFOODS] = {
    { "food ration",					400, 2, 20, 20 },
    { "fruit-with-very-long-name-that-i-am-padding-out-to-be-greater-than-eighty-characters-so-that-a-test-in-options.c-will-work",			300, 1, 0, 0 },
    { "cram",						120, 3, 0, 0 },
    { "honey cake",					 80, 10, 0, 0 },
    { "lemba",						 50, 50, 0, 0 },
    { "miruvor",					 50, 200, 0, 0 },
};

struct init_artifact arts[MAXARTIFACT] = {
    { "Magic Purse of Yendor",	25, 1, 1, 1, 1, 460000, 50 },
    { "Phial of Galadriel",	35, 2, 2, 2, 1, 1250000, 10 },
    { "Amulet of Yendor",	45, 4, 1, 1, 2, 1600000, 10 },
    { "Palantir of Might",	60, 1, 4, 1, 2, 1850000, 70 },
    { "Crown of Might",		75, 6, 2, 1, 1, 2350000, 50 },
    { "Sceptre of Might",	80, 2, 2, 1, 6, 3800000, 50 },
    { "Silmaril of Ea",		90, 4, 2, 5, 1, 5000000, 50 },
    { "Wand of Orcus",		100, 4, 2, 3, 10, 8000000, 50 },
};

/*
 * these are the spells that a magician can cast
 */
struct spells magic_spells[MAXSPELLS_M] = {
	{ P_TFIND,		3,	TYP_POTION,	FALSE },
	{ S_IDENT,		5,	TYP_SCROLL,	FALSE },
	{ P_MFIND,		7,	TYP_POTION,	FALSE },
	{ S_LIGHT,		15,	TYP_SCROLL,	TRUE  },
	{ S_REMOVE,		15,	TYP_SCROLL,	FALSE },
	{ S_TELEP,		15,	TYP_SCROLL,	FALSE },
	{ S_CONFUSE,		15,	TYP_SCROLL,	FALSE },
	{ S_MAP,		15,	TYP_SCROLL,	FALSE },
	{ P_CLEAR,		20,	TYP_POTION,	FALSE },
	{ S_SLEEP,		25,	TYP_SCROLL,	FALSE },
	{ WS_MISSILE,		25,	TYP_STICK,	FALSE },
	{ P_SEEINVIS,		25,	TYP_POTION,	TRUE  },
	{ WS_COLD,		30,	TYP_STICK,	FALSE },
	{ WS_ELECT,		30,	TYP_STICK,	FALSE },
	{ WS_CANCEL,		30,	TYP_STICK,	FALSE },
	{ S_PROTECT,		30,	TYP_SCROLL,	FALSE },
	{ WS_FIRE,		30,	TYP_STICK,	FALSE },
	{ P_PHASE,		40,	TYP_POTION,	FALSE },
	{ S_HOLD,		50,	TYP_SCROLL,	FALSE },
	{ S_PETRIFY,		50,	TYP_SCROLL,	FALSE },
	{ S_ALLENCH,		75,	TYP_SCROLL,	FALSE },
};

/*
 * these are the spells that an illusionist can cast
 */
struct spells illusion_spells[MAXSPELLS_I] = {
	{ S_LIGHT,		3,	TYP_SCROLL,	TRUE  },
	{ S_IDENT,		5,	TYP_SCROLL,	FALSE },
	{ P_MFIND,		7,	TYP_POTION,	FALSE },
	{ S_CONFUSE,		15,	TYP_SCROLL,	FALSE },
	{ S_REMOVE,		15,	TYP_SCROLL,	FALSE },
	{ S_TELEP,		15,	TYP_SCROLL,	FALSE },
	{ P_DISGUISE,		15,	TYP_POTION,	FALSE },
	{ P_INVIS,		20,	TYP_POTION,	FALSE },
	{ P_SEEINVIS,		25,	TYP_POTION,	TRUE  },
	{ S_SLEEP,		25,	TYP_SCROLL,	FALSE },
	{ WS_MISSILE,		25,	TYP_STICK,	FALSE },
	{ WS_SLOW_M,		30,	TYP_STICK,	FALSE },
	{ S_REFLECT,		30,	TYP_SCROLL,	FALSE },
	{ S_ELECTRIFY,		30,	TYP_SCROLL,	FALSE },
	{ WS_CANCEL,		30,	TYP_STICK,	FALSE },
	{ S_PROTECT,		30,	TYP_SCROLL,	FALSE },
	{ WS_PARALYZE,		30,	TYP_STICK,	FALSE },
	{ P_PHASE,		40,	TYP_POTION,	FALSE },
	{ S_HOLD,		50,	TYP_SCROLL,	FALSE },
	{ S_PETRIFY,		50,	TYP_SCROLL,	FALSE },
	{ S_SCARE,		75,	TYP_SCROLL,	TRUE  },
};

/*
 * these are the spells that a cleric can cast
 */
struct spells cleric_spells[MAXPRAYERS_C] = {
	{ P_MFIND,		3,	TYP_POTION,	FALSE },
	{ P_TFIND,		7,	TYP_POTION,	FALSE },
	{ S_IDENT,		15,	TYP_SCROLL,	FALSE },
	{ S_REMOVE,		20,	TYP_SCROLL,	FALSE },
	{ P_HEALING,		30,	TYP_POTION,	FALSE },
	{ S_CURING,		35,	TYP_SCROLL,	FALSE },
	{ P_HASTE,		35,	TYP_POTION,	FALSE },
	{ P_CLEAR,		35,	TYP_POTION,	FALSE },
	{ P_SEEINVIS,		35,	TYP_POTION,	TRUE  },
	{ P_RESTORE,		35,	TYP_POTION,	FALSE },
	{ P_PHASE,		40,	TYP_POTION,	FALSE },
	{ S_TELEP,		40,	TYP_SCROLL,	FALSE },
};

/*
 * these are the spells that a druid can cast
 */
struct spells druid_spells[MAXPRAYERS_D] = {
	{ P_MFIND,		3,	TYP_POTION,	FALSE },
	{ P_TFIND,		7,	TYP_POTION,	FALSE },
	{ S_IDENT,		15,	TYP_SCROLL,	FALSE },
	{ S_REMOVE,		20,	TYP_SCROLL,	FALSE },
	{ P_HEALING,		30,	TYP_POTION,	FALSE },
	{ S_CURING,		35,	TYP_SCROLL,	FALSE },
	{ P_CLEAR,		35,	TYP_POTION,	FALSE },
	{ P_SEEINVIS,		35,	TYP_POTION,	TRUE  },
	{ P_RESTORE,		35,	TYP_POTION,	FALSE },
	{ S_TELEP,		40,	TYP_SCROLL,	FALSE },
	{ S_CHARM,     		40,     TYP_SCROLL,	FALSE },
	{ S_SUMMON,		50,	TYP_SCROLL,     FALSE },
};

char *cnames[10][15] = {
{	"Veteran",		"Warrior",
	"Swordsman",		"Hero",				/* Fighter */
	"Swashbuckler",		"Myrmidon",
	"Champion",		"Superhero",
	"Lord",			"Lord",
	"Lord",			"Lord",
	"Lord",			"Lord",
	"Lord"
},
{	"Gallant",		"Keeper",
	"Protector",		"Defender",			/* Paladin */
	"Warder",		"Guardian",
	"Chevalier",		"Justiciar",
	"Paladin",		"Paladin",
	"Paladin",		"Paladin",
	"Paladin",		"Paladin",
	"Paladin"
},
{	"Runner",		"Strider",
	"Scout",		"Courser",			/* Ranger */
	"Tracker",		"Guide",
	"Pathfinder",		"Ranger",
	"Ranger Knight",	"Ranger Lord",
	"Ranger Lord",		"Ranger Lord",
	"Ranger Lord",		"Ranger Lord",
	"Ranger Lord"
},
{	"Acolyte",		"Adept",
	"Priest",		"Curate",			/* Cleric */
	"Prefect",		"Canon",
	"Lama",			"Patriarch",
	"High Priest",		"High Priest",
	"High Priest",		"High Priest",
	"High Priest",		"High Priest",
	"High Priest"
},
{	"Aspirant",			"Ovate",		/* Druid */
	"Initiate of the 1st Circle",	"Initiate of the 2nd Circle",
	"Initiate of the 3rd Circle",	"Initiate of the 4th Circle",
	"Initiate of the 5th Circle",	"Initiate of the 6th Circle",
	"Initiate of the 7th Circle",	"Initiate of the 8th Circle",
	"Initiate of the 9th Circle",	"Druid",
	"Archdruid",			"The Great Druid",
	"The Grand Druid"
},

{	"Prestidigitator",	"Evoker",
	"Conjurer",		"Theurgist",			/* Magic User */
	"Thaumaturgist",	"Magician",
	"Enchanter",		"Warlock",
	"Sorcerer",		"Necromancer",
	"Wizard",		"Wizard",
	"Wizard",		"Wizard",
	"Wizard"
},
{	"Prestidigitator",	"Minor Trickster",
	"Trickster",		"Master Trickster",	       /* Illusionist */
	"Cabalist",		"Visionist",
	"Phantasmist",		"Apparitionist",
	"Spellbinder",		"Illusionist",
	"Illusionist",		"Illusionist",
	"Illusionist",		"Illusionist",
	"Illusionist"
},
{	"Rogue",		"Footpad",
	"Cutpurse",		"Robber",			/* Thief */
	"Burglar",		"Filcher",
	"Sharper",		"Magsman",
	"Thief",		"Master Thief",
	"Master Thief",		"Master Thief",
	"Master Thief",		"Master Thief",
	"Master Thief"
},
{	"Bravo",		"Rutterkin",
	"Waghalter",		"Murderer",			/* Assasin */
	"Thug",			"Killer",
	"Cutthroat",		"Executioner",
	"Assassin",		"Expert Assassin",
	"Senior Assassin",	"Chief Assassin",
	"Prime Assassin",	"Guildmaster Assassin",
	"Grandfather of Assassins"
},
{	"Ninja",		"Ninja",
	"Ninja",		"Ninja",			/* Ninja */
	"Ninja",		"Ninja",
	"Ninja",		"Ninja",
	"Ninja",		"Ninja",
	"Ninja",		"Ninja",
	"Ninja",		"Ninja",
	"Ninja"
}
} ;

struct h_list helpstr[] = {
    '?',	"	prints help",
    '/',	"	identify object",
    'h',	"	left",
    'j',	"	down",
    'k',	"	up",
    'l',	"	right",
    'y',	"	up & left",
    'u',	"	up & right",
    'b',	"	down & left",
    'n',	"	down & right",
    '<',	"SHIFT><dir> run that way",
    'm',	"<dir>	move onto without picking up",
    't',	"<dir>	throw something",
    'z',	"<dir>	zap a wand or staff",
    '>',	"	go down a staircase",
    's',	"	search for trap/secret door",
    '.',	"	rest for a while",
    'i',	"	inventory",
    'I',	"	inventory single item",
    'q',	"	quaff potion",
    'r',	"	read paper",
    'e',	"	eat food",
    'w',	"	wield a weapon",
    'W',	"	wear armor",
    'T',	"	take armor off",
    'P',	"	put on ring",
    'R',	"	remove ring",
    'A',	"       activate/apply an artifact",
    'd',	"	drop object",
    'c',	"	call object (generic)",
    'M',	"	mark object (specific)",
    'o',	"	examine/set options",
    'C',	"	cast a spell/say a prayer",
    'p',	"	pray to a deity",
    'a',	"	affect the undead",
    '^',	"	set a trap",
    'D',	"	dip something (into a pool)",
    CTRL(T),	"<dir>	take (steal) from (direction)",
    CTRL(R),	"	redraw screen",
    CTRL(P),	"	back up to 10 previous messages",
    ESCAPE,	"	cancel command",
    'v',	"	print program version number",
    '!',	"	shell escape",
    'S',	"	save game",
    'Q',	"	quit",
    '=',	"	listen for monsters",
    'f',	"<dir>	fight monster",
    'F',	"<dir>	fight monster to the death",
#if defined(BSD4)
    '&',	"	temporarily suspend game",
#endif
/*
 * Wizard commands.  Identified by (h_ch != 0 && h_desc == 0).
 */
	'-',		0,
	'V',		"	print worth of object",
	'@',		"	system statistics",
	CTRL(C),	"	charge item",
	CTRL(D),	"	random number check",
	CTRL(E),	"	food statistics",
	CTRL(F),	"	floor map",
	CTRL(G),	"	goto level",
	CTRL(I),	"	inventory level",
	CTRL(M),	"	see monster",
	CTRL(O),	"	outfit pack",
	CTRL(V),	"	create item",
	CTRL(W),	"	wizard's password",
	CTRL(X),	"	random teleport",
	CTRL(Z),	"	identify item",
	0,		0
};
