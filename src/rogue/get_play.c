/*
 * Last Modified: 07/09/86
 */
/*
 * Procedures for saving and retrieving a characters
 * starting attributes, armour, and weapon.
 * New addition to version 1.3 by S. A. Hester  11/8/83
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include "rogue.h"

#define	I_STR		0
#define	I_INTEL		1
#define	I_WISDOM	2
#define	I_DEXT		3
#define	I_CONST		4
#define	I_CHARISMA	5
#define	I_HPT		6
#define	I_POWER		7
#define	I_CTYPE		8
#define	MAXPATT		9	/* Total Number of above defines. */
#define	MAXPDEF		10	/* Maximum number of pre-defined chars */

static int def_array[MAXPDEF][MAXPATT];	/* Pre-def'd chars */

geta_player()
{

	char char_file[2*LINELEN];		/* Where the file should be! */
	struct	stat	stbuf, st2buf;
	int	fd;
	char	pbuf[2*LINELEN];
	short    cnt,i;

	strcpy(char_file,home);
	strcat(char_file, ROGDEFS);

	if ((fd = open(char_file, 0)) < 0)
	    return FALSE;

	fstat(fd, &stbuf);
	read(fd, (char *)&(st2buf.st_ino), sizeof(st2buf.st_ino));
	read(fd, (char *)&(st2buf.st_nlink), sizeof(st2buf.st_nlink));
	read(fd, (char *)&(st2buf.st_ctime), sizeof(st2buf.st_ctime));
	read(fd, (char *)&(st2buf.st_dev), sizeof(st2buf.st_dev));

	/*
	 * Now check to see if the player tried to alter it.
	 */
	if (stbuf.st_ino != st2buf.st_ino ||
	    stbuf.st_dev != st2buf.st_dev ||
	    stbuf.st_nlink != 1 ||
	    ((stbuf.st_ctime - st2buf.st_ctime) > 25 )) {
		return FALSE;
	}

	/*
	 * It's OK.  Get the info!
	 */
	encread((char *)def_array, sizeof(def_array), fd);
	close(fd);
 
	wclear(hw);
	touchwin(hw);

	cnt = 0;
	for(i=0; i<MAXPDEF; i++)
	    if(def_array[i][I_STR])
		cnt++;

	if(!cnt){
		mvwaddstr(hw,5,5,"You have no saved characters!");
		mvwaddstr(hw, LINES - 1, 0, spacemsg);
		wrefresh(hw);
		wait_for(' ');
		return FALSE;
	}

	sprintf(pbuf, "You have %d character%c: ",cnt,(cnt==1)? 0 : 's');
	mvwaddstr(hw,5,5,pbuf);
	for(i=0;i<MAXPDEF;i++)
	    if(def_array[i][I_STR]) {
		char	*class;
		switch(def_array[i][I_CTYPE]) {
		    when C_FIGHTER:
			class = "fighter";
		    when C_MAGICIAN:
			class = "magician";
		    when C_CLERIC:
			class = "cleric";
		    when C_THIEF:
			class = "thief";
		    when C_PALADIN:
		        class = "paladin";
		    when C_RANGER:
		        class = "ranger";
		    when C_DRUID:
		        class = "druid";
		    when C_ILLUSION:
		        class = "illusionist";
		    when C_ASSASIN:
		        class = "assasin";
		    when C_NINJA:
		        class = "ninja";
		    otherwise:
			class = "unknown";
		}
		sprintf(pbuf, "%d. (%s): Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d Pow: %d Hpt: %d",
			    i+1,
			  class,
		    def_array[i][I_INTEL],
		    def_array[i][I_STR],
		    def_array[i][I_WISDOM],
		    def_array[i][I_DEXT],
		    def_array[i][I_CONST],
		    def_array[i][I_CHARISMA],
		    def_array[i][I_POWER],
		    def_array[i][I_HPT]);
		mvwaddstr(hw,6+i,0,pbuf);
		
	    }
	mvwaddstr(hw,0,0,"Do you wish to select a character? ");
	wrefresh(hw);
	if ((getchar() & 0177) != 'y')	return FALSE;
again:
	wmove(hw, LINES - 1, 0);
	wclrtoeol(hw);
	mvwaddstr(hw,0,0,"Enter the number of a pre-defined character:");
	wclrtoeol(hw);
	wrefresh(hw);
	get_str(pbuf, hw);
	i = atoi(pbuf) - 1;
	while(i < 0 || i > MAXPDEF-1) {
	    mvwaddstr(hw,0,0,"Use the range 1 to");
	    wprintw(hw," %d!",MAXPDEF);
	    wclrtoeol(hw);
	    wrefresh(hw);
	    get_str(pbuf, hw);
	    i = atoi(pbuf) - 1;
	}
	if(def_array[i][I_STR] == 0) {
	    mvwaddstr(hw,0,0,"Please enter the number of a known character: ");
	    wclrtoeol(hw);
	    wstandout(hw);
	    mvwaddstr(hw, LINES - 1, 0, spacemsg);
	    wstandend(hw);
	    wrefresh(hw);
	    wait_for(' ');
	    goto again;
	}

	/* Set some global ones things */
	pstats.s_str = def_array[i][I_STR];
	pstats.s_intel = def_array[i][I_INTEL];
	pstats.s_wisdom = def_array[i][I_WISDOM];
	pstats.s_dext = def_array[i][I_DEXT];
	pstats.s_const = def_array[i][I_CONST];
	pstats.s_charisma = def_array[i][I_CHARISMA];
	max_stats.s_hpt = pstats.s_hpt = def_array[i][I_HPT];
	max_stats.s_power = pstats.s_power = def_array[i][I_POWER];
	char_type = def_array[i][I_CTYPE];
	player.t_ctype = def_array[i][I_CTYPE];
	return TRUE;
}


puta_player()
{

	char char_file[2*LINELEN];		/* Where the file should be! */
	register int	fd;
	struct	stat	stbuf;
	char	pbuf[2*LINELEN];
	char	*class;
	short   i;

	strcpy(char_file,home);
	strcat(char_file, ROGDEFS );

	wclear(hw);
	touchwin(hw);
	switch(player.t_ctype) {
	when C_FIGHTER:
	    class = "fighter";
	when C_MAGICIAN:
	    class = "magician";
	when C_CLERIC:
	    class = "cleric";
	when C_THIEF:
	    class = "thief";
	when C_PALADIN:
	    class = "paladin";
	when C_RANGER:
	    class = "ranger";
	when C_DRUID:
	    class = "druid";
	when C_ILLUSION:
	    class = "illusionist";
	when C_ASSASIN:
	    class = "assasin";
	when C_NINJA:
	    class = "ninja";
	otherwise:
	    class = "unknown";
	}

	wclear(hw);
	sprintf(pbuf, "You have a %s with the following attributes:",class);
	mvwaddstr(hw,2,0,pbuf);
	sprintf(pbuf, "Int: %d Str: %d Wis: %d Dex: %d Const: %d Charisma: %d Hpt: %d Pow: %d",
	    pstats.s_intel,
	    pstats.s_str,
	    pstats.s_wisdom,
	    pstats.s_dext,
	    pstats.s_const,
	    pstats.s_charisma,
	    pstats.s_hpt,
	    pstats.s_power);
	mvwaddstr(hw,4,0,pbuf);
	mvwaddstr(hw,0,0,"Would you like to save this character?");
	wclrtoeol(hw);
	wrefresh(hw);
	if((getchar() & 0177) != 'y')
	    return(TRUE);

	wstandout(hw);
	mvwaddstr(hw,8,0,"YOUR CURRENT CHARACTERS:");
	wstandend(hw);
	wclrtoeol(hw);
	for(i=0;i<MAXPDEF;i++) {
	    if(def_array[i][I_STR]) {
		switch(def_array[i][I_CTYPE]) {
		    when C_FIGHTER:
			class = "fighter";
		    when C_MAGICIAN:
			class = "magician";
		    when C_CLERIC:
			class = "cleric";
		    when C_THIEF:
			class = "thief";
		    when C_PALADIN:
		        class = "paladin";
		    when C_RANGER:
		        class = "ranger";
		    when C_DRUID:
		        class = "druid";
		    when C_ILLUSION:
		        class = "illusionist";
		    when C_ASSASIN:
		        class = "assasin";
		    when C_NINJA:
		        class = "ninja";
		    otherwise:
			class = "unknown";
		}
		sprintf(pbuf, "%d. (%s): Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d Pow: %d Hpt: %d",
			    i+1,
			  class,
		    def_array[i][I_INTEL],
		    def_array[i][I_STR],
		    def_array[i][I_WISDOM],
		    def_array[i][I_DEXT],
		    def_array[i][I_CONST],
		    def_array[i][I_CHARISMA],
		    def_array[i][I_POWER],
		    def_array[i][I_HPT]);
		mvwaddstr(hw,11+i,0,pbuf);
		
	    }
	    else {
		sprintf(pbuf, "%d.  ### NO CHARACTER DEFINED ###", i+1);
		mvwaddstr(hw,11+i,0,pbuf);
	    }
	}
	mvwaddstr(hw,0,0,"Enter a number which this character is to be saved as:");
	wrefresh(hw);
	get_str(pbuf, hw);
	i = atoi(pbuf) - 1;
	while(i < 0 || i > MAXPDEF-1) {
	    mvwaddstr(hw,0,0,"Use the range 1 to");
	    wprintw(hw," %d!",MAXPDEF);
	    wclrtoeol(hw);
	    wrefresh(hw);
	    get_str(pbuf, hw);
	    i = atoi(pbuf) - 1;
	}
	/* Set some global stuff */
	def_array[i][I_STR] = pstats.s_str;
	def_array[i][I_INTEL] = pstats.s_intel;
	def_array[i][I_WISDOM] = pstats.s_wisdom;
	def_array[i][I_DEXT] = pstats.s_dext;
	def_array[i][I_CONST] = pstats.s_const;
	def_array[i][I_CHARISMA] = pstats.s_charisma;
	def_array[i][I_HPT] = pstats.s_hpt;
	def_array[i][I_POWER] = pstats.s_power;
	def_array[i][I_CTYPE] = player.t_ctype;

	/* OK. Now let's write this stuff out! */
	if ((fd = creat(char_file, 0644)) < 0) {
	    sprintf(pbuf,"I can't seem to open/create %s", char_file);
	    mvwaddstr(hw,5,5,pbuf);
	    mvwaddstr(hw,6,5,"However I'll let you play it anyway!");
	    mvwaddstr(hw, LINES - 1, 0, spacemsg);
	    wrefresh(hw);
	    wait_for(' ');
	    return(TRUE);
	}
	fstat(fd, &stbuf);
	write(fd, (char *)&(stbuf.st_ino), sizeof(stbuf.st_ino));
	write(fd, (char *)&(stbuf.st_nlink), sizeof(stbuf.st_nlink));
	write(fd, (char *)&(stbuf.st_ctime), sizeof(stbuf.st_ctime));
	write(fd, (char *)&(stbuf.st_dev), sizeof(stbuf.st_dev));
	encwrite((char *)def_array, sizeof(def_array), fd);
	close(fd);
	return TRUE;
}

do_getplayer()
{
    if (char_type == C_NOTSET) {
	/* See what type character will be */
	wclear(hw) ;
	touchwin(hw) ;
	mvwaddstr(hw,2,0, "[a] Fighter\n[b] Paladin\n[c] Ranger\n[d] Cleric\n[e] Druid\n[f] Magician");
	waddstr(hw,"\n[g] Illusionist\n[h] Thief\n[i] Assasin\n[j] Ninja") ;
	mvwaddstr(hw, 0, 0, "What character class do you desire? ") ;
	wrefresh(hw) ;
	char_type = readchar() - 'a';
	while (char_type < C_FIGHTER || char_type >= C_MONSTER) {
	    mvwaddstr(hw, 0, 0, "Please enter a character in the displayed range: ");
	    wrefresh(hw);
	    char_type = readchar() - 'a';
	}
    }
    player.t_ctype = char_type ;
}
