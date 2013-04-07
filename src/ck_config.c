//
//  ck_config.c
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#include "chokus.h"

#include <stdlib.h>
#include <argtable2.h>
#include <yaml.h>

#include "ck_log.h"


struct ck_opts {
    ck_bool foreground;
    bstring configFilename;
    
    int port;
    // Bind address....
    void* bindAddress;
};
typedef struct ck_opts ck_opts;

ck_opts gOptions;

typedef void* (*ck_yaml_event_handler)(yaml_event_t* event);



void* yehMappedKeywordsStart(yaml_event_t* event);
void* yehMappedKeywordsScalar(yaml_event_t* event);


void* yehStreamStart(yaml_event_t* event);


void* yehNetworkBindHost(yaml_event_t* event);
void* yehNetworkBindPort(yaml_event_t* event);

void* yehLoggingConsoleEnabled(yaml_event_t* event);
void* yehLoggingConsoleLevel(yaml_event_t* event);

void* yehLoggingSyslogEnabled(yaml_event_t* event);
void* yehLoggingSyslogLevel(yaml_event_t* event);

//void* yehDocument(yaml_event_t* event);
//void* yehDocumentScalar(yaml_event_t* event);
//
//void* yehNetworkStart(yaml_event_t* event);
//void* yehNetworkScalar(yaml_event_t* event);
//
//void* yehNetworkFiltersStart(yaml_event_t* event);
//void* yehNetworkFiltersScalar(yaml_event_t* event);
//
//
//void* yehLogging(yaml_event_t* event);
//void* yehOperations(yaml_event_t* event);

void usage(void* argtable[])
{
    if (argtable) {
        arg_print_syntax(stdout, argtable, "");
    }
}


void ck_parseOpts(int argc, char** argv)
{
    int nerrors = 0;
    
    // See http://argtable.sourceforge.net/doc/argtable2-intro.html
    
    struct arg_lit* debug = arg_litn("d", "debug", 0, 5, "Enable additional debugging levels");
    struct arg_lit* foreground = arg_lit0("f", "foreground", "Stay in the foreground and don't fork");
    
    struct arg_file* configFile = arg_file0("c", "config", "<config file>", "Name of file to read configuration from");
    
    struct arg_end* end = arg_end(20);
    
    void* argtable[] = {debug, foreground, configFile, end};
    
    if (arg_nullcheck(argtable))
    {
        printf("Error: No memory initializing argument table\n");
        exit(-1);
    }
    
    // Could initialize default values here ...
    
    // Parse
    nerrors = arg_parse(argc, argv, argtable);
    if (nerrors)
    {
        arg_print_errors(stderr, end, "chokus");
        
        usage(argtable);
        
        exit(-1);
    }
    
    // Configure the various things controllable from the command line
    gLogLevel = debug->count;
    gOptions.foreground = foreground->count;
    
    bdestroy(gOptions.configFilename);
    if (configFile->count)
    {
        gOptions.configFilename = bfromcstr(configFile->filename[configFile->count-1]);
    }
    else
    {
        gOptions.configFilename = NULL;
    }
    
    // Log everything
    ck_trace("debug level %d", gLogLevel);
    ck_trace("foreground = %s", STRING_FROM_BOOL(gOptions.foreground));
    if(gOptions.configFilename) 
    {
        ck_trace("config file = %s", gOptions.configFilename->data);
    }
    
}


int loadConfigFile(bstring filename)
{
    char* szFilename = NULL;
    FILE* fp = NULL;
    int ret = -1;
    
    yaml_parser_t parser = {0};
    yaml_event_t event = {0};
    ck_bool gotEndEvent = FALSE;
    ck_yaml_event_handler handler = &yehStreamStart;
    
    szFilename = bstr2cstr(filename, '_');
    if (!szFilename)
    {
        ck_err("Out of memory");
        goto end;
    }
    
    if (!yaml_parser_initialize(&parser))
    {
        ck_err("Failed to initialize the yaml parser object");
        goto end;
    }
    
    fp = fopen(szFilename, "r");
    if (!fp)
    {
        ck_debug("Failed to open %s", szFilename);
        goto end;
    }
    
    ck_info("Reading config from %s", szFilename);
    
    yaml_parser_set_input_file(&parser, fp);

    
    while(!gotEndEvent)
    {
        // Get the next event
        if (!yaml_parser_parse(&parser, &event))
        {
            ck_err("yaml_parser_parse returned an error");
            goto end;
        }
        
        if (event.type == YAML_STREAM_END_EVENT)
        {
            gotEndEvent = TRUE;
        }
        else
        {
            if (handler) handler = handler(&event);
        }
        
        yaml_event_delete(&event);
    }
    
    ck_debug("Parsing completed");
    
end:
    yaml_parser_delete(&parser);
    bcstrfree(szFilename);
    return ret;
}

/**
 * Wrapper to allow loading a constant named config file
 */
int loadConfigFileS(const char* filename)
{
    bstring name = bfromcstr(filename);
    int ret = loadConfigFile(name);
    bdestroy(name);
    return ret;
}

/**
 * Loads all configuration files, one after another
 */
int ck_loadConfig()
{
//    loadConfigFileS("/etc/chokus.yaml");
//    loadConfigFileS("/etc/chokus/config.yaml");
//    loadConfigFileS("/usr/local/chokus.yaml");
//    loadConfigFileS("/usr/local/chokus/config.yaml");
//    
    if (gOptions.configFilename) loadConfigFile(gOptions.configFilename);
    
    // TODO: Return an error I guess???
    return 0;
}


/*****************************************************************************/

void logMark(char* name, yaml_mark_t* mark)
{
    ck_debug("%s: %d (%d, %d)", name, mark->index, mark->line, mark->column);
}

void logEvent(yaml_event_t* event)
{
    switch(event->type)
    {
        default:
            // Intentional
            
        case YAML_NO_EVENT:
            ck_debug("No event");
            break;
            
        case YAML_STREAM_START_EVENT:
            ck_debug("YAML_STREAM_START_EVENT");
            break;
            
        case YAML_STREAM_END_EVENT:
            ck_debug("YAML_STREAM_END_EVENT");
            break;
            
        case YAML_DOCUMENT_START_EVENT:
            ck_debug("YAML_DOCUMENT_START_EVENT");
            break;
            
        case YAML_DOCUMENT_END_EVENT:
            ck_debug("YAML_DOCUMENT_END_EVENT");
            break;
            
        case YAML_ALIAS_EVENT:
            ck_debug("YAML_ALIAS_EVENT");
            break;
            
        case YAML_SCALAR_EVENT:
            ck_debug("YAML_SCALAR_EVENT = %s", event->data.scalar.value);
            break;
            
        case YAML_SEQUENCE_START_EVENT:
            ck_debug("YAML_SEQUENCE_START_EVENT");
            break;
            
        case YAML_SEQUENCE_END_EVENT:
            ck_debug("YAML_SEQUENCE_END_EVENT");
            break;
            
        case YAML_MAPPING_START_EVENT:
            ck_debug("YAML_MAPPING_START_EVENT");
            break;
            
        case YAML_MAPPING_END_EVENT:
            ck_debug("YAML_MAPPING_END_EVENT");
            break;     
    }
    
    logMark("start", &event->start_mark);
    logMark(" end ", &event->end_mark);
}



struct keyword_map
{
    struct tagbstring keyword;
    
    // If this handler is set it will be installed when this keyword is
    // discovered as a scaler
    ck_yaml_event_handler handler;
    
    // If the handler isn't set, but subMap is then it will get pushed
    // onto the keyword map stack
    struct keyword_map* subMap[];
};
typedef struct keyword_map keyword_map;

static keyword_map kmNetworkBindHost = { bsStatic("host"), yehNetworkBindHost, NULL};
static keyword_map kmNetworkBindPort = { bsStatic("port"), yehNetworkBindPort, NULL};

static keyword_map kmNetworkBind = { bsStatic("bind"), NULL, {&kmNetworkBindHost, &kmNetworkBindPort, NULL}};
static keyword_map kmNetworkFilters = { bsStatic("filters"), NULL, NULL};

static keyword_map kmNetwork = { bsStatic("network"), NULL, {&kmNetworkFilters, &kmNetworkBind, NULL} };

// logging -> console
static keyword_map kmLoggingConsoleEnabled = { bsStatic("enabled"), yehLoggingConsoleEnabled, NULL};
static keyword_map kmLoggingConsoleLevel = { bsStatic("level"), yehLoggingConsoleLevel, NULL};
static keyword_map kmLoggingConsole = { bsStatic("bind"), NULL, {&kmLoggingConsoleEnabled, &kmLoggingConsoleLevel, NULL}};

// logging -> syslog
static keyword_map kmLoggingSyslogEnabled = { bsStatic("enabled"), yehLoggingSyslogEnabled, NULL};
static keyword_map kmLoggingSyslogLevel = { bsStatic("level"), yehLoggingSyslogLevel, NULL};
static keyword_map kmLoggingSyslog = { bsStatic("bind"), NULL, {&kmLoggingSyslogEnabled, &kmLoggingSyslogLevel, NULL}};

// logging
static keyword_map kmLogging = { bsStatic("network"), NULL, {&kmLoggingSyslog, &kmLoggingConsole, NULL} };


static keyword_map* kmRootList[] = {&kmNetwork, &kmLogging, NULL};


typedef struct
{
    keyword_map** mapList;
    void* previous;
    
    const char* contextName;
    size_t startColumn;
    ck_yaml_event_handler higherLevelHandler;
} map_node;

static map_node* map_stack = NULL;

void pushMap(const char* name, keyword_map** list, ck_yaml_event_handler higherLevelHandler)
{
    map_node* pNode = NULL;
    
    pNode = malloc(sizeof(map_node));
    if (!pNode) {
        ck_err("Out of memory");
        exit(-1);
    }
    
    pNode->mapList = list;
    pNode->previous = map_stack;
    
    pNode->contextName = name;
    pNode->startColumn = 0;
    pNode->higherLevelHandler = higherLevelHandler;
    
    map_stack = pNode;
}

void popMap()
{
    map_node* toFree = NULL;
    
    if (!map_stack) return;
    
    toFree = map_stack;
    map_stack = map_stack->previous;
    
    free(toFree);
}

/**
 * The setup that should happen before this is set as the event handler is that
 * the new map should be pushed onto the stack. This will grab the first 
 * MAPPING_START event it sees and apply it's keywords at that level
 */
void* yehMappedKeywordsStart(yaml_event_t* event)
{
    if (!map_stack) {
        ck_err("yehMappedKeywordsStart called when map_stack was not defined. Returning to stream start");
        return yehStreamStart;
    }
    
    if (event->type != YAML_MAPPING_START_EVENT)
    {
        ck_err("%s: Was expecting a map start event but got something else instead", map_stack->contextName);
        return yehStreamStart;
    }
    
    map_stack->startColumn = event->start_mark.column;
    
    return yehMappedKeywordsScalar;
}

void* yehMappedKeywordsScalar(yaml_event_t* event)
{
    keyword_map** cursor = NULL;
    
    if (!map_stack) {
        ck_err("yehMappedKeywordsScalar called when map_stack was not defined. Returning to stream start");
        return yehStreamStart;
    }
    
    //ck_debug("map_stack->start=%d, e.start=%d", map_stack->startColumn, event->start_mark.column);

    if (event->start_mark.column == map_stack->startColumn)
    {
        if (event->type == YAML_SCALAR_EVENT)
        {
            ck_debug("Checking scalar %s", event->data.scalar.value);
            
            // We have a scalar at the current map level.
            // Let's see if we recognize what to do with this scalar
            bstring value = bfromcstr((char*)event->data.scalar.value);

            btrimws(value);
            btolower(value);
            
            cursor = map_stack->mapList;
            while(*cursor != NULL) {
                
                keyword_map* cv = *cursor;
                
                if (bstrcmp(&(cv->keyword), value) == 0)
                {
                    // We have a match batman!
                    
                    // Is there a handler? because if so, that's super easy, just set it.
                    // When it decides it doesn't want to do anything else, it should be smart
                    // enough to set the handler back to yehMappedKeywordScalar
                    if (cv->handler) {
                        bdestroy(value);
                        return cv->handler;
                    }
                    
                    // No handler, so maybe there is a submap we need to set?
                    if (cv->subMap) {
                        pushMap((char*)cv->keyword.data, cv->subMap, yehMappedKeywordsScalar);

                        bdestroy(value);
                        return yehMappedKeywordsStart;
                    }
                }
                
                // Increment
                cursor++;
            }

            // If we didn't return that means we didn't recognize this keyword
            ck_info("Ctx(%s) unrecognized keyword: %s", map_stack->contextName, bdata(value));
            
            bdestroy(value);
        }
    }
    else if (event->type == YAML_MAPPING_END_EVENT && event->start_mark.column < map_stack->startColumn)
    {
        // We are done with our mapping so pop up a level
        ck_yaml_event_handler next = map_stack->higherLevelHandler;
        
        popMap();
        //ck_debug("pop");
        return next;
    }
    // else, it was a lower level event that we probably got because something bailed in
    // a kind of weird fashion.
    
    return yehMappedKeywordsScalar;    
}


// At the base document node.


void* yehStreamStart(yaml_event_t* event)
{
    //    logEvent(event);
    
    // Ignore everything until a document start
    if (event->type == YAML_DOCUMENT_START_EVENT)
    {
        pushMap("Document", kmRootList, yehStreamStart);
        return yehMappedKeywordsStart;
    }
    else
    {
        return yehStreamStart;
    }
}

void* yehNetworkBindHost(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_info("bind = %s", event->data.scalar.value);
    }
    return yehMappedKeywordsScalar;
}

void* yehNetworkBindPort(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_info("port = %s", event->data.scalar.value);
    }
    return yehMappedKeywordsScalar;
    
}

ck_bool boolFromStr(yaml_char_t* str)
{
    char c;
    
    if (!str) return FALSE;
    
    c = str[0];
    return (c == 't' || c == 'T' || c == 'y' || c == 'Y');
}

ck_logLevel levelFromStr(yaml_char_t* str)
{
    char c;
    
    if (!str) return CK_LL_ERR;
    
    c = str[0];
    
    if (c >= 'a') c -= ('a' - 'A');
    
    switch (c) {
        case 'e':
            return CK_LL_ERR;
            
        case 'w':
            return CK_LL_WARN;

        case 'i':
            return CK_LL_INFO;
            
        case 'd':
            return CK_LL_DEBUG;
            
        case 't':
            return CK_LL_TRACE;
            
        default:
            break;
    }
    
    return CK_LL_ERR;
}

void* yehLoggingConsoleEnabled(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_enableConsole(boolFromStr(event->data.scalar.value));
    }
    return yehMappedKeywordsScalar;
    
}
void* yehLoggingConsoleLevel(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_setConsoleLevel(levelFromStr(event->data.scalar.value));
    }
    return yehMappedKeywordsScalar;
}

void* yehLoggingSyslogEnabled(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_enableSyslog(boolFromStr(event->data.scalar.value));
    }
    return yehMappedKeywordsScalar;
    
}
void* yehLoggingSyslogLevel(yaml_event_t* event)
{
    // Since we only expect one scalar just check for that and handle it
    if (event->type == YAML_SCALAR_EVENT)
    {
        ck_setSyslogLevel(levelFromStr(event->data.scalar.value));
    }
    return yehMappedKeywordsScalar;
}
