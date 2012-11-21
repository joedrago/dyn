#include <stdio.h>
#include <stdlib.h>

typedef enum dynRegexTokenType
{
    DRTT_EOF = 0,
    DRTT_CHAR,
    DRTT_OR,
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

    DRPR_COUNT
} dynRegexParseResult;

struct dynRegexNode;
struct dynRegexLink;
struct dynRegex;

typedef struct dynRegexNode
{
    struct dynRegexLink **links;
    int final;
} dynRegexNode;

typedef enum dynRegexLinkType
{
    DRLT_BLANK = 0,
    DRLT_CHAR,
    DRLT_NOT_CHAR,
    DRLT_CC,
    DRLT_NOT_CC,

    DRLT_COUNT
} dynRegexLinkType;

typedef struct dynRegexLink
{
    int type;
    int c1;
    int c2;
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

static dynRegexNode *dynRegexNodeCreate(struct dynRegex *r)
{
    dynRegexNode *n = (dynRegexNode *)calloc(1, sizeof(dynRegexNode));
    daPush(&r->nodes, n);
    return n;
}

static void dynRegexNodeDestroy(struct dynRegexNode *n)
{
    daDestroy(&n->links, NULL);
    free(n);
}

static dynRegexLink *dynRegexLinkCreate(struct dynRegex *r, dynRegexLinkType type, dynRegexNode *src, dynRegexNode *dst, int c1, int c2)
{
    dynRegexLink *l = (dynRegexLink *)calloc(1, sizeof(dynRegexLink));
    l->type = type;
    l->src = src;
    l->dst = dst;
    l->c1 = c1;
    l->c2 = c2;
    daPush(&r->links, l);
    daPush(&src->links, l);
    return l;
}

static void dynRegexLinkDestroy(dynRegexLink *l)
{
    free(l);
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

static dynRegexNode * appendCharacterLink(dynRegex *dr, dynRegexNode *startNode, dynRegexNode *endNode, int charRangeBegin, int charRangeEnd, int not)
{
    dynRegexNode *s = dynRegexNodeCreate(dr);
    int type;
    if(charRangeEnd)
    {
        type = not ? DRLT_NOT_CC : DRLT_CC;
    }
    else
    {
        type = not ? DRLT_NOT_CHAR : DRLT_CHAR;
    }
    dynRegexLinkCreate(dr, DRLT_BLANK, startNode, s, 0, 0);
    dynRegexLinkCreate(dr, type, s, endNode, charRangeBegin, charRangeEnd);
    if(not)
    {
        return s;
    }
    return startNode;
}

// Returns the "end" node
static dynRegexParseResult dynRegexParse(dynRegex *dr, regexParser *parser, dynRegexNode *startNode, dynRegexNode *endNode, dynRegexTokenType endType, int flags)
{
    int done = 0;
    dynRegexNode *appendNode = startNode;
    while(!done)
    {
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
                    dynRegexParseResult result = dynRegexParse(dr, parser, startNode, endNode, endType, flags);
                    dynRegexNode *final;
                    if(result != DRPR_OK)
                    {
                        return result;
                    }

                    done = 1;
                }
                break;

            case DRTT_CC_BEGIN:
                {
                    int c = 0;
                    int range = 0;
                    int not = 0;
                    int first = 1;
                    dynRegexNode *e = dynRegexNodeCreate(dr);
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
                                appendNode = appendCharacterLink(dr, appendNode, e, c, 0, not);
                            }
                            break;
                        }
                        if(token.type != DRTT_CHAR)
                        {
                            return DRPR_INVALID_CC;
                        }

                        // handle DRTT_CHAR
                        if(token.c == '-')
                        {
                            if(c)
                            {
                                range = 1;
                            }
                            else
                            {
                                appendNode = appendCharacterLink(dr, appendNode, e, token.c, 0, not);
                            }
                        }
                        else
                        {
                            if(range)
                            {
                                appendNode = appendCharacterLink(dr, appendNode, e, c, token.c, not);
                                range = 0;
                                c = 0;
                            }
                            else
                            {
                                if(c)
                                {
                                    appendNode = appendCharacterLink(dr, appendNode, e, c, 0, not);
                                }
                                c = token.c;
                            }
                        }
                    }
                    appendNode = e;
                }
                break;

            case DRTT_CHAR:
                {
                    dynRegexNode *s = dynRegexNodeCreate(dr);
                    dynRegexNode *e = dynRegexNodeCreate(dr);
                    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, s, 0, 0);
                    dynRegexLinkCreate(dr, DRLT_CHAR, s, e, token.c, 0);
                    appendNode = e;
                }
                break;
        };
    }
    dynRegexLinkCreate(dr, DRLT_BLANK, appendNode, endNode, 0, 0);
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
    dynRegexNode *start = dynRegexNodeCreate(dr);
    dynRegexNode *end = dynRegexNodeCreate(dr);
    dynRegexParseResult result = dynRegexParse(dr, &parser, start, end, DRTT_EOF, 0);
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
            n->final ? ",style=filled,color=gray" : ""
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
        char label[128];
        label[0] = 0;
        if(link->type == DRLT_CHAR)
        {
            sprintf(label, "[label=\"%c\",color=blue]", link->c1);
        }
        else if(link->type == DRLT_NOT_CHAR)
        {
            sprintf(label, "[label=\"%c\",color=red]", link->c1);
        }
        else if(link->type == DRLT_CC)
        {
            sprintf(label, "[label=\"%c-%c\",color=blue]", link->c1, link->c2);
        }
        else if(link->type == DRLT_NOT_CC)
        {
            sprintf(label, "[label=\"%c-%c\",color=red]", link->c1, link->c2);
        }
        printf("    s%p -> s%p %s;\n", link->src, link->dst, label);
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
