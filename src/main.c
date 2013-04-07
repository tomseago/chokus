#include "chokus.h"
#include "ck_config.h"

int main(int argc, char** argv)
{
    int ret = 0;
    
    ck_parseOpts(argc, argv);
    if ((ret = ck_loadConfig()) != 0)
    {
        ck_err("Loading the config failed");
        return -1;
    }
    
    
	return 0;
}

