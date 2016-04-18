#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL 4
#endif

#define DEBUG(...)      {if(DEBUG_LEVEL > 3) debug(__VA_ARGS__);}
#define NOTICE(...)     {if(DEBUG_LEVEL > 2) debug(__VA_ARGS__);}
#define WARNING(...)    {if(DEBUG_LEVEL > 1) debug(__VA_ARGS__);}
#define ERROR(...)      {if(DEBUG_LEVEL > 0) debug(__VA_ARGS__);}

#endif // __DEBUG_H__
