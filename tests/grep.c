#include "dynArray.h"
#include "dynString.h"

#include <stdio.h>
#include <stdlib.h>

typedef enum dynRegexTokenType
{
    DRTT_EOF = 0,
    DRTT_CHAR,
    DRTT_OR,
    DRTT_ZERO_OR_MORE,
    DRTT_ONE_OR_MORE,
    DRTT_CAPTURE_BEGIN,
    DRTT_CAPTURE_END,
    DRTT_CC_BEGIN,
    DRTT_CC_END,

    DRTT_COUNT
} dynRegexTokenType;

typedef enum dynRegexParseFlags
{
    DRPF_OR = (1 << 0),

    DRPF_COUNT
} dynRegexParseFlags;

typedef enum dynRegexParseResult
{
    DRPR_OK = 0,
    DRPR_EARLY_EOF,
    DRPR_INVALID_CC,
    DRPR_INVALID_MORE,

    DRPR_COUNT
} dynRegexParseResult;

struct dynRegexNode;
struct dynRegexLink;
struct dynRegex;

typedef struct dynRegexNode
{
    struct dynRegexLink **links;
    int final;
    int generation; // just for debugging
} dynRegexNode;

typedef enum dynRegexLinkType
{
    DRLT_BLANK = 0,
    DRLT_CHAR,

    DRLT_COUNT
} dynRegexLinkType;

typedef struct dynRegexCharRange
{
    int c1;
    int c2;
} dynRegexCharRange;

dynRegexCharRange * dynRegexCharRangeCreate(int c1, int c2)
{
    dynRegexCharRange *range = (dynRegexCharRange *)calloc(1, sizeof(dynRegexCharRange));
    range->c1 = c1;
    range->c2 = c2;
    return range;
}

void dynRegexCharRangeDestroy(dynRegexCharRange *range)
{
    free(range);
}

typedef struct dynRegexLink
{
    int type;
    int not;
    dynRegexCharRange **ranges;
    struct dynRegexNode *src;
    struct dynRegexNode *dst;
} dynRegexLink;

typedef struct dynRegex
{
    dynRegexNode *start;
    dynRegexNode *end;
    dynRegexNode **nodes;
    dynRegexLink **links;
} dynRegex;

typedef struct regexToken
{
    int type;
    int c;
} regexToken;

typedef struct regexParser
{
    const char *input;
    const char *curr;
} regexParser;

static dynRegexNode *dynRegexNodeCreate(struct dynRegex *r, int generation)
{
    dynRegexNode *n = (dynRegexNode *)calloc(1, sizeof(dynRegexNode));
    n->generation = generation;
    daPush(&r->nodes, n);
    return n;
}

static void dynRegexNodeDestroy(struct dynRegexNode *n)
{
    daDestroy(&n->links, NULL);
    free(n);
}

static dynRegexLink *dynRegexLinkCreate(struct dynRegex *r, dynRegexLinkType type, dynRegexNode *src, dynRegexNode *dst)
{
    dynRegexLink *l = (dynRegexLink *)calloc(1, sizeof(dynRegexLink));
    l->type = type;
    l->src = src;
    l->dst = dst;
    daPush(&r->links, l);
    daPush(&src->links, l);
    return l;
}

static void dynRegexLinkDestroy(dynRegexLink *l)
{
    daDestroy(&l->ranges, dynRegexCharRangeDestroy);
    free(l);
}

static void dynRegexLinkAddRange(dynRegexLink *l, int c1, int c2)
{
    daPush(&l->ranges, dynRegexCharRangeCreate(c1, c2));
}

static regexToken regexLex(regexParser *parser)
{
    regexToken token;
    if(*parser->curr)
    {
        switch(*parser->curr)
        {
            case '|':
                {
                    token.type = DRTT_OR;
                    ++parser->curr;
                }
                break;

            case '*':
                {
                    token.type = DRTT_ZERO_OR_MORE;
                    ++parser->curr;
                }
                break;

            case '+':
                {
                    token.type = DRTT_ONE_OR_MORE;
                    ++parser->curr;
                }
                break;

            case '(':
                {
                    token.type = DRTT_CAPTURE_BEGIN;
                    ++parser->curr;
                }
                break;

            case ')':
                {
                    token.type = DRTT_CAPTURE_END;
                    ++parser->curr;
                }
                break;

            case '[':
                {
                    token.type = DRTT_CC_BEGIN;
                    ++parser->curr;
                }
                break;

            case ']':
                {
                    token.type = DRTT_CC_END;
                    ++parser->curr;
                }
                break;

            default:
                {
                    token.type = DRTT_CHAR;
                    token.c = *parser->curr;
                    ++parser->curr;
                }
                break;
        }
    }
    else
    {
        token.type = DRTT_EOF;
    }
    return token;
}

#if 0
static dynRegexNode * appendCharacterLink(dynRegex *dr, dynRegexNode *startNode, dynRegexNode *endNode, int charRangeBegin, int charRangeEnd, int not)
{
    dynRegexNode *s = dynRegexNodeCreate(dr);
    dynRegexLinkCreate(dr, DRLT_BLANK, startNode, s);
    dynRegexLinkAddRange(dynRegexLinkCreate(dr, DRLT_CHAR, s, endNode), charRangeBegin, charRangeEnd);
    
    if(not)
    {
        return s;
    }
    return startNode;
}
#endif

// Returns the "end" node
static dynRegexParseResult dynRegexParse(dynRegex *dr, regexParser *parser, dynRegexNode *startNode, dynRegexNode *endNode, dynRegexTokenType endType, int flags, int generation)
{
    int done = 0;
    dynRegexNode *appendNode = startNode;
    dynRegexNode *lastStart = NULL;
    dynRegexNode *lastEnd = NULL;
    ++generation;
    while(!done)
    {
        dynRegexNode *s = NULL;
        dynRegexNode *e = NULL;
        regexToken token = regexLex(parser);
        if((token.type == DRTT_EOF) || (token.type == endType))
        {
            if(token.type != endType)
            {
                // a premature end to the regex, bail out
                return DRPR_EARLY_EOF;
            }
            break;
        }

        switch(token.type)
        {
            case DRTT_OR:
                {
                    dynRegexParseResult result = dynRegexParse(dr, parser, startNode, endNode, endType, flags, generation);
                    if(result != DRPR_OK)
                    {
                        return result;
                    }

                    done = 1;
                }
                break;

            case DRTT_ZERO_OR_MORE:
            case DRTT_ONE_OR_MORE:
                {
                    if(!lastStart || !lastEnd)
                    {
                        return DRPR_INVALID_MORE;
                    }

                    if(token.type == DRTT_ZERO_OR_MORE)
                    {
                        dynRegexLinkCreate(dr, DRLT_BLANK, lastStart, lastEnd);
                    }
                    dynRegexLinkCreate(dr, DRLT_BLANK, lastEnd, lastStart);
                }
                break;

            case DRTT_CC_BEGIN:
                {
                    int c = 0;
                    int range = 0;
                    int not = 0;
                    int first = 1;

                    dynRegexLink *cc;
                    s = dynRegexNodeCreate(dr, generation);
                    e = dynRegexNodeCreate(dr, generation);
                    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, s);
                    cc = dynRegexLinkCreate(dr, DRLT_CHAR, s, e);
                    appendNode = e;

                    while(token.type != DRTT_CC_END)
                    {
                        token = regexLex(parser);
                        if(token.type == DRTT_EOF)
                        {
                            return DRPR_EARLY_EOF;
                        }
                        if(token.type == DRTT_CC_END)
                        {
                            if(c)
                            {
                                // clean up leftover character class stuff here
                                dynRegexLinkAddRange(cc, c, 0);
                            }
                            break;
                        }
                        if(token.type != DRTT_CHAR)
                        {
                            return DRPR_INVALID_CC;
                        }

                        // handle DRTT_CHAR
                        if(token.c == '^')
                        {
                            if(first)
                            {
                                cc->not = 1;
                            }
                            else
                            {
                                dynRegexLinkAddRange(cc, token.c, 0);
                            }
                        }
                        else if(token.c == '-')
                        {
                            if(c)
                            {
                                range = 1;
                            }
                            else
                            {
                                dynRegexLinkAddRange(cc, token.c, 0);
                            }
                        }
                        else
                        {
                            if(range)
                            {
                                dynRegexLinkAddRange(cc, c, token.c);
                                range = 0;
                                c = 0;
                            }
                            else
                            {
                                if(c)
                                {
                                    dynRegexLinkAddRange(cc, c, 0);
                                }
                                c = token.c;
                            }
                        }
                        first = 0;
                    }
                }
                break;

            case DRTT_CAPTURE_BEGIN:
                {
                    s = dynRegexNodeCreate(dr, generation);
                    e = dynRegexNodeCreate(dr, generation);
                    dynRegexParseResult result = dynRegexParse(dr, parser, s, e, DRTT_CAPTURE_END, flags, 0);
                    if(result != DRPR_OK)
                    {
                        return result;
                    }
                    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, s);
                    appendNode = e;
                }
                break;

            case DRTT_CHAR:
                {
                    s = dynRegexNodeCreate(dr, generation);
                    e = dynRegexNodeCreate(dr, generation);
                    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, s);
                    dynRegexLinkAddRange(dynRegexLinkCreate(dr, DRLT_CHAR, s, e), token.c, 0);
                    appendNode = e;
                }
                break;
        };

        lastStart = s;
        lastEnd = e;
    }
    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, endNode);
    return DRPR_OK;
}

void dynRegexDestroy(dynRegex *dr)
{
    daDestroy(&dr->nodes, dynRegexNodeDestroy);
    daDestroy(&dr->links, dynRegexLinkDestroy);
    free(dr);
}

dynRegex *dynRegexCreate(const char *input)
{
    regexParser parser = {0};
    dynRegex *dr = (dynRegex *)calloc(1, sizeof(dynRegex));
    parser.input = input;
    parser.curr = parser.input;
    dynRegexNode *start = dynRegexNodeCreate(dr, 0);
    dynRegexNode *end = dynRegexNodeCreate(dr, 0);
    dynRegexParseResult result = dynRegexParse(dr, &parser, start, end, DRTT_EOF, 0, 1);
    if(result == DRPR_OK)
    {
        dynRegexNode *final = end;//dynRegexNodeCreate(dr);
        final->final = 1;
        dr->start = start;
        dr->end = final;
    }
    else
    {
        dynRegexDestroy(dr);
        dr = NULL;
    }
    return dr;
}

static void dsAppendChar(char **ds, int c)
{
    dsConcatf(ds, "%c", (char)c);
}

const char *generation(int g)
{
    switch(g)
    {
        case 0: return ",color=\"#ff0000\"";
        case 1: return ",color=\"#00ff00\"";
        case 2: return ",color=\"#0000ff\"";
        case 3: return ",color=\"#ffff00\"";
    };
    return "";
}

void dynRegexDot(dynRegex *dr)
{
    int i;
    char nodeLabel[2] = {'A', 'A'};

    printf("digraph PennyMachine {\n");
    printf("    overlap=false;\n");
    printf("    splines=true;\n");
    printf("    rankdir=LR;\n");
    for(i = 0; i < daSize(&dr->nodes); ++i)
    {
        dynRegexNode *n = dr->nodes[i];
        printf("    s%p [label=\"%c%c%s%s\",shape=circle,width=2,height=2%s];\n", 
            n, nodeLabel[0], nodeLabel[1],
            (n == dr->start) ? " (start)" : "",
            (n == dr->end) ? " (end)" : "",
            n->final ? ",style=filled,color=gray" : generation(n->generation)
            );
        ++nodeLabel[1];
        if(nodeLabel[1] > 'Z')
        {
            nodeLabel[1] = 'A';
            ++nodeLabel[0];
        }
    }
    for(i = 0; i < daSize(&dr->links); ++i)
    {
        dynRegexLink *link = dr->links[i];
        char *label = NULL;
        char *options = NULL;
        dsConcatf(&label, "");
        dsConcatf(&options, "");
        if(link->type == DRLT_CHAR)
        {
            int r;
            for(r = 0; r < daSize(&link->ranges); ++r)
            {
                dynRegexCharRange *range = link->ranges[r];
                dsAppendChar(&label, range->c1);
                if(range->c2)
                {
                    dsConcatf(&label, "-");
                    dsAppendChar(&label, range->c2);
                }
            }
            if(link->not)
            {
                dsConcatf(&options, ",color=red");
            }
            else
            {
                dsConcatf(&options, ",color=blue");
            }
        }
        printf("    s%p -> s%p [label=\"%s\"%s];\n", link->src, link->dst, label, options);
        dsDestroy(&label);
        dsDestroy(&options);
    }
    printf("}\n");
}

int main(int argc, char **argv)
{
    if(argc > 1)
    {
        dynRegex *dr = dynRegexCreate(argv[1]);
        if(dr)
        {
            dynRegexDot(dr);
            dynRegexDestroy(dr);
        }
    }
    return 0;
}
