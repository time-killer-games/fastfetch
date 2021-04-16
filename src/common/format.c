#include "fastfetch.h"

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg)
{
    if(formatarg->type == FF_FORMAT_ARG_TYPE_INT)
        ffStrbufAppendF(buffer, "%i", *(int*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_UINT)
        ffStrbufAppendF(buffer, "%u", *(uint32_t*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_UINT8)
        ffStrbufAppendF(buffer, "%hhu", *(uint8_t*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_STRING)
        ffStrbufAppendF(buffer, "%s", (const char*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_STRBUF)
        ffStrbufAppend(buffer, (FFstrbuf*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_DOUBLE)
        ffStrbufAppendF(buffer, "%g", *(double*)formatarg->value);
    else if(formatarg->type != FF_FORMAT_ARG_TYPE_NULL)
    {
        fprintf(stderr, "Error: format string \"%s\": argument is not implemented: %i\n", buffer->chars, formatarg->type);
        ffStrbufDestroy(buffer);
        exit(806);
    }
}

void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments)
{
    uint32_t argCounter = 1; //First arg is 1 in fomat string

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
        }

        if(i == formatstr->length - 1)
        {
            ffStrbufAppendC(buffer, '{');
            continue; //This will always stop the loop
        }

        ++i;

        if(formatstr->chars[i] == '{')
        {
            ffStrbufAppendC(buffer, '{');
            continue;
        }

        uint32_t argIndex;

        if(formatstr->chars[i] == '}')
        {
            if(argCounter > numArgs)
            {
                ffStrbufAppendS(buffer, "{}");
                continue;
            }

            argIndex = argCounter++;
        }
        else
        {
            FFstrbuf argnumstr;
            ffStrbufInit(&argnumstr);

            while(formatstr->chars[i] != '}' && i < formatstr->length)
                ffStrbufAppendC(&argnumstr, formatstr->chars[i++]);

            if(
                ffStrbufIgnCaseCompS(&argnumstr, "e") == 0 ||
                ffStrbufIgnCaseCompS(&argnumstr, "error") == 0 ||
                ffStrbufIgnCaseCompS(&argnumstr, "0") == 0
            ) {
                ffStrbufAppend(buffer, error);
                ffStrbufDestroy(&argnumstr);
                continue;
            }

            //Test if argnumstr is valid
            if(
                argnumstr.length == 0 ||
                ffStrbufGetC(&argnumstr, 0) == '-' ||
                sscanf(argnumstr.chars, "%u", &argIndex) != 1 ||
                argIndex > numArgs
            ) {
                //Not valid
                ffStrbufAppendC(buffer, '{');
                ffStrbufAppend(buffer, &argnumstr);
                if(formatstr->chars[i] == '}') // We dont have a closing { when ending because whole format string is over
                    ffStrbufAppendC(buffer, '}');
                ffStrbufDestroy(&argnumstr);
                continue;
            }

            ffStrbufDestroy(&argnumstr);
        }

        ffFormatAppendFormatArg(buffer, &arguments[argIndex - 1]);
    }

    ffStrbufTrimRight(buffer, ' ');
}
