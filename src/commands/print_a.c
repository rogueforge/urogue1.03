/*
 * Last Modified: 05/25/86
 */
#include "rogue.h"

extern struct init_armor armors[MAXARMORS];

main()
{
    register int i;

    printf("Armor      %%  AC Weight Worth\n");

    for (i=0; i<MAXARMORS; i++) {

	printf("%15.15s ",
	    armors[i].a_name
	    );
	printf("%3d ",
	    armors[i].a_prob
	    );
	printf(" %2d ",
	    armors[i].a_class
	    );
	printf(" %3d ",
	    armors[i].a_wght
	    );
	printf(" %5d ",
	    armors[i].a_worth
	    );

	printf("\n");
    }
}
