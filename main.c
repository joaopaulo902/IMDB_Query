
#include "dbContext.h"
#include "sysManager.h"
#include <stdio.h>
#include <ctype.h>
int main(void) {
    char buffer[16];
    char cmd;
    do {
        printf("Enter your command:\n[G] Make New Request to API\n[I] Enter System\n");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        cmd = buffer[0];
        switch (toupper(cmd)) {
            case 'G':  make_titles_full_request();
                printf("\n");
                break;
            case 'I': initialize_system();
                return 0;
                break;
            default:
                continue;
        }
    }while (toupper(cmd) != 'L');


    //test_filter();
    //BP_tree_test();

    return 0;
}
