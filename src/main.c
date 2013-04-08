#include "chokus.h"
#include "ck_config.h"

int main(int argc, char** argv)
{
    int ret = 0;
    
    ckConfig_ParseOpts(argc, argv);
    if ((ret = ckConfig_LoadConfig()) != 0)
    {
        ck_Err("Loading the config failed");
        return -1;
    }
    
    
	return 0;
}

