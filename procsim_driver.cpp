#include <cstdio>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "procsim.hpp"

FILE* inFile = stdin;

void print_help_and_exit(void) {
    printf("procsim [OPTIONS]\n");
    printf("  -j k0\t\tNumber of k0 FUs\n");
    printf("  -k k1\t\tNumber of k1 FUs\n");
    printf("  -l k2\t\tNumber of k2 FUs\n");
    printf("  -m M\t\tNumber of blocks per set is 2^S\n");
    printf("  -f N\t\tNumber of instructions to fetch\n");
    printf("  -d D\t\tDispatch Queue Multiplier\n");
    printf("  -i traces/file.trace\n");
    printf("  -h\t\tThis helpful output\n");
    exit(0);
}

//void print_statistics(proc_stats_t* p_stats);

int main(int argc, char* argv[]) {
    int opt;
    int f = DEFAULT_F;
    int m = DEFAULT_M;
    int k0 = DEFAULT_K0;
    int k1 = DEFAULT_K1;
    int k2 = DEFAULT_K2;
    int d = DEFAULT_D;

    /* Read arguments */ 
    while(-1 != (opt = getopt(argc, argv, "d:i:j:k:l:f:m:h"))) {
        switch(opt) {
        case 'd':
            d = atoi(optarg);
            break;
        case 'j':
            k0 = atoi(optarg);
            break;
        case 'k':
            k1 = atoi(optarg);
            break;
        case 'l':
            k2 = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
            break;
        case 'f':
            f = atoi(optarg);
            break;
        case 'i':
            inFile = fopen(optarg, "r");
            if (inFile == NULL)
            {
                fprintf(stderr, "Failed to open %s for reading\n", optarg);
                print_help_and_exit();
            }
            break;
        case 'h':
            /* Fall through */
        default:
            print_help_and_exit();
            break;
        }
    }

    dout("Processor Settings\n");
    dout("D: %i\n", d);
    dout("k0: %i\n", k0);
    dout("k1: %i\n", k1);
    dout("k2: %i\n", k2);
    dout("F: %i\n", f);
    dout("M: %i\n", m);
    dout("\n");

    /* Setup the processor */
    setup_proc(inFile, d, k0, k1, k2, f, m);

    /* Setup statistics */
    proc_stats_t stats;
    memset(&stats, 0, sizeof(proc_stats_t));

    /* Run the processor */
    run_proc(&stats);

    /* Finalize stats */
    complete_proc(&stats);

    print_statistics(&stats);

    return 0;
}
