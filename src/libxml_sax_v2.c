#include <stdio.h>
#include <string.h>
#include <libxml/SAX.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUFFLEN 1024*1024*10

static int print_result = 1;
static int print_time = 1;
static int max_depth = 0;
static int max_nodes = 0;
static struct timespec ts_before, ts_after, ts_diff;

// https://gist.github.com/diabloneo/9619917#gistcomment-3364033
static inline void timespec_diff(struct timespec *a, struct timespec *b, struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

int read_xmlfile(FILE *f);
xmlSAXHandler make_sax_handler();

struct _node_data {
    char * node_element;
    char * node_value;
    int    node_depth;
    int    has_child;
    struct _node_data * next;
    struct _node_data * prev;
};

typedef struct _node_data node_data;

struct _xml_nodes {
    node_data    * start;
    node_data    * current;
    unsigned int depth;
    unsigned int pathlen;
    char         * currpath;
    size_t       currpathbufflen;
    unsigned int nrofnodes;
};

typedef struct _xml_nodes xml_nodes;

node_data * create_node_data() {
    node_data * nd;
    nd = malloc(sizeof(node_data));
    nd->node_element = NULL;
    nd->node_value = NULL;
    nd->node_depth = 0;
    nd->has_child  = 0;
    nd->next = NULL;
    nd->prev = NULL;
    return nd;
}

void delete_node_data(node_data * nd) {
    if (nd != NULL) {
        if (nd->node_element != NULL) {
            free(nd->node_element);
        }
        if (nd->node_value != NULL) {
            free(nd->node_value);
        }
        free(nd);
    }
}

static void OnStartElementNs(
    void *ctx,
    const xmlChar *localname,
    const xmlChar *prefix,
    const xmlChar *URI,
    int nb_namespaces,
    const xmlChar **namespaces,
    int nb_attributes,
    int nb_defaulted,
    const xmlChar **attributes
);

static void OnEndElementNs(
    void* ctx,
    const xmlChar* localname,
    const xmlChar* prefix,
    const xmlChar* URI
);

static void OnCharacters(void* ctx, const xmlChar * ch, int len);

xmlSAXHandler make_sax_handler (){
    xmlSAXHandler SAXHander;

    memset(&SAXHander, 0, sizeof(xmlSAXHandler));

    SAXHander.initialized = XML_SAX2_MAGIC;
    SAXHander.startElementNs = OnStartElementNs;
    SAXHander.endElementNs = OnEndElementNs;
    SAXHander.characters = OnCharacters;

    return SAXHander;
}

int read_xmlfile(FILE *f) {
    char chars[1024000];
    int res = fread(chars, 1, 4, f);
    if (res <= 0) {
        return 1;
    }

    xmlSAXHandler SAXHander = make_sax_handler();
 
    xml_nodes my_xml_node;
    // initialize xml_nodes struct
    my_xml_node.start           = NULL;
    my_xml_node.current         = NULL;
    my_xml_node.depth           = 0;
    my_xml_node.pathlen         = 0;
    my_xml_node.currpath        = NULL;
    my_xml_node.currpathbufflen = 0;
    my_xml_node.nrofnodes       = 0;

    xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt(
        &SAXHander, &my_xml_node, chars, res, NULL
    );

    ts_diff.tv_sec  = 0;
    ts_diff.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &ts_before);

    while ((res = fread(chars, 1, sizeof(chars), f)) > 0) {
        if(xmlParseChunk(ctxt, chars, res, 0)) {
            xmlParserError(ctxt, "xmlParseChunk");
            return 1;
        }
    }
    xmlParseChunk(ctxt, chars, 0, 1);

    clock_gettime(CLOCK_REALTIME, &ts_after);
    timespec_diff(&ts_after, &ts_before, &ts_diff);

    delete_node_data(my_xml_node.current);
    if (my_xml_node.currpath != NULL) {
        free(my_xml_node.currpath);
    }

    if (ctxt != NULL) {
        xmlFreeParserCtxt(ctxt);
    }
    xmlCleanupParser();

    return 0;
}

static void OnStartElementNs(
    void *ctx,
    const xmlChar *localname,
    const xmlChar *prefix,
    const xmlChar *URI,
    int nb_namespaces,
    const xmlChar **namespaces,
    int nb_attributes,
    int nb_defaulted,
    const xmlChar **attributes
) {

    node_data * nd = create_node_data();
    size_t taglen = strlen((const char *)localname);

    nd->node_element = malloc(taglen+1);

    strcpy(nd->node_element, (const char *)localname);

    xml_nodes * xml_data = (xml_nodes *)ctx;
    if (xml_data->start == NULL) {
        xml_data->start = nd;
    }
    else {
        nd->prev = xml_data->current;
        nd->prev->has_child = 1;
    }
    xml_data->current = nd;
    xml_data->depth++;
    if (max_depth > 0 && max_depth > xml_data->depth) {
        printf("Depth of XML tree reached the given maximum value (%d)\n", xml_data->depth);
        exit(1);
    }
    xml_data->pathlen += (taglen + 1);
    if (xml_data->pathlen > xml_data->currpathbufflen) {
        if (xml_data->currpathbufflen == 0) {
            xml_data->currpath = malloc(xml_data->pathlen);
            xml_data->currpath[0] = '\0';
        }
        else {
            // need +2 extra bytes for `.` `\0` string
            xml_data->currpath = realloc(xml_data->currpath, xml_data->pathlen + 2);
        }
        xml_data->currpathbufflen = xml_data->pathlen;
    }

    if (xml_data->depth > 1) {
        strcat(xml_data->currpath, ".");
    }
    strcat(xml_data->currpath, (char *)localname);
}

static void OnEndElementNs(
    void* ctx,
    const xmlChar* localname,
    const xmlChar* prefix,
    const xmlChar* URI
) {

    xml_nodes * xml_data = (xml_nodes *)ctx;
    node_data * current = xml_data->current;
    size_t taglen = strlen((const char *)localname);

    if (current->has_child == 0) {
        xml_data->nrofnodes++;
        if (max_nodes > 0 && xml_data->nrofnodes > max_nodes) {
            printf("Maximum number of tree nodes reached the given maximum value (%d)\n", xml_data->nrofnodes);
            exit(1);
        }
        else {
            if (print_result == 1) {
                printf("key: '%s', val: '%s'\n", xml_data->currpath, current->node_value);
            }
        }
    }

    xml_data->current = current->prev;
    xml_data->pathlen -= (taglen + 1);

    xml_data->currpath[(xml_data->pathlen > 0) ? xml_data->pathlen - 1 : 0] = '\0';

    xml_data->depth--;
    delete_node_data(current);
}

static void OnCharacters(void *ctx, const xmlChar *ch, int len) {
    xml_nodes * xml_data = (xml_nodes *)ctx;

    /*
        need to check the node_value; in some cases the previous element is the current
        eg
        <level1>
            <node1>content</node1>
            <node2>content</node2>
        </level1>
        in this case after the /node1 the current item is the level1 which already allocated
     */
    if (xml_data->current->node_value == NULL) {
        xml_data->current->node_value = malloc(len + 1);
        strncpy(xml_data->current->node_value, (const char *)ch, len);
        xml_data->current->node_value[len] = '\0';
    }
}

void showhelp(char * name) {
    printf("Use: %s [OPTIONS] XMLFILE\n\n", name);
    printf("OPTIONS:\n");
    printf("\t-h\tThis help\n");
    printf("\t-H\tHide key:value pairs, only print time\n");
    printf("\t-T\tHide time results, only print key:value pairs\n");
    printf("\t-d\tSet the max depth of XML tree; when the parser exceeds that, terminate\n");
    printf("\t-n\tSet the max number of nodes; when the parser exceeds that, terminate\n");
    printf("\n");
}


int main(int argc, char *argv[]) {

    char c;

    while ((c = getopt (argc, argv, "hHTd:n:")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return EXIT_SUCCESS;
            case 'H':
                print_result = 0;
                break;
            case 'T':
                print_time = 0;
                break;
            case 'd':
                max_depth = atoi(optarg);
                break;
            case 'n':
                max_nodes = atoi(optarg);
                break;
            default:
                break;
        }
    }

    if (argc < 2) {
        puts("Argument missing");
        exit(1);
    }
    FILE *f = fopen(argv[argc-1], "r");
    if (!f) {
        puts("file open error.");
        exit(1);
    }

    if(read_xmlfile(f)) {
        puts("xml read error.");
        exit(1);
    }

    fclose(f);

    if (print_time == 1) {
        printf("Total time: %lf sec\n", ts_diff.tv_sec + (ts_diff.tv_nsec/1000000000.0));
    }
    return 0;
}

