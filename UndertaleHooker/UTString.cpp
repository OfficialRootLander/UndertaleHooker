#include "pch.h"
#include "UTString.h"

std::vector<TextReplacement> Replacements = {
}; //empty for now


char* CurrentGameString = nullptr;
char* StringValue = nullptr;
OriginalUTStringFnType UTWriteStringHookOG = nullptr;
OriginalUTStringFnType UTStringGetHookOG = nullptr;
bool StringThreadRunning = true;
int ReadTicks = 0;
int TextPrintTicks = 0;
int TextModeTicks = 0;
int InActiveTextModeTicks = 0;
bool InTextMode = false;

char LastTail[6] = { 0 };
char BestString[512] = { 0 };
int BestLen = 0;
int AttemptCount = 0;

int StringReadTicks = 0;

//bug 1, crashes when to long wanted to buy item
//has to do with inventory popping up

HMODULE hModuleBase = GetModuleHandle(NULL);
static char OverrideBuffer[] = "\E0* ada^1.?/*"; // writable buffer
DWORD_PTR origContinue = (uintptr_t)hModuleBase + 0x1061A5; // will be base + 0x1061A7



int __fastcall UTString::WriteStringHook(UTString* ThisPtr)
{
    return UTWriteStringHookOG(ThisPtr);
}


//how to override
//1.get each byte until last expected ecx character in a address map
//2.write the new string across this address map
int ReadCharByte = 0;
int LastReadCharByte = 0;
BYTE* ReadCharAddress = nullptr;
int ConstantByte = 67; // C
bool toggle = false;

const int MaxSingleCharStrings = 1024;  // maximum bytes to track
BYTE* SingleCharMap[MaxSingleCharStrings] = { 0 };
int SingleCharCount = 0;

char TargetMap[256] = { 0 }; // max replacement length
const int TargetLen = sizeof(TargetMap) - 1; // exclude null terminator
BYTE* CharMap[TargetLen] = { 0 };           // track pointers to each char

int ReplaceMentLength;

int TourCounter = 0;
int ShopOffset = 100000;
int LoadCounter = 0;

//Undertale String always begin with *
//then apply a offset like 2 karakters 
//then print your characters

int Filled = 0;
int OffsetFromBeginning = 0;
bool EndOfLine = false;

bool IsSafeToReplace(char c)
{
    // Only overwrite if it's a normal letter, number, or space
    return (c >= ' ' && c <= '~') && !(c == '/' || c == '*' || c == '\'' ||
        c == '!' || c == '?' || c == '.' ||
        c == '&' || c == '^' || c == '-');
}

// Vector of replacements

bool ShouldOverrideDialogue = false;


int __fastcall UTString::GetStringHookFunc(UTString* ThisPtr) // edi = Dialogue
{
    //this apparantly makes all dialoque black 
    LoadCounter++;

    __asm
    {
        push eax
        movsx eax, byte ptr[ecx] // sign-extend 8-bit to 32-bit
        mov ReadCharByte, eax
        pop eax
    }

    BYTE* ptr = (BYTE*)ThisPtr;
    int length = 0;
    BYTE* start = ptr;

    while (*ptr)   // loop until null terminator
    {
        /*
        if (toggle)
            *ptr = 'A';
        else
            *ptr = 'C';
        */ //toggle later

        length++;
        toggle = !toggle;
        ptr++;
    }

    if (length == 1 && ShouldOverrideDialogue) // only process single-character strings
    {
        ReadCharAddress = &start[0];

        if (ReadCharByte == '*') //this needs to be better since this now only works for goddawn shops
        {
            OffsetFromBeginning = 0;
        }
        else if (OffsetFromBeginning < ReplaceMentLength)
        {
        //we ignore choice reports so &.& is auto end of line
            *ReadCharAddress = TargetMap[OffsetFromBeginning];
            OffsetFromBeginning++;
        }
        else
        {
        //idk but this is not fine
        //for normal dialoque
            *ReadCharAddress = ' ';
        }
    }
    else
    {
        //should be preserved
    }

    LastReadCharByte = ReadCharByte;

    ReadTicks++;

    if (ReadTicks < 200)
    {
        return UTStringGetHookOG(ThisPtr);
    }
    ReadTicks = 0;

    __asm
    {
        mov StringValue, ecx
    }
    //char** StringPtr = (char**)ThisPtr;
    //*StringPtr = OverrideBuffer;

    return UTStringGetHookOG(ThisPtr);
}

std::string UTString::GetUTString(std::string Str)
{
    if (Str.empty()) return "";


    std::string out;
    out.reserve(Str.size() * 2 + 1);

    out.push_back(Str[0]);
    out.push_back(Str[0]);

    for (int i = 1; i < Str.size(); i++)
    {
        out.push_back(Str[i]);
        out.push_back(Str[i]);
    }

    return out;
}

char safeBuffer2[512];
void UTString::OverrideStringNow(const char* ReplacementStr)
{
    if (!StringValue)
        return;

    std::string converted = UTString::GetUTString(ReplacementStr);

    TextReplacement rep;
    rep.Target = _strdup(StringValue);
    rep.Replace = _strdup(converted.c_str());
    rep.TargetLength = strlen(rep.Target);
    rep.ReplaceLength = strlen(rep.Replace);

    Replacements.push_back(rep);

    // Fill TargetMap safely
    memset(TargetMap, 0, sizeof(TargetMap));
    memcpy(TargetMap, rep.Replace, min(rep.ReplaceLength, sizeof(TargetMap) - 1));

    ReplaceMentLength = rep.ReplaceLength;
    OffsetFromBeginning = 9999;
    ShouldOverrideDialogue = true;

    printf("[OverrideStringNow] Added replacement: '%s' -> '%s'\n",
        rep.Target,
        rep.Replace);
}



char safeBuffer[512];
DWORD WINAPI UTString::StringThread(LPVOID lParams) //when running again the string somehow doesnt update to trigger
{
    while (StringThreadRunning)
    {

        if (StringValue && IsBadReadPtr(StringValue, 512) == false)
        {
            strncpy_s(safeBuffer, StringValue, sizeof(safeBuffer) - 1);
            safeBuffer[sizeof(safeBuffer) - 1] = 0;
            int len = strlen(safeBuffer);

            if (len >= 5)
            {
                TextModeTicks++;
                if (!InTextMode && TextModeTicks >= 3)
                {
                    InTextMode = true;
                    printf("[ENTER TEXT MODE]\n");
                }

                const char* newTail = safeBuffer + (len - 5);

                if (strcmp(newTail, LastTail) != 0)
                {
                    if (len > BestLen)
                    {
                        strcpy_s(BestString, safeBuffer);
                        BestLen = len;
                    }

                    AttemptCount++;

                    if (AttemptCount >= 20)
                    {
                        printf("dialogue: %s\n", BestString);
                        strcpy_s(LastTail, BestString + (BestLen - 5));
                        EndOfLine = false;

                        bool FoundReplaceMent = false;
                        for (const auto& rep : Replacements)
                        {
                            // Check if StringValue contains rep.Target
                            const char* found = strstr(safeBuffer, rep.Target);
                            if (found)
                            {
                                memcpy(TargetMap, rep.Replace, rep.ReplaceLength); // copy new target map
                                ReplaceMentLength = (int)rep.ReplaceLength;
                                OffsetFromBeginning = 9999;
                                FoundReplaceMent = true;
                                printf("Found String Match!\n");
                                break;
                            }
                        }
                        ShouldOverrideDialogue = FoundReplaceMent;

                        //execute dialoque triggers
                        // Reset for next dialogue
                        BestLen = 0;
                        AttemptCount = 0;
                        BestString[0] = 0;
                    }
                }
            }
            else
            {
                if (InTextMode)
                {
                    if (TextModeTicks == 0 || InActiveTextModeTicks > 100)
                    {
                        InTextMode = false;
                        printf("[EXIT TEXT MODE]\n");
                        InActiveTextModeTicks = 0;
                    }
                    else
                    {
                        InActiveTextModeTicks++;
                        TextModeTicks--;
                    }
                }
            }
            StringReadTicks = 0;
        }
        Sleep(10); //risky but ok
    }

    return 0;
}

