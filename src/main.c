#include "chokus.h"
#include "ck_config.h"

int main(int argc, char** argv)
{
    int ret = 0;
    
    gParseOpts(argc, argv);
    if ((ret = gLoadConfig()) != 0)
    {
        ck_err("Loading the config failed");
        return -1;
    }
    
    
	return 0;
}

