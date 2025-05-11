/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hashtable.h"
#include "linkLish.h"
#include "myString.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MOVB_CHECK 0
#define MOVW_CHECK 1
#define MOVDW_CHECK 2

#define CONVERT_INT_TO_DINT 0
#define CONVERT_DINT_TO_REAL 1


#define CAL_DIV 0
#define CAL_MUL 1

//------------------------------------------------- define memory region --------------------------------------------
// Lựa chọn vùng nhớ
#define I_MEM 0
#define Q_MEM 1
#define M_MEM 2
#define AI_MEM 3
#define u16_VM_MEM 4
#define u32_VM_MEM 5
#define f32_VM_MEM 6

// Chọn số lượng biến cho vùng nhớ (kiểu byte)
#define SUM_I 3
#define SUM_Q 3
#define SUM_M 3
#define SUM_AI 3

/*******************************************************************************
 * Variables
 ******************************************************************************/

static LinkList *First, *Last, *FirstFinal, *LastFinal; // First và Last dùng cho  TransferToList ()   ; FirstFinal , LastFinal  là chuỗi cuối cùng cần tìm
static FILE *pFileTimer = NULL;
static int CountTimer = 0;
extern stringHashTable g_Save_IO[H_PRIMER_NUMBER];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void CheckCountQuestionMark(int *CountQuestionMark, int *CheckBigBranch, char *(*OutString), char *(*OutCheckBigBranch));
static void CheckNOT(LinkList *(*pMain), LinkList *pNext, LinkList *pNext1, char *(*OutString), char *Insert);
static void AddPlusToOutString(LinkList *(*pMain), char *(*OutString), int *CountQuestionMark, char *Insert);
static void InsertTimer(LinkList *pMain, int *CountTimer, FILE *pFileTimer);
static void SetupCounterUpOrDown(LinkList *(*pMain), char *OutString, FILE *pFile);
static void SetupCounterUpDown(LinkList *(*pMain), char *OutString, FILE *pFile);
static char *CheckQuestionMask(int CountQuestionMark, char *OutString);
static void InsertMov(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int checkMov);
static void InsertConvert(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int TypeConvert);
static void InsertCaculate(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int TypeConvert);
static int getNumber(char *p, int len);
static void DefineRegionMemory(FILE *pFile, int memoryRegion, int sumOfmem);
static void DefineIO(FILE *pFile, int memoryRegion, int sumOfmem);
static void readInputPin(FILE *pFile, int sumOfmem);
static void writeOutputPin(FILE *pFile, int sumOfmem);

/************************************************************************************
 * Static function
 *************************************************************************************/

static void CheckCountQuestionMark(int *CountQuestionMark, int *CheckBigBranch, char *(*OutString), char *(*OutCheckBigBranch))
{
    char *InsertQuestionMark = "?";
    if (*CountQuestionMark > 1)
    {
        *OutString = MS_AddParenthesesIfMissing(*OutString);
        int SizeToken = 0;
        int SizeTokenTemp = 0;
        int CountTemp = 0;
        int size_O = strlen(*OutString);
        char *OutStringTemp = "";
        OutStringTemp = MS_StrAllocAndAppend(OutStringTemp, *OutString); // Lưu tạm thời OUT vào OUT_T
        char *token = strtok(*OutString, "?");                        /// (...)?((.....)?(......))
        // thêm 1 vòng while để biết timer , couter
        while (token != NULL)
        {
            CountTemp++;
            SizeToken = strlen(token); // tính size token cuối cùng
            if (SizeTokenTemp == SizeToken)
            {
                break;
            }

            SizeTokenTemp = SizeToken;

            token = strtok(NULL, "?");
        }
        char *token1 = strtok(OutStringTemp, "?");
        for (int i = 0; i < CountTemp - 2; i++)
        {
            *OutCheckBigBranch = MS_StrAllocAndAppend(*OutCheckBigBranch, token1); /// (...)?
            *OutCheckBigBranch = MS_StrAllocAndAppend(*OutCheckBigBranch, InsertQuestionMark);
            token1 = strtok(NULL, "?");
        }
        *OutString = "";
        *OutString = MS_StrAllocAndAppend(*OutString, token1);
        *OutString = MS_StrAllocAndAppend(*OutString, InsertQuestionMark);
        token1 = strtok(NULL, "?");
        *OutString = MS_StrAllocAndAppend(*OutString, token1);
        *CheckBigBranch = 1;
        *CountQuestionMark = 1;
    }
}

static  char *CheckQuestionMask(int CountQuestionMark, char *OutString)
{
    if (CountQuestionMark > 0)
    {
        char *tempString;
        char *InsertQuestionMark = "?";
        int SizeToken = 0;
        int SizeTokenTemp = 0;
        int CountTemp = 0;
        int size_O = strlen(OutString);
        char *OutStringTemp = "";
        char *OutStringTemp_1 = "";
        OutStringTemp_1 = MS_StrAllocAndAppend(OutStringTemp_1, OutString);
        OutStringTemp = MS_StrAllocAndAppend(OutStringTemp, OutString); // Lưu tạm thời OUT vào OUT_T
        char *token = strtok(OutStringTemp_1, "?");                  /// (...)?((.....)?(......))
        // thêm 1 vòng while để biết timer , couter
        while (token != NULL)
        {
            CountTemp++;
            SizeToken = strlen(token); // tính size token cuối cùng
            if (SizeTokenTemp == SizeToken)
            {
                break;
            }

            SizeTokenTemp = SizeToken;

            token = strtok(NULL, "?");
        }
        char *token1 = strtok(OutStringTemp, "?");
        for (int i = 0; i < CountTemp - 1; i++)
        {
            token1 = strtok(NULL, "?");
        }
        tempString = "";
        tempString = MS_StrAllocAndAppend(tempString, token1);

        return tempString;
    }
    else
    {
        return OutString;
    }
}

static void CheckNOT(LinkList *(*pMain), LinkList *pNext, LinkList *pNext1, char *(*OutString), char *Insert)
{
    char *InsertClosingBracket = ")";
    if (strcmp(pNext1->data, "NOT") == 0)
    {

        *OutString = MS_StrAllocAndAppend(*OutString, Insert);
        *OutString = MS_StrAllocAndAppend(*OutString, pNext->data);
        *OutString = MS_StrAllocAndAppend(*OutString, InsertClosingBracket);
        *pMain = pNext1->next;
    }
    else if (strcmp(pNext1->data, "NOT") != 0)
    {
        *OutString = MS_StrAllocAndAppend(*OutString, Insert);
        *OutString = MS_StrAllocAndAppend(*OutString, pNext->data);
        *pMain = pNext1;
    }
}

static void AddPlusToOutString(LinkList *(*pMain), char *(*OutString), int *CountQuestionMark, char *Insert)
{
    char *InsertOpeningBracket = "(";
    char *InsertClosingBracket = ")";
    LinkList *pNext;
    *OutString = MS_StrAllocAndAppend(InsertOpeningBracket, *OutString); // Mở ngoặc cả cụm ALD này
    pNext = (*pMain)->next;
    if (strncmp(pNext->data, "!", 1) == 0)
    {
        char *arr_temp = "!";
        *OutString = MS_StrAllocAndAppend(arr_temp, *OutString);
        *pMain = (*pMain)->next;
    }
    int size_of_arr = strlen(*OutString);

    char *OUTtemp, *OUTtemp1;
    OUTtemp = (char *)calloc(size_of_arr, sizeof(char));
    strcpy(OUTtemp, *OutString);
    char *token = strtok(OUTtemp, "?");
    OUTtemp = MS_StrAllocAndAppend(OUTtemp, Insert);
    (*CountQuestionMark)--; // Thay đấu "?" bằng dấu "+"
    token = strtok(NULL, " ");
    OUTtemp1 = MS_StrAllocAndAppend(OUTtemp, token);
    free(*OutString);
    size_of_arr = strlen(OUTtemp1);
    *OutString = (char *)calloc(size_of_arr, sizeof(char));
    strcpy(*OutString, OUTtemp1);
    *OutString = MS_StrAllocAndAppend(*OutString, InsertClosingBracket); // Đóng ngoặc cụm ALD này
    free(OUTtemp1);
    *pMain = (*pMain)->next;
}

static void SetupCounterUpOrDown(LinkList *(*pMain), char *OutString, FILE *pFile)
{
    char *TempArry = "";
    char *equal = " = ";
    char *token = strtok(OutString, "?");                // ( M0_1) < OutString , token >
    *pMain = (*pMain)->next;                             // C1
    TempArry = MS_StrAllocAndAppend((*pMain)->data, equal); // C1_CTU
    TempArry = MS_StrAllocAndAppend("vao", TempArry);       // vaoC1_CTU
    token = MS_AddParenthesesIfMissing(token);
    TempArry = MS_StrAllocAndAppend(TempArry, token);
    TempArry = MS_StrAllocAndAppend(TempArry, " ;\n");
    fputs(TempArry, pFile);
    token = strtok(NULL, "?");
    TempArry = MS_StrAllocAndAppend((*pMain)->data, equal);
    TempArry = MS_StrAllocAndAppend("reset", TempArry);
    token = MS_AddParenthesesIfMissing(token);
    TempArry = MS_StrAllocAndAppend(TempArry, token);
    TempArry = MS_StrAllocAndAppend(TempArry, " ;\n");
    fputs(TempArry, pFile);
    *pMain = (*pMain)->next;
}

static void SetupCounterUpDown(LinkList *(*pMain), char *OutString, FILE *pFile) // "_CTU ="
{
    char *TempArry = "";
    char *token = strtok(OutString, "?");
    *pMain = (*pMain)->next;
    TempArry = MS_StrAllocAndAppend((*pMain)->data, " = ");
    TempArry = MS_StrAllocAndAppend("tang", TempArry);
    token = MS_AddParenthesesIfMissing(token);
    TempArry = MS_StrAllocAndAppend(TempArry, token);
    TempArry = MS_StrAllocAndAppend(TempArry, " ;\n");
    fputs(TempArry, pFile);
    token = strtok(NULL, "?");
    TempArry = MS_StrAllocAndAppend((*pMain)->data, " = ");
    TempArry = MS_StrAllocAndAppend(" giam", TempArry);
    token = MS_AddParenthesesIfMissing(token);
    TempArry = MS_StrAllocAndAppend(TempArry, token);
    TempArry = MS_StrAllocAndAppend(TempArry, " ;\n");
    fputs(TempArry, pFile);
    token = strtok(NULL, "?");
    TempArry = MS_StrAllocAndAppend((*pMain)->data, " = ");
    TempArry = MS_StrAllocAndAppend(" reset", TempArry);
    token = MS_AddParenthesesIfMissing(token);
    TempArry = MS_StrAllocAndAppend(TempArry, token);
    TempArry = MS_StrAllocAndAppend(TempArry, " ;\n");
    fputs(TempArry, pFile);
    *pMain = (*pMain)->next;
}


static void DefineRegionMemory(FILE *pFile, int memoryRegion, int sumOfmem)
{
    char check[8];
    if (memoryRegion == I_MEM)
    {
        fprintf(pFile, "\n\n// Define I\n");
        strcpy(check, "I");
    }
    else if (memoryRegion == Q_MEM)
    {
        fprintf(pFile, "\n\n// Define Q\n");
        strcpy(check, "Q");
    }
    else if (memoryRegion == M_MEM)
    {
        fprintf(pFile, "\n\n// Define M\n");
        strcpy(check, "M");
    }
    else if (memoryRegion == AI_MEM)
    {
        fprintf(pFile, "\n\n// Define AIW\n");
        strcpy(check, "AI");
    }
    else if (memoryRegion == u16_VM_MEM)
    {
        fprintf(pFile, "\n\n// Define AIW\n");
        strcpy(check, "u16VW");
    }
    else if (memoryRegion == u32_VM_MEM)
    {
        fprintf(pFile, "\n\n// Define AIW\n");
        strcpy(check, "u32VD");
    }
    else if (memoryRegion == f32_VM_MEM)
    {
        fprintf(pFile, "\n\n// Define AIW\n");
        strcpy(check, "f32VD");
    }

    for (int x = 0; x < sumOfmem; x++)
    {
        int bytedef = 1; // Định nghĩa 1 byte , dW , W
        for (int i = 0; i < 8; i++)
        {
            if (memoryRegion < AI_MEM)
            {
                if (bytedef == 1)
                {
                    fprintf(pFile, "#define %sB%d %s[%d][%d]\n", check, x, check, x, i);
                    fprintf(pFile, "#define %sDW%d %s[%d][%d]\n", check, x, check, x, i);
                    fprintf(pFile, "#define %sW%d %s[%d][%d]\n", check, x, check, x, i);
                    bytedef = 0;
                }
                fprintf(pFile, "#define %s%d_%d %s[%d][%d]\n", check, x, i, check, x, i);
            }
            else if (memoryRegion >= AI_MEM)
            {
                fprintf(pFile, "#define %sW%d %s[%d][%d]\n", check, i, check, i);
            }
            else
            {
                fprintf(pFile, "#define %s%d %s[%d]\n", check, i, check, i);
            }
        }
    }
}

static void DefineIO(FILE *pFile, int memoryRegion, int sumOfmem)
{
    char check[2];
    if (memoryRegion == I_MEM)
    {
        fprintf(pFile, "\n\n// Define Input Pin\n");
        strcpy(check, "I");
    }
    else if (memoryRegion == Q_MEM)
    {
        fprintf(pFile, "\n\n// Define Output pin\n");
        strcpy(check, "Q");
    }
    for (int x = 0; x < sumOfmem; x++)
    {
        char *compare;
        for (int i = 0; i < 8; i++)
        {
            char buffer[15];
            sprintf(buffer, "%s%d_%d", check, x, i);
            compare = buffer;
            int Search = H_FindFunction(compare);
            if (Search >= 0)
                fprintf(pFile, "#define %s_PIN GPIO_PIN_\n", g_Save_IO[Search]);
        }
    }

    if (memoryRegion == I_MEM)
    {
        fprintf(pFile, "\n\n// Define Input Port\n");
    }
    else if (memoryRegion == Q_MEM)
    {
        fprintf(pFile, "\n\n// Define Output Port\n");
    }
    for (int x = 0; x < sumOfmem; x++)
    {
        char *compare;

        for (int i = 0; i < 8; i++)
        {
            char buffer[15];
            sprintf(buffer, "%s%d_%d", check, x, i);
            compare = buffer;
            int Search = H_FindFunction(compare);
            if (Search >= 0)
                fprintf(pFile, "#define %s_PORT GPIO\n", g_Save_IO[Search]);
        }
    }
}

static void readInputPin(FILE *pFile, int sumOfmem)
{
    int Search =0;

    fprintf(pFile, "{\n");

    for (int x = 0; x < sumOfmem; x++)
    {
        char *compare;
        for (int i = 0; i < 8; i++)
        {
            char buffer_I[15];
            char buffer_AI[15];
            sprintf(buffer_I, "I%d_%d", x, i);
            sprintf(buffer_AI, "AIW_%d", i);
            compare = buffer_I;
            Search = H_FindFunction(compare);
            if (Search >= 0)
                fprintf(pFile, "%s = !HAL_GPIO_ReadPin(%s_PORT, %s_PIN);\n", g_Save_IO[Search], g_Save_IO[Search], g_Save_IO[Search]);
            compare = buffer_AI;

            Search = H_FindFunction(compare);
        }
    }
    fprintf(pFile, "}\n");
}

static void writeOutputPin(FILE *pFile, int sumOfmem)
{
    fprintf(pFile, "{\n");
    for (int x = 0; x < sumOfmem; x++)
    {
        char *compare;
        for (int i = 0; i < 8; i++)
        {
            char buffer[15];
            sprintf(buffer, "Q%d_%d", x, i);
            compare = buffer;
            int Search = H_FindFunction(compare);
            if (Search >= 0)
            {
                fprintf(pFile, "if(%s >= 1){\n", g_Save_IO[Search]);
                fprintf(pFile, "		HAL_GPIO_WritePin(%s_PORT, %s_PIN, ENABLE);\n", g_Save_IO[Search], g_Save_IO[Search]);
                fprintf(pFile, "		}\n");
                fprintf(pFile, "	else{\n");
                fprintf(pFile, "		HAL_GPIO_WritePin(%s_PORT, %s_PIN, DISABLE);\n", g_Save_IO[Search], g_Save_IO[Search]);
                fprintf(pFile, "	}\n");
            }
        }
    }
    fprintf(pFile, "}\n");
}

static void InsertMov(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int checkMov)
{
    int check;
    if (checkMov == MOVB_CHECK)
        check = 1;
    if (checkMov == MOVW_CHECK)
        check = 2;
    if (checkMov == MOVDW_CHECK)
        check = 4;
    LinkList *pNext = (*pMain)->next;
    LinkList *pNext1 = pNext->next;
    pNext = pNext1->next;
    LinkList *temp;
    temp = (*pMain);
    (*pMain) = (*pMain)->next;
    L_DeleteLinkList(&temp, &(temp->prev), &(temp->next));
    temp = (*pMain)->next;
    if (pNext != NULL)
    {
        if (strcmp(pNext->data, "AENO") == 0)
        {

            char *InsertString = CheckQuestionMask(CountQuestionMark, OutString);
            InsertString = MS_AddParenthesesIfMissing(InsertString);
            // printf("\n%s\n", InsertString);
            fprintf(pFile, "if(%s)\n ", InsertString);

            if (strncmp(temp->data, "VW",2) == 0)
            {
                temp->data = MS_StrAllocAndAppend("u16",temp->data);
            }
            else if (strncmp(temp->data, "VD",2) == 0)
            {
                temp->data = MS_StrAllocAndAppend("u32",temp->data);
            }

            if (strncmp((*pMain)->data, "VW",2) == 0)
            {
                (*pMain)->data = MS_StrAllocAndAppend("u16",(*pMain)->data);
            }
            else if (strncmp((*pMain)->data, "VD",2) == 0)
            {
                (*pMain)->data = MS_StrAllocAndAppend("u32",temp->data);
            }

            fprintf(pFile, " (memcpy(&%s,&%s,%d)) ;\n", temp->data, (*pMain)->data, check);
            L_DeleteLinkList(&(*pMain), &((*pMain)->prev), &((*pMain)->next));
            L_DeleteLinkList(&temp, &(temp->prev), &(temp->next));
            (*pMain) = pNext->next;
            L_DeleteLinkList(&pNext, &(pNext->prev), &(pNext->next));
        }
        else
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            fprintf(pFile, "if(%s)\n ", OutString);
            fprintf(pFile, " (memcpy(&%s,&%s,1)) ; \n", temp->data, (*pMain)->data);
            L_DeleteLinkList(&(*pMain), &((*pMain)->prev), &((*pMain)->next));
            L_DeleteLinkList(&temp, &(temp->prev), &(temp->next));
            (*pMain) = pNext;
            // L_DeleteLinkList(&pNext, &(pNext->prev), &(pNext->next));
        }
    }
    else
    {
        OutString = MS_AddParenthesesIfMissing(OutString);
        fprintf(pFile, "if(%s)\n ", OutString);
        fprintf(pFile, " (memcpy(&%s,&%s,1)) ; \n", temp->data, (*pMain)->data);
    }
}


static void InsertConvert(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int TypeConvert)
{
    LinkList *pNext = (*pMain)->next;
    LinkList *pNext1 = pNext->next;
    // pNext = pNext1->next;
    LinkList *temp;
    temp = (*pMain);
    (*pMain) = (*pMain)->next;
    // L_DeleteLinkList(&temp, &(temp->prev), &(temp->next));
    temp = (*pMain)->next;

    if (TypeConvert == CONVERT_INT_TO_DINT)
    {
        char *InsertString = CheckQuestionMask(CountQuestionMark, OutString);
        InsertString = MS_AddParenthesesIfMissing(InsertString);
        // printf("\n%s\n", InsertString);
        fprintf(pFile, "if(%s)\n ", InsertString);
        fprintf(pFile, " u32%s = (uint32_t)u16%s;\n", temp->data, (*pMain)->data);
        (*pMain) = pNext->next;
    }
    else if (TypeConvert == CONVERT_DINT_TO_REAL)
    {
        char *InsertString = CheckQuestionMask(CountQuestionMark, OutString);
        InsertString = MS_AddParenthesesIfMissing(InsertString);
        // printf("\n%s\n", InsertString);
        fprintf(pFile, "if(%s)\n ", InsertString);
        fprintf(pFile, " f32%s = (float)u32%s;\n", temp->data, (*pMain)->data);
        (*pMain) = pNext->next;
    }
}

static void InsertCaculate(LinkList *(*pMain), char *OutString, int CountQuestionMark, FILE *pFile, int TypeConvert)
{
    int i = 0;
    int check = 1;
    LinkList *pNext = (*pMain)->next;
    LinkList *pNext1 = pNext->next;
    // pNext = pNext1->next;
    LinkList *temp;
    temp = (*pMain);
    (*pMain) = (*pMain)->next;
    // L_DeleteLinkList(&temp, &(temp->prev), &(temp->next));
    temp = (*pMain)->next;


    if (strncmp(temp->data, "VW",2) == 0)
    {
        temp->data = MS_StrAllocAndAppend("u16",temp->data);
    }
    else if (strncmp(temp->data, "VD",2) == 0)
    {
        temp->data = MS_StrAllocAndAppend("u32",temp->data);
    }
    else
    {
        check = 0;
    }

    while(((*pMain)->data[i] !='\0') && check)
    {
        if((*pMain)->data[i] == '_')
        {
            (*pMain)->data[i] = '.';
        }
        i++;
    }

    if (TypeConvert == CAL_DIV)
    {
        char *InsertString = CheckQuestionMask(CountQuestionMark, OutString);
        InsertString = MS_AddParenthesesIfMissing(InsertString);
        // printf("\n%s\n", InsertString);
        fprintf(pFile, "if(%s)\n ", InsertString);
        fprintf(pFile, " %s = %s/%s;\n", temp->data,temp->data, (*pMain)->data);
        (*pMain) = pNext->next;
    }
    else if (TypeConvert == CAL_MUL)
    {
        char *InsertString = CheckQuestionMask(CountQuestionMark, OutString);
        InsertString = MS_AddParenthesesIfMissing(InsertString);
        // printf("\n%s\n", InsertString);
        fprintf(pFile, "if(%s)\n ", InsertString);
        fprintf(pFile, " %s = %s*%s;\n", temp->data,temp->data, (*pMain)->data);
        (*pMain) = pNext->next;
    }
}


static int getNumber(char *p, int len) // Đọc số từ chuỗi  vd : "97" -> (int) 97
{
    int value;
    if (len = 2)
    {
        value = (((p[0] - 48) * 10) + (p[1] - 48));
    }
    else if (len == 1)
    {
        value = p[0] - 48;
    }
    else
    {
        value = (((p[0] - 48) * 100) + (p[1] - 48) * 10 + (p[2] - 48));
    }

    return value;
}

static void InsertTimer(LinkList *pMain, int *CountTimer, FILE *pFileTimer)
{
    LinkList *pNext;
    char *buffer = "";
    char *temp = "";
    char *token;
    buffer = MS_StrAllocAndAppend(buffer, pMain->data); // TON
    pNext = pMain->next;
    buffer = MS_StrAllocAndAppend(buffer, pNext->data); // T97
    temp = MS_StrAllocAndAppend(temp, pNext->data);     // T97
    token = strtok(temp, "T");
    int k = getNumber(token, strlen(token));
    int b;
    /*
        1ms
            // TON , TOFF
            T32 ,T96
            // TONR
            T0 , T64
        10ms
            // TON , TOFF
            T97 - T99
            T100
            T33-T36
            // TONR
            //
            T1-4 ,T65-68

        100ms
            // TON , TOFF
            T37-T63
            T101-T255
            // TONR
            T5-T31
            T69-T95
     */
    if ((k == 32) || (k == 96) || (k == 0) || (k == 64))
    {
        b = 1;
    }
    else if ((k == 100) || ((97 <= k) && (k <= 99)) || ((33 <= k) && (k <= 36)) || ((1 <= k) && (k <= 4)) || ((65 <= k) && (k <= 68)))
    {
        b = 10;
    }
    else
    {
        b = 100;
    }

    pNext = pNext->next; // 20
                         // fprintf(pFileTimer, " handle_timer[%d] =  xTimerCreate(\"timer%s\", pdMS_TO_TICKS(%s*%d),pdFALSE,(static void *)(%d+1),%s) ;\n", *CountTimer, buffer, pNext->data, b, *CountTimer, buffer);
    fprintf(pFileTimer, " handle_timerPLC[%d] =  xTimerCreate(\"timer%s\", pdMS_TO_TICKS(%d),pdTRUE,(static void *)(%d+1),TimerCallBack) ;\n", *CountTimer, buffer, b, *CountTimer);
    sprintf(buffer, "%s_%d", buffer, *CountTimer + 1);
    H_InsertFunction(buffer);
    (*CountTimer)++;
}


/*******************************************************************************
 * Code
 ******************************************************************************/

void STL_CreatFileNoComment(int *RowOfFile)
{
    FILE *pfile = NULL, *pFileFinal = NULL;

    char TempArray[30];
    int count = 0;
    int ReductionCount = 0;
    int CheckProgram = 0; // Kiểm tra xem đã vào chương trình chưa
    pFileFinal = fopen("PLC.txt", "w");
    // Mở file bằn hàm fopen
    pfile = fopen("PLC.awl", "r");
    if (pfile == NULL)
    {
        printf("Can not open file");
    }
    //Đọc từng dòng từ file cho tới khi gặp NULL
    while (fgets(TempArray, 128, pfile) != NULL)
    {
        count = count + 1;
        if (CheckProgram == 0 && (strncmp(TempArray, "LD", 2) == 0))
        {
            ReductionCount = count;
            CheckProgram = 1;
        }

        if (CheckProgram == 1)
        {

            if (strncmp(TempArray, "Network", 7) == 0)
            {
                fputs("N\n", pFileFinal);
            }
            else if (strncmp(TempArray, "END", 3) == 0)
            {
                fputs("N\n", pFileFinal);
                break;
            }
            else
            {
                fputs(TempArray, pFileFinal);
            }
        }
    }
    *RowOfFile = count - ReductionCount;

    fclose(pfile);
    fclose(pFileFinal);
}

void STL_CreatList(int RowOfFile)
{
    LinkList *pMain;
    char TempArray[30];
    First = NULL;
    FILE *pFileFinal;
    pFileFinal = fopen("PLC.txt", "r");
    for (int i = 0; i < RowOfFile; i++)
    {
        fgets(TempArray, 30, pFileFinal);
        for (int CharacterOfRow = 0; CharacterOfRow < 30; CharacterOfRow++)
        {
            pMain = (LinkList *)malloc(sizeof(LinkList));
            pMain->next = NULL;
            pMain->data = (char *)calloc(30, sizeof(char));
            for (int CheckCharacter = 0; CheckCharacter < 30; CheckCharacter++)
            {

                if (TempArray[CharacterOfRow] == ',')
                {
                    CharacterOfRow++;
                    CheckCharacter = CheckCharacter - 1;
                    continue;
                }
                if (TempArray[CharacterOfRow] == '.')
                {
                    (pMain->data)[CheckCharacter] = '_';
                    CharacterOfRow++;
                    continue;
                }

                if (TempArray[CharacterOfRow] == '\n')
                {

                    break;
                }
                (pMain->data)[CheckCharacter] = TempArray[CharacterOfRow];
                CharacterOfRow++;
            }
            if (First == NULL)
            {
                L_CreateTheFirstPointerOfList(&pMain, &First, &Last);
            }
            else
            {
                L_CreateTheElementOfList(&pMain, &Last);
            }
            if (TempArray[CharacterOfRow] == '\n')
            {
                break;
            }
        }
    }
    fclose(pFileFinal);
}

void STL_SaveDataIO(void)
{
    LinkList *pMain, *pNext;
    pMain = FirstFinal;
    while (pMain != NULL)
    {
        if ((strcmp(pMain->data, "LD") == 0))
        {
            pMain = pMain->next;
            LinkList *check = pMain->next;
            if ((strcmp(pMain->data, "ED") != 0) || (strcmp(pMain->data, "EU") != 0))
                H_InsertFunction(pMain->data);
        }
        else if ((strcmp(pMain->data, "O") == 0))
        {
            pMain = pMain->next;
            LinkList *check = pMain->next;
            if ((strcmp(pMain->data, "ED") != 0) || (strcmp(pMain->data, "EU") != 0))
                H_InsertFunction(pMain->data);
        }
        else if ((strcmp(pMain->data, "A") == 0))
        {
            pMain = pMain->next;
            LinkList *check = pMain->next;
            if ((strcmp(pMain->data, "ED") != 0) || (strcmp(pMain->data, "EU") != 0))
                H_InsertFunction(pMain->data);
        }
        else if ((strcmp(pMain->data, "=") == 0))
        {
            pMain = pMain->next;
            H_InsertFunction(pMain->data);
        }
        else if ((strncmp(pMain->data, "CT", 2) == 0))
        {
            pMain = pMain->next;
            H_InsertFunction(pMain->data);
        }
        else if ((strncmp(pMain->data, "TO", 2) == 0))
        {
            pMain = pMain->next;
            H_InsertFunction(pMain->data);
        }
        else if ((strncmp(pMain->data, "MOV", 3) == 0))
        {
            pMain = pMain->next;
            H_InsertFunction(pMain->data);
        }
        pMain = pMain->next;
    }
}

void STL_EditList(void)
{
    LinkList *pMainOfFirst, *pMainOfFirstFinal;
    pMainOfFirst = First;
    FirstFinal = NULL;
    int SizeOfToken = 0; // Đếm size token
    int CheckClose = 0;  // Xác định tiếp điểm thường đóng
    while (pMainOfFirst != NULL)
    {
        char *token = strtok(pMainOfFirst->data, " ");
        while (token != NULL)
        {
            // tính kích  thước token để tạo vùng nhớ cho  dữ liệu của phần tử mới
            SizeOfToken = strlen(token);
            // cấp phát vùng nhớ cho phần tử
            pMainOfFirstFinal = (LinkList *)malloc(sizeof(LinkList));

            pMainOfFirstFinal->next = NULL;
            if (CheckClose == 0) // Nếu ko phải là  tiếp điểm thường đóng  ta copy token vào phần tử
            {
                pMainOfFirstFinal->data = (char *)calloc(SizeOfToken, sizeof(char));
                strcpy(pMainOfFirstFinal->data, token);
            }
            else // Nếu là tiếp điểm thường  đóng
            {
                pMainOfFirstFinal->data = (char *)calloc(SizeOfToken, sizeof(char));
                strcpy(pMainOfFirstFinal->data, token);
                // Chèn tín hiệu thường đóng vào bảng
                H_InsertFunction(pMainOfFirstFinal->data);
                // Chèn tín hiệu thường đóng vào  Link list (!I0_0)
                pMainOfFirstFinal->data = MS_StrAllocAndAppend("!", pMainOfFirstFinal->data);
                CheckClose = 0;
            }
            /// Lưu địa chỉ con trỏ FirstFinal và con trỏ LastFinal
            if (FirstFinal == NULL)
            {
                L_CreateTheFirstPointerOfList(&pMainOfFirstFinal, &FirstFinal, &LastFinal);
            }
            else
            {
                L_CreateTheElementOfList(&pMainOfFirstFinal, &LastFinal);
            }
            // xác định xem có phải tiếp điểm thường đóng không bằng cách kiểm tra câu lệnh là sau đó gán b=1 để lần sau thêm 'H'
            if (strncmp(token, "LDN", 3) == 0 || strncmp(token, "AN", 2) == 0 || strncmp(token, "ON", 2) == 0)
            {
                CheckClose = 1;
            }

            // Nếu là xung sườn lên ta thêm "sl" vào sau biến bằng cách tạo 1 phần tử mới thế vào vị trí của phần tử cũ và xóa phần tử cũ đi
            if (strncmp(token, "EU", 2) == 0)
            {
                LinkList *temp1, *temp2, *temp3;
                temp1 = pMainOfFirstFinal->prev;
                H_InsertFunction(temp1->data);
                temp2 = temp1->prev; // temp1 = i0.0 => temp2 = ld
                static int demEU = 0;
                char *edge = "sl";
                char buffer[] = "";
                sprintf(buffer, "%d", demEU);
                edge = MS_StrAllocAndAppend(edge, buffer);
                char *TempArray2 = MS_StrAllocAndAppend(temp1->data, edge);
                int SizeOfTempArray2 = strlen(TempArray2);
                temp3 = (LinkList *)malloc(sizeof(LinkList));
                // Thay r3 vào r1
                temp3->next = temp1->next;
                temp3->prev = temp1->prev;
                temp2->next = temp3;
                temp3->data = (char *)calloc(SizeOfTempArray2, sizeof(char));
                for (int i = 0; i <= SizeOfTempArray2; i++)
                {
                    (temp3->data)[i] = TempArray2[i];
                }
                free(temp1);
                demEU++;
            }
            // Nếu là xung sườn xuống  ta thêm "sx" vào sau biến bằng cách tạo 1 phần tử mới thế vào vị trí của phần tử cũ và xóa phần tử cũ đi
            if (strncmp(token, "ED", 2) == 0)
            {
                LinkList *temp1, *temp2, *temp3;
                temp1 = pMainOfFirstFinal->prev;
                H_InsertFunction(temp1->data);
                temp2 = temp1->prev; // temp1 = i0.0 => temp2 = ld
                static int demED = 0;
                char *edge = "sl";
                char buffer[] = "";
                sprintf(buffer, "%d", demED);
                edge = MS_StrAllocAndAppend(edge, buffer);
                char *TempArray2 = MS_StrAllocAndAppend(temp1->data, edge);
                int SizeOfTempArray2 = strlen(TempArray2);
                temp3 = (LinkList *)malloc(sizeof(LinkList));
                // Thay r3 vào r1
                temp3->next = temp1->next;
                temp3->prev = temp1->prev;
                temp2->next = temp3;
                temp3->data = (char *)calloc(SizeOfTempArray2, sizeof(char));
                for (int i = 0; i <= SizeOfTempArray2; i++)
                {
                    (temp3->data)[i] = TempArray2[i];
                }
                free(temp1);
                demED++;
            }

            token = strtok(NULL, " ");
        }
        pMainOfFirst = pMainOfFirst->next;
    }
}

void STL_SplitBranch(void)
{
    LinkList *pCheck, *pMain, *pTemp;
    LinkList *pNext, *pNext1, *pPrev, *pPrev1;
    int CoutOpen = 0;  // Đếm số "("
    int CoutClose = 0; // Đếm số ")"
    pCheck = FirstFinal;
    while (pCheck != NULL)
    {
        if (pCheck == FirstFinal) // Thêm ký tự '(' vào đầu
        {
            pMain = (LinkList *)malloc(sizeof(LinkList));
            pMain->data = (char *)calloc(2, sizeof(char));
            pMain->data = "(\0";
            CoutOpen++;
            L_RecreateTheFirstElement(&pMain, &pCheck);
            FirstFinal = pMain;
            pNext = pCheck->next;
            pNext1 = pNext->next;
            if (strcmp(pNext1->data, "NOT") == 0)
            {
                pTemp = (LinkList *)malloc(sizeof(LinkList));
                pTemp->data = (char *)calloc(4, sizeof(char));
                pTemp->data = "not\0";
                L_RecreateTheFirstElement(&pTemp, &pMain);
                FirstFinal = pTemp;
            }
        }
        else
        {
            if ((strcmp(pCheck->data, "ALD") == 0) || (strcmp(pCheck->data, "OLD") == 0) || (strcmp(pCheck->data, "TON") == 0) || (strcmp(pCheck->data, "TONR") == 0) || (strcmp(pCheck->data, "TOF") == 0) || (strcmp(pCheck->data, "CTU") == 0) || (strcmp(pCheck->data, "CTD") == 0) || (strcmp(pCheck->data, "CTUD") == 0))
            {
                if ((strcmp(pCheck->data, "ALD") == 0) || (strcmp(pCheck->data, "OLD") == 0)) // not của 1 nhánh
                {
                    pNext = pCheck->next;
                    if (strcmp(pNext->data, "NOT") == 0)
                    {
                        pMain = (LinkList *)malloc(sizeof(LinkList));
                        pMain->data = (char *)calloc(2, sizeof(char));
                        pMain->data = "!\0";
                        L_InsertNextElement(&pCheck, &pMain, &pNext);
                    }
                }
                pPrev = pCheck->prev;
                if ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0)) // Đây ko phải nhánh lớn
                {
                    pMain = (LinkList *)malloc(sizeof(LinkList));
                    pMain->data = (char *)calloc(2, sizeof(char));
                    pMain->data = ")\0";
                    CoutClose++;
                    L_InsertPrevElement(&pCheck, &pMain);
                }
            }
            else if ((strcmp(pCheck->data, "=") == 0))
            {
                if (CoutOpen > CoutClose)
                {
                    pMain = (LinkList *)malloc(sizeof(LinkList));
                    pMain->data = (char *)calloc(2, sizeof(char));
                    pMain->data = ")\0";
                    CoutClose++;
                    L_InsertPrevElement(&pCheck, &pMain);
                }
            }
            else if ((strncmp(pCheck->data, "LD", 2) == 0) || (strncmp(pCheck->data, "LDN", 3) == 0))
            {
                pPrev = pCheck->prev;
                pPrev1 = pPrev->prev;
                if ((strcmp(pPrev1->data, "=") == 0) || (strcmp(pPrev->data, "N") == 0) || (strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0))
                {
                    pMain = (LinkList *)malloc(sizeof(LinkList));
                    pMain->data = (char *)calloc(2, sizeof(char));
                    pMain->data = "(\0";
                    CoutOpen++;
                    L_InsertPrevElement(&pCheck, &pMain);
                }
                else
                {
                    if (CoutOpen == CoutClose)
                    {
                        pMain = (LinkList *)malloc(sizeof(LinkList));
                        pMain->data = (char *)calloc(2, sizeof(char));
                        pMain->data = "(\0";
                        CoutOpen++;
                        L_InsertPrevElement(&pCheck, &pMain);
                    }
                    else
                    {
                        pMain = (LinkList *)malloc(sizeof(LinkList));
                        pMain->data = (char *)calloc(2, sizeof(char));
                        pMain->data = ")\0";
                        CoutOpen++;
                        L_InsertPrevElement(&pCheck, &pMain);
                        pMain = (LinkList *)malloc(sizeof(LinkList));
                        pMain->data = (char *)calloc(2, sizeof(char));
                        pMain->data = "(\0";
                        CoutClose++;
                        L_InsertPrevElement(&pCheck, &pMain);
                    }
                }
            }
            else if ((strncmp(pCheck->data, "A", 1) == 0) || (strncmp(pCheck->data, "O", 1) == 0) || (strncmp(pCheck->data, "AN", 2) == 0) || (strncmp(pCheck->data, "ON", 2) == 0))
            {
                if ((strncmp(pCheck->data, "AW", 2) == 0) || (strncmp(pCheck->data, "OW", 2) == 0))
                {
                    // Với trường hợp các câu lệnh so sánh OW , OW
                    pNext = pCheck->next;  // C1
                    pNext1 = pNext->next;  // 5
                    pNext1 = pNext1->next; // NOT
                    pPrev = pCheck->prev;  // 3
                }
                else
                {
                    // câu lệnh thường
                    pNext = pCheck->next; // I0_1
                    pNext1 = pNext->next; // NOT
                    pPrev = pCheck->prev; // I0_0
                }

                if (strcmp(pNext1->data, "NOT") == 0) // NOT của 1 tập hợp các biến
                {
                    int check_nhanh = 0;
                    while (strcmp(pPrev->data, "(") != 0) // Tìm vị trí đầu của nhánh
                    {
                        if ((strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0)) // nếu đâu là not của (    nhánh và các biến )
                        {
                            check_nhanh = 1;
                            break;
                        }
                        pPrev = pPrev->prev;
                    }
                    if (pPrev != FirstFinal)
                    {
                        if (check_nhanh == 0)
                        {
                            pPrev1 = pPrev->prev;
                            if ((strcmp(pPrev1->data, ")") != 0) && (strcmp(pPrev1->data, "1not") != 0))
                            {
                                pMain = (LinkList *)malloc(sizeof(LinkList));
                                pMain->data = (char *)calloc(4, sizeof(char));
                                pMain->data = "not\0";
                                L_InsertPrevElement(&pPrev, &pMain);
                            }
                            else
                            {
                                pMain = (LinkList *)malloc(sizeof(LinkList));
                                pMain->data = (char *)calloc(5, sizeof(char));
                                pMain->data = "1not\0";
                                L_InsertPrevElement(&pPrev, &pMain);
                            }
                        }
                        else
                        {

                            pNext1->data = MS_StrAllocAndAppend("T_", pNext1->data);
                        }
                    }
                    else
                    {
                        pMain = (LinkList *)malloc(sizeof(LinkList));
                        pMain->data = (char *)calloc(4, sizeof(char));
                        pMain->data = "not\0";
                        L_RecreateTheFirstElement(&pMain, &pPrev);
                        FirstFinal = pMain;
                    }
                }
            }
            else if ((strncmp(pCheck->data, "MOV", 3) == 0))
            {
                pPrev = pCheck->prev;
                if ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0) && (strcmp(pPrev->data, "AENO") != 0))
                {
                    pMain = (LinkList *)malloc(sizeof(LinkList));
                    pMain->data = (char *)calloc(2, sizeof(char));
                    pMain->data = ")\0";
                    CoutClose++;
                    L_InsertPrevElement(&pCheck, &pMain);
                }
            }
        }
        pCheck = pCheck->next;
    }
}

void STL_InsertListToFileData(void)
{
    LinkList *pMain, *pNext, *pNext1, *pPrev, *pPrev1;
    FILE *pFile;
    pFile = fopen("build/DataPLC.c", "a");
    fprintf(pFile, "while(1)\n{\n");
    fprintf(pFile, "read_Pin_Input();\n");
    char *OutString;
    char *InsertOpeningBracket = "(";
    char *InsertClosingBracket = ")";
    char *InsertMul = "*";
    char *Insertplus = "+";
    char *InsertQuestionMark = "?";
    int CheckEndNetWork = 0; // Kiểm tra xem hết 1 network chưa
    int CheckBigBranch = 0;
    char *OutCheckBigBranch = ""; // Kiểm tra xem có nhánh lớn trước đó không
    char *OutStringLP = "";
    int CountQuestionMark = 0; // Đếm số dấu "?"
    pMain = FirstFinal;
    int CountNetWork = 0;
    int checkTimer = 1;
    int debug_count =0;
    while (pMain != NULL)
    {
        printf("%s-",pMain->data);
        pMain = pMain->next;
    }
    pMain =FirstFinal;
    while (pMain != NULL)
    {
        debug_count++;

        if (CheckEndNetWork == 0) // Chưa kết thúc 1 network
        {
            char buffer[50];
            CountNetWork++;
            sprintf(buffer, "/*--------------NetWork %d -----------*/\n\n", CountNetWork);
            fputs(buffer, pFile);
            OutString = "";
            CheckEndNetWork = 1;
        }
        if (strcmp(pMain->data, "N") == 0)
        {

            CheckEndNetWork = 0;
            pMain = pMain->next;
            continue;
        }

        if (strcmp(pMain->data, "(") == 0)
        {
            pPrev = pMain->prev;
            if (pPrev != NULL)
            {
                pPrev1 = pPrev->prev;
                if ((strcmp(pPrev->data, "not") != 0) && (strcmp(pPrev->data, "1not") != 0))
                {
                    if (strncmp(pPrev->data, ")", 1) == 0) // Nếu trước  đó sẽ có dấu ")" thì thêm "?" vào giữa 2 dấu
                    {
                        char *tempOUT = "";
                        tempOUT = MS_StrAllocAndAppend(tempOUT, OutString);
                        int a = strlen(tempOUT);
                        if (((int)tempOUT[a - 1]) != 63) // == "?"
                        {
                            OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                            CountQuestionMark++;
                            OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                            pMain = pMain->next;
                            continue;
                        }
                    }
                    else if ((strcmp(pPrev->data, "NOT") == 0))
                    {
                        char *tempOUT = "";
                        tempOUT = MS_StrAllocAndAppend(tempOUT, OutString);
                        int a = strlen(tempOUT);
                        if (((int)tempOUT[a - 1]) != 63) // == "?"
                        {
                            OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                            CountQuestionMark++;
                            OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                            pMain = pMain->next;
                            continue;
                        }
                    }
                    else
                    {
                        OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                    }
                }
                else if ((strcmp(pPrev->data, "1not") == 0)) // trước đó có dấu "("
                {
                    char *tempOUT = "";
                    tempOUT = MS_StrAllocAndAppend(tempOUT, OutString);
                    int a = strlen(tempOUT);
                    if (((int)tempOUT[a - 1]) != 63) // == "?"
                    {
                        OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                        CountQuestionMark++;
                        OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                        pMain = pMain->next;
                        continue;
                    }
                }
                else if ((strcmp(pPrev->data, "not") == 0))
                {
                    int check_big = 0;
                    pPrev = pPrev->prev;
                    if (pPrev != NULL)
                    {
                        pPrev = pPrev->prev;
                        while (pPrev)
                        {
                            if ((strcmp(pPrev->data, "OLD") == 0) || (strcmp(pPrev->data, "ALD") == 0)) // Nhánh lớn
                            {
                                check_big = 1;
                                break;
                            }
                            pPrev = pPrev->prev;
                        }
                        if (check_big)
                        {
                            char *tempOUT = "";
                            tempOUT = MS_StrAllocAndAppend(tempOUT, OutString);
                            int a = strlen(tempOUT);
                            if (((int)tempOUT[a - 1]) != 63) // == "?"
                            {
                                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                                CountQuestionMark++;
                                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                                pMain = pMain->next;
                                continue;
                            }
                        }
                        else
                        {
                            OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                        }
                    }
                    else
                    {
                        OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                    }
                }
                else
                {
                    OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                }
            }
            else
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
            }
        }
        else if (strcmp(pMain->data, ")") == 0)
        {
            pNext = pMain->next;
            pNext1 = pNext->next;
            if ((strcmp(pNext->data, "not") != 0) && (strcmp(pNext->data, "1not") != 0))
            {
                if (strcmp(pNext->data, "(") == 0) // Nếu sau đó sẽ có dấu "(" thì thêm "?" vào giữa 2 dấu
                {
                    OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                    OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                    CountQuestionMark++;
                    OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                    pMain = pNext->next;
                    continue;
                }
                else
                {
                    OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                }
            }
            else if (strcmp(pNext->data, "1not") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;

                while ((strncmp(pNext1->data, "1not", 3) == 0))
                {

                    pNext1 = pNext1->next; // Bỏ qua các phần tử 1not và not
                }

                pMain = pNext1;
                continue;
            }
            else
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }
        }
        else if ((strcmp(pMain->data, "LD") == 0) || (strcmp(pMain->data, "LDN") == 0))
        {
            pPrev = pMain->prev;
            pPrev1 = pPrev->prev;
            pNext = pMain->next;
            pNext1 = pNext->next;
            if (pPrev1 != NULL) // dảm bảo kể cả à đâu chương trình mà có not thì vẫn được xét thêm "!"
            {
                if ((strcmp(pPrev1->data, "not") == 0) || (strcmp(pPrev1->data, "1not") == 0))
                {
                    OutString = MS_StrAllocAndAppend(OutString, "!");
                    OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                    pPrev = pPrev1->prev;
                    if (pPrev != NULL)
                    {
                        while ((strcmp(pPrev->data, "not") == 0) || (strcmp(pPrev->data, "1not") == 0))
                        {
                            OutString = MS_StrAllocAndAppend(OutString, "!");
                            OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                            pPrev = pPrev->prev;
                            if (pPrev == NULL)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            OutString = MS_StrAllocAndAppend(OutString, pNext->data);
            pMain = pNext1;
            continue;
        }
        else if ((strcmp(pMain->data, "EU") == 0))
        {
            static int countEU = 0;
            char *temp = "";
            char *temp1 = "";
            char *token;
            char *token1;
            int size_old = 0;
            int size_new = 0;
            temp = MS_StrAllocAndAppend(temp, pNext->data); // I0_0sl
            size_old = strlen(temp);
            temp1 = MS_StrAllocAndAppend(temp1, pNext->data); // !I0_0sl1
            token = strtok(temp, "!");
            size_new = strlen(token);
            if (size_new == size_old)
            {
                char *tokenEU = strtok(temp, "s"); // I0_0
                fprintf(pFile, " volatile uint8_t %s = 0;\n", pNext->data);
                fprintf(pFile, "volatile static uint8_t checkEU%d = 1 ;\n", countEU);
                fprintf(pFile, "if ( !(%s) )\n{\n	checkEU%d = 0 ;\n}\n", tokenEU, countEU);
                fprintf(pFile, "if ( (!checkEU%d) && (%s) ) \n{\ncheckEU%d = 1 ; \n%s = 1 ;\n}\n", countEU, tokenEU, countEU, pNext->data);
                countEU++;
                pMain = pMain->next;
                continue;
            }
            else
            {
                token1 = strtok(temp1, "!");
                fprintf(pFile, " volatile uint8_t %s = 1;\n", token1);
                char *tokenEU = strtok(temp, "s"); // I0_0
                fprintf(pFile, "volatile static uint8_t checkEU%d = 1 ;\n", countEU);
                fprintf(pFile, "if ( !(%s) )\n{\n	checkEU%d = 0 ;\n}\n", tokenEU, countEU);
                fprintf(pFile, "if ( (!checkEU%d) && (%s) ) \n{\ncheckEU%d = 1 ; \n%s = 0 ;\n}\n", countEU, tokenEU, countEU, token1);
                countEU++;
                pMain = pMain->next;
                continue;
            }
        }
        else if ((strcmp(pMain->data, "ED") == 0))
        {
            static int countED = 0;
            char *temp = "";
            char *temp1 = "";
            char *token;
            char *token1;
            int size_old = 0;
            int size_new = 0;
            temp = MS_StrAllocAndAppend(temp, pNext->data); // I0_0sl
            size_old = strlen(temp);
            temp1 = MS_StrAllocAndAppend(temp1, pNext->data); // !I0_0sl1
            token = strtok(temp, "!");
            size_new = strlen(token);
            if (size_new == size_old)
            {

                char *tokenED = strtok(temp, "s");
                fprintf(pFile, " volatile uint8_t %s = 1;\n", pNext->data);
                fprintf(pFile, "volatile static uint8_t checkED%d = 0 ;\n", countED);
                fprintf(pFile, "if (%s)\n{\n	checkEU%d = 0 ;\n}\n", tokenED, countED);
                fprintf(pFile, "if ( (!checkEU%d) && (!(%s)) ) \n{\ncheckEU%d = 1 ; \n%s = 1 ;\n}\n", countED, tokenED, countED, pNext->data);
                countED++;
                pMain = pMain->next;
                continue;
            }
            else
            {
                token1 = strtok(temp1, "!");
                fprintf(pFile, " volatile uint8_t %s = 1;\n", token1);
                char *tokenED = strtok(temp, "s");
                fprintf(pFile, "volatile static uint8_t checkED%d = 0 ;\n", countED);
                fprintf(pFile, "if (!(%s))\n{\n	checkEU%d = 0 ;\n}\n", tokenED, countED);
                fprintf(pFile, "if ( (!checkEU%d) && (!(%s)) ) \n{\ncheckEU%d = 1 ; \n%s = 0 ;\n}\n", countED, tokenED, countED, token1);
                countED++;
                pMain = pMain->next;
                continue;
            }
        }
        else if ((strcmp(pMain->data, "A") == 0) || (strcmp(pMain->data, "AN") == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            pPrev = pMain->prev; // LinkList *pPREV_temp = pPrev;
            pNext = pMain->next;
            pNext1 = pNext->next;
            pPrev1 = pPrev->prev;
            if ((strcmp(pPrev1->data, "O") == 0) || (strcmp(pPrev1->data, "ON") == 0))
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket); // (I0+I1)*I2
            }
            CheckNOT(&pMain, pNext, pNext1, &OutString, InsertMul);

            while ((strcmp(pMain->data, "A") == 0) || (strcmp(pMain->data, "AN") == 0))
            {

                pNext = pMain->next;
                pNext1 = pNext->next;
                CheckNOT(&pMain, pNext, pNext1, &OutString, InsertMul);
            }
            if ((strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0)) // nhánh nhân hoặc cộng với biến
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }
            if (strcmp(pMain->data, "(") == 0)
            {
                if ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0))
                {
                    OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                }

                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                pMain = pMain->next;
            }
            if (((strcmp(pMain->data, "O") == 0) || (strcmp(pMain->data, "ON") == 0)) && ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0)))
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }

            if ((strcmp(pMain->data, ")") == 0) && ((strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0)))
            {
                pMain = pMain->next;
            }
            if (((strcmp(pMain->data, "ALD") == 0) || (strcmp(pMain->data, "OLD") == 0)) && CheckBigBranch == 1)
            {
                OutString = MS_AddParenthesesIfMissing(OutString);
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                CheckBigBranch = 0;
                CountQuestionMark++;
                OutCheckBigBranch = "";
            }

            continue;
        }
        else if ((strcmp(pMain->data, "O") == 0) || (strcmp(pMain->data, "ON") == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            pPrev = pMain->prev;
            pNext = pMain->next;
            pNext1 = pNext->next;
            pPrev1 = pPrev->prev;
            if ((strcmp(pPrev1->data, "A") == 0) || (strcmp(pPrev1->data, "AN") == 0))
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }
            CheckNOT(&pMain, pNext, pNext1, &OutString, Insertplus);
            while ((strcmp(pMain->data, "O") == 0) || (strcmp(pMain->data, "ON") == 0))
            {

                pNext = pMain->next;
                pNext1 = pNext->next;
                CheckNOT(&pMain, pNext, pNext1, &OutString, Insertplus);
            }
            if ((strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0)) // nhánh nhân hoặc cộng với biến
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }
            if (strcmp(pMain->data, "(") == 0)
            {
                if ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0))
                {
                    OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                }

                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                pMain = pMain->next;
            }
            if (((strcmp(pMain->data, "A") == 0) || (strcmp(pMain->data, "AN") == 0)) && ((strcmp(pPrev->data, "ALD") != 0) && (strcmp(pPrev->data, "OLD") != 0)))
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            }

            if ((strcmp(pMain->data, ")") == 0) && ((strcmp(pPrev->data, "ALD") == 0) || (strcmp(pPrev->data, "OLD") == 0)))
            {
                pMain = pMain->next;
            }

            if (((strcmp(pMain->data, "ALD") == 0) || (strcmp(pMain->data, "OLD") == 0)) && CheckBigBranch == 1)
            {
                OutString = MS_AddParenthesesIfMissing(OutString);
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                CheckBigBranch = 0;
                CountQuestionMark++;
                OutCheckBigBranch = "";
            }

            continue;
        }
        else if (strcmp(pMain->data, "ALD") == 0)
        {
            CheckCountQuestionMark(&CountQuestionMark, &CheckBigBranch, &OutString, &OutCheckBigBranch);
            AddPlusToOutString(&pMain, &OutString, &CountQuestionMark, InsertMul);
            if (strcmp(pMain->data, "(") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                pMain = pMain->next;

                continue;
            }
            pNext = pMain->next;
            if ((CheckBigBranch == 1) && (((strncmp(pMain->data, "OLD", 1) == 0) || (strncmp(pMain->data, "ALD", 1) == 0)) || ((strncmp(pNext->data, "OLD", 1) == 0) || (strncmp(pNext->data, "ALD", 1) == 0))))
            {
                OutString = MS_AddParenthesesIfMissing(OutString);
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                CheckBigBranch = 0;
                CountQuestionMark++;
                OutCheckBigBranch = "";
            }
            continue;
        }
        else if (strcmp(pMain->data, "OLD") == 0)
        {
            CheckCountQuestionMark(&CountQuestionMark, &CheckBigBranch, &OutString, &OutCheckBigBranch);
            AddPlusToOutString(&pMain, &OutString, &CountQuestionMark, Insertplus);

            if (strcmp(pMain->data, "(") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                pMain = pMain->next;

                continue;
            }
            pNext = pMain->next;
            if ((CheckBigBranch == 1) && (((strncmp(pMain->data, "OLD", 1) == 0) || (strncmp(pMain->data, "ALD", 1) == 0)) || ((strncmp(pNext->data, "OLD", 1) == 0) || (strncmp(pNext->data, "ALD", 1) == 0))))
            {
                OutString = MS_AddParenthesesIfMissing(OutString);
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                CheckBigBranch = 0;
                CountQuestionMark++;
                OutCheckBigBranch = "";
            }
            continue;
        }
        else if (strcmp(pMain->data, "T_NOT") == 0)
        {
            OutString = MS_StrAllocAndAppend(InsertOpeningBracket, OutString);
            OutString = MS_StrAllocAndAppend("!", OutString);
            OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
            pMain = pMain->next;
            ;
            if (strcmp(pMain->data, ")") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertClosingBracket);
                pMain = pMain->next;
            }
            if (strcmp(pMain->data, "(") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
                OutString = MS_StrAllocAndAppend(OutString, InsertOpeningBracket);
                pMain = pMain->next;
                continue;
            }
            if (strcmp(pMain->data, "not") == 0)
            {
                OutString = MS_StrAllocAndAppend(OutString, InsertQuestionMark);
                CountQuestionMark++;
            }

            if (((strcmp(pMain->data, "ALD") == 0) || (strcmp(pMain->data, "OLD") == 0)) && CheckBigBranch == 1)
            {
                OutString = MS_AddParenthesesIfMissing(OutString);
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                CheckBigBranch = 0;
                CountQuestionMark++;
                OutCheckBigBranch = "";
            }
            continue;
        }
        else if (strcmp(pMain->data, "=") == 0) // q0.1=(A*B)\n
        {

            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            else
                OutString = MS_AddParenthesesIfMissing(OutString);
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");
            OutString = MS_StrAllocAndAppend(pMain->data, OutString);
            pMain = pMain->next;
            OutString = MS_StrAllocAndAppend(pMain->data, OutString);
            OutCheckBigBranch = "";
            fputs(OutString, pFile);
            fprintf(pFile, "if (%s>0)\n{\n%s = 1;\n}\nelse\n{\n%s = 0 ;\n}\n", pMain->data, pMain->data, pMain->data);
        }
        else if (strcmp(pMain->data, "CTU") == 0)
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            SetupCounterUpOrDown(&pMain, OutString, pFile);
            // pMAin : 5
            pPrev = pMain->prev; // C1
            fprintf(pFile, "if (reset%s) \n{\ncount%s = 0 ;\n%s = 0 ; \nstart%s = 1 ;\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else\n{\nif (vao%s)\n{\n ", pPrev->data);
            fprintf(pFile, "if ( ( (check%s ==1 ) || ( start%s ==1 ) ) && (count%s <= 4294967295)  )\n", pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "{\ncount%s ++ ;\nstart%s = 0 ;\n}\nif (count%s >= dat%s )\n{%s =1 ;\n}\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else \n{\n check%s = 1 ;\n}\n}\n", pPrev->data);
        }
        else if (strcmp(pMain->data, "CTD") == 0)
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            SetupCounterUpOrDown(&pMain, OutString, pFile);
            // pMAin : 12
            pPrev = pMain->prev; // c2
            fprintf(pFile, "if (reset%s)\n{\nnho%s = dat%s ;\ncount%s = dat%s ;\n%s = 0 ;\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else \n{\nif (vao%s)\n{\n", pPrev->data);
            fprintf(pFile, "if ( ( (checkC2 ==1 ) || ( start%s ==1 ))  && ( nho%s > 0 ))\n{\nnho%s --  ;\ncount%s = nho%s ;\nstart%s = 0 ;\n}\n ", pPrev->data, pPrev->data, pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "if (count%s <= 0 )\n{\n%s = 1 ;\n}\n}\nelse \n{\ncheck%s = 1 ;\n}\n}\n", pPrev->data, pPrev->data, pPrev->data);
        }
        else if (strcmp(pMain->data, "CTUD") == 0)
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            SetupCounterUpDown(&pMain, OutString, pFile);
            // pMAin = 12 ;
            pPrev = pMain->prev; // C3
            fprintf(pFile, "if (reset%s)\n{\nnho%s = 0 ;\ncount%s = 0 ;\n%s = 0 ;\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else \n{\n\nif (tang%s)\n{\n", pPrev->data);
            fprintf(pFile, "if ( ( (check%stang ==1 ) || ( start%stang ==1 ))  && ( count%s <= 4294967295 ))\n ", pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "{\nnho%s ++  ;\ncount%s = nho%s ;\nstart%stang = 0 ;\n}\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else \n{\ncheck%stang = 1 ;\n}\n", pPrev->data);
            fprintf(pFile, "if (giam%s)\n{\n", pPrev->data);
            fprintf(pFile, "if ( ( (check%sgiam ==1 ) || ( start%sgiam ==1 ))  && ( nho%s > 0 )) \n", pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "{\nnho%s --  ;\ncount%s = nho%s ;\nstart%sgiam = 0 ;\n}\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data);
            fprintf(pFile, "else\n{\ncheck%sgiam = 1 ;\n}\n ", pPrev->data);
            fprintf(pFile, "if (count%s >=dat%s )\n{%s = 1 ;\n}\nelse\n{\n%s = 0 ;\n}\n}\n", pPrev->data, pPrev->data, pPrev->data, pPrev->data);
        }
        else if (strcmp(pMain->data, "TON") == 0)
        {
            if (checkTimer == 1)
            {
                pFileTimer = fopen("Timer.txt", "w");
                fprintf(pFileTimer, "void initTimer(void)\n");
                fprintf(pFileTimer, "{\n");
                checkTimer = 0;
            }
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            else
                OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            InsertTimer(pMain, &CountTimer, pFileTimer);
            // main -> data : TON
            char *equal = " = ";
            char *temparray = "";
            char *temparray1 = "";
            pMain = pMain->next;                                     // T97
            temparray = MS_StrAllocAndAppend(temparray, pMain->data);   // T97
            temparray1 = MS_StrAllocAndAppend(temparray1, pMain->data); // T97
            fprintf(pFile, "if (!%sreset )\n{\n ", temparray1);
            pMain->data = MS_StrAllocAndAppend("vao", pMain->data);   // int vaoT97
            pMain->data = MS_StrAllocAndAppend(pMain->data, equal);   // int vaoT97 =
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");      // (I0_1&I0_1) ;\n
            OutString = MS_StrAllocAndAppend(pMain->data, OutString); // int vaoT97 = (I0_1&I0_1) ;\n
            pMain = pMain->next;                                   // 20
            OutString = MS_StrAllocAndAppend(OutString, temparray);   // int vaoT97_ON = (I0_1&I0_1) ;\n int datT97_ON = 20
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");
            fputs(OutString, pFile);
            fprintf(pFile, "if ( vao%s  )\n{\n", temparray1);
            fprintf(pFile, "if ( count%s >= dat%s  ) \n%s = 1 ;\n", temparray1, temparray1, temparray1);
            fprintf(pFile, " \nif ((%s == 0) && (check%s ==0) )\n{\n", temparray1, temparray1);
            fprintf(pFile, "xTimerStart (handle_timerPLC[%d], portMAX_DELAY);\n", CountTimer - 1);
            fprintf(pFile, "check%s = 1;\n}\n}\n", temparray1);
            fprintf(pFile, "else \n");
            fprintf(pFile, "{\nif( check%s == 1 )\n{\n", temparray1);
            fprintf(pFile, "xTimerStop(handle_timerPLC[%d], portMAX_DELAY);\n", CountTimer - 1);
            fprintf(pFile, "check%s = 0 ;\n}\n%s=0;\ncount%s = 0 ;\n}\n", temparray1, temparray1, temparray1);
            fprintf(pFile, "\n}\n");
            fprintf(pFile, "else\n{\nxTimerStop(handle_timerPLC[%d], portMAX_DELAY);\ncount%s = 0;\n%s = 0 ;\ncheck%s= 0 ;}\n", CountTimer - 1, temparray1, temparray1, temparray1);
        }
        else if (strcmp(pMain->data, "TOF") == 0)
        {
            if (checkTimer == 1)
            {
                pFileTimer = fopen("Timer.txt", "w");
                fprintf(pFileTimer, "void initTimer(void)\n");
                fprintf(pFileTimer, "{\n");
                checkTimer = 0;
            }
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            else
                OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            InsertTimer(pMain, &CountTimer, pFileTimer);
            char *equal = " = ";
            char *temparray = "";
            char *temparray1 = "";
            pMain = pMain->next;                                     // T97
            temparray = MS_StrAllocAndAppend(temparray, pMain->data);   // T97
            temparray1 = MS_StrAllocAndAppend(temparray1, pMain->data); // T97
            fprintf(pFile, "if (!%sreset )\n{\n ", temparray1);
            pMain->data = MS_StrAllocAndAppend("vao", pMain->data);   // int vaoT97
            pMain->data = MS_StrAllocAndAppend(pMain->data, equal);   // int vaoT97 =
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");      // (I0_1&I0_1) ;\n
            OutString = MS_StrAllocAndAppend(pMain->data, OutString); // int vaoT97 = (I0_1&I0_1) ;\n
            pMain = pMain->next;                                   // 20
            OutString = MS_StrAllocAndAppend(OutString, temparray);   // int vaoT97_ON = (I0_1&I0_1) ;\n int datT97_ON = 20
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");
            fputs(OutString, pFile);
            fprintf(pFile, "if ( vao%s  )\n{\ncount%s = 0 ;\ncheck%sOn = 1 ;\n%s =1 ;\n if ( check%sOff == 1 ||  start%s == 1 )\n{\n", temparray1, temparray1, temparray1, temparray1, temparray1, temparray1);
            fprintf(pFile, "xTimerStop(handle_timerPLC[%d], portMAX_DELAY);\ncheck%sOff = 0 ;\nstart%s =0 \n }\n}\n", CountTimer - 1, temparray1, temparray1);
            fprintf(pFile, " else \n{\ncheck%sOff = 1 ;\nif ( check%sOn == 1 )\n{ \n", temparray1, temparray1);
            fprintf(pFile, "xTimerStart (handle_timerPLC[%d], portMAX_DELAY); \ncheck%sOn = 0  ; \n}\nif ( count%s >= dat%s ) \n{\n%s = 0 ;\n}\n}\n", CountTimer - 1, temparray1, temparray1, temparray1, temparray1);
            fprintf(pFile, "\n}\n");
            fprintf(pFile, "else\n{\nxTimerStop(handle_timerPLC[%d], portMAX_DELAY);\ncount%s = 0;\n%s = 0 ;}\n", CountTimer - 1, temparray1, temparray1);
        }
        else if (strcmp(pMain->data, "TONR") == 0)
        {
            if (checkTimer == 1)
            {
                pFileTimer = fopen("Timer.txt", "w");
                fprintf(pFileTimer, "void initTimer(void)\n");
                fprintf(pFileTimer, "{\n");
                checkTimer = 0;
            }
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            else
                OutString = MS_AddParenthesesIfMissing(OutString);
            CheckBigBranch = 0;
            CountQuestionMark = 0;
            if (strlen(OutCheckBigBranch) != 0)
            {
                OutString = MS_StrAllocAndAppend(OutCheckBigBranch, OutString);
                OutCheckBigBranch = "";
            }
            InsertTimer(pMain, &CountTimer, pFileTimer);
            char *equal = " = ";
            char *temparray = "";
            char *temparray1 = "";
            pMain = pMain->next;                                     // T97
            temparray = MS_StrAllocAndAppend(temparray, pMain->data);   // T97
            temparray1 = MS_StrAllocAndAppend(temparray1, pMain->data); // T97
            fprintf(pFile, "if (!%sreset )\n{\n ", temparray1);
            pMain->data = MS_StrAllocAndAppend("vao", pMain->data);   // int vaoT97
            pMain->data = MS_StrAllocAndAppend(pMain->data, equal);   // int vaoT97 =
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");      // (I0_1&I0_1) ;\n
            OutString = MS_StrAllocAndAppend(pMain->data, OutString); // int vaoT97 = (I0_1&I0_1) ;\n
            pMain = pMain->next;                                   // 20
            OutString = MS_StrAllocAndAppend(OutString, temparray);   // int vaoT97_ON = (I0_1&I0_1) ;\n int datT97_ON = 20
            OutString = MS_StrAllocAndAppend(OutString, " ;\n");
            fputs(OutString, pFile);
            fprintf(pFile, "if ( vao%s ) \n{\ncheck%sOn = 1 ;\nif ( check%sOff == 1 || start%s == 1 )\n", temparray1, temparray1, temparray1, temparray1);
            fprintf(pFile, "{ \nxTimerStart (handle_timerPLC[%d], portMAX_DELAY); \ncheck%sOff = 0 ; \nstart%s = 0 ;\n}\n", CountTimer - 1, temparray1, temparray1);
            fprintf(pFile, "if ( (count%s-checkCount%s) >= dat%s) \n{%s = 1 ;\ncheckCount%s = 0 ;\nxTimerStop(handle_timer[%d], portMAX_DELAY);\ncount%s = 0 ;\n}\n}\n", temparray1, temparray1, temparray1, temparray1, temparray1, CountTimer - 1, temparray1);
            fprintf(pFile, "else\n{check%sOff = 1 ;\nif( (!count%s) && (check%sOn == 1 )  )\ncheckCount%s = count%s ;\ncheck%sOn = 0 ;\n}\n", temparray1, temparray1, temparray1, temparray1, temparray1, temparray1);
            fprintf(pFile, "\n}\n");
            fprintf(pFile, "else\n{\nxTimerStop(handle_timerPLC[%d], portMAX_DELAY);\ncount%s = 0;\n%s = 0 ;\ncheck%sOff =1\n}\n", CountTimer - 1, temparray1, temparray1, temparray1);
        }
        else if (strcmp(pMain->data, "MOVB") == 0)
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertMov(&pMain, OutString, CountQuestionMark, pFile, MOVB_CHECK);
            continue;
        }
        else if (strcmp(pMain->data, "MOVW") == 0)
        {
            InsertMov(&pMain, OutString, CountQuestionMark, pFile, MOVW_CHECK);
            continue;
        }
        else if (strcmp(pMain->data, "MOVDW") == 0)
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertMov(&pMain, OutString, CountQuestionMark, pFile, MOVDW_CHECK);
            continue;
        }
        else if (strcmp(pMain->data, "R") == 0)
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            char *tempOutString = "";
            char *token;
            tempOutString = MS_StrAllocAndAppend(OutString, tempOutString);
            token = strtok(tempOutString, "=");
            pMain = pMain->next;
            if (strncmp(pMain->data, "T", 1) == 0)
            {
                fprintf(pFile, "%sreset = %s;\n", pMain->data, OutString);
                fprintf(pFile, "if (%sreset>0)\n{\n%sreset = 1;\n}\nelse\n{\n%sreset = 0 ;\n}\n", pMain->data, pMain->data, pMain->data);
            }
            else
            {
                pNext = pMain->next;
                if (token)
                {
                    fprintf(pFile, "if (%s)\n{\nmemset(&%s,0,%s)\n}\n", token, pMain->data, pNext->data);
                }
                else
                    fprintf(pFile, "if (%s)\n{\nmemset(&%s,0,%s)\n}\n", OutString, pMain->data, pNext->data);
            }
        }
        else if (strcmp(pMain->data, "S") == 0)
        {
            OutString = MS_AddParenthesesIfMissing(OutString);
            char *tempOutString = "";
            char *token;
            tempOutString = MS_StrAllocAndAppend(OutString, tempOutString);
            token = strtok(tempOutString, "=");
            pMain = pMain->next;
            pNext = pMain->next;
            if (token)
            {
                fprintf(pFile, "if (%s)\n{\nmemset(&%s,1,%s);\n}\n", token, pMain->data, pNext->data);
            }
            else
                fprintf(pFile, "if (%s)\n{\nmemset(&%s,1,%s);\n}\n", OutString, pMain->data, pNext->data);
        }
        else if ((strncmp(pMain->data, "AW", 2) == 0) || (strncmp(pMain->data, "OW", 2) == 0) || (strncmp(pMain->data, "LDW", 2) == 0)) // AW=
        {
            static int countTC = 0;
            char *token;
            pNext = pMain->next;
            pNext1 = pNext->next;
            token = strtok(pMain->data, "W"); // pMain : A
            token = strtok(NULL, "W");        // token : =
            if (strcmp(token, "=") == 0)
            {
                char *temp = "";
                temp = MS_StrAllocAndAppend(temp, pNext->data);
                fprintf(pFile, "volatile uint8_t temp%s_%d = 0 ;\n", pNext->data, countTC);
                fprintf(pFile, "if (count%s == %s )\n{\n", pNext->data, pNext1->data);
                fprintf(pFile, "temp%s_%d = 1 ;\n}\n", pNext->data, countTC);
                // free(pNext->data);
                sprintf(pNext->data, "temp%s_%d", temp, countTC);
                L_DeleteLinkList(&(pNext1), &(pNext1->prev), &(pNext1->next));
            }
            else if (strcmp(token, "<>") == 0)
            {
                char *temp = "";
                temp = MS_StrAllocAndAppend(temp, pNext->data);
                fprintf(pFile, "volatile uint8_t temp%s_%d = 0 ;\n", pNext->data, countTC);
                fprintf(pFile, "if (count%s !=%s )\n{\n", pNext->data, pNext1->data);
                fprintf(pFile, "temp%s_%d = 1 ;\n}\n", pNext->data, countTC);
                // free(pNext->data);
                sprintf(pNext->data, "temp%s_%d", temp, countTC);
                L_DeleteLinkList(&(pNext1), &(pNext1->prev), &(pNext1->next));
            }
            else
            {
                char *temp = "";
                temp = MS_StrAllocAndAppend(temp, pNext->data);
                fprintf(pFile, "volatile uint8_t temp%s_%d = 0 ;\n", pNext->data, countTC);
                fprintf(pFile, "if (count%s %s %s )\n{\n", pNext->data, token, pNext1->data);
                fprintf(pFile, "temp%s_%d = 1 ;\n}\n", pNext->data, countTC);
                // free(pNext->data);
                sprintf(pNext->data, "temp%s_%d", temp, countTC);
                L_DeleteLinkList(&(pNext1), &(pNext1->prev), &(pNext1->next));
            }
            countTC++;
            continue;
        }
        else if (strcmp(pMain->data, "LPS") == 0)
        {
            OutStringLP = "";
            OutStringLP = MS_StrAllocAndAppend(OutStringLP, OutString);
        }
        else if ((strcmp(pMain->data, "LPP") == 0) || (strcmp(pMain->data, "LRD") == 0))
        {
            OutString = "";
            OutString = MS_StrAllocAndAppend(OutString, OutStringLP);
        }
        else if ((strncmp(pMain->data, "ITD",3) == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertConvert(&pMain,OutString, CountQuestionMark, pFile, CONVERT_INT_TO_DINT);
            continue;
        }
        else if ((strncmp(pMain->data, "DTR",3) == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertConvert(&pMain,OutString, CountQuestionMark, pFile, CONVERT_DINT_TO_REAL);
            continue;
        }
        else if ((strncmp(pMain->data, "/R",2) == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertCaculate(&pMain,OutString, CountQuestionMark, pFile, CAL_DIV);
            continue;
        }
        else if ((strncmp(pMain->data, "*R",2) == 0))
        {
            char *tempCheckOutString = "";
            char *tokenCkeck;
            tempCheckOutString = MS_StrAllocAndAppend(OutString, tempCheckOutString);
            tokenCkeck = strtok(tempCheckOutString, "=");
            int sizetokenCkeck = strlen(tokenCkeck);
            int sizeOutString = strlen(OutString);
            if (sizetokenCkeck != sizeOutString)
            {
                OutString = "";
                OutString = MS_StrAllocAndAppend(OutString, tokenCkeck);
            }
            InsertCaculate(&pMain,OutString, CountQuestionMark, pFile, CAL_MUL);
            continue;
        }
        pMain = pMain->next;
    }
    fprintf(pFile, " write_Pin_Output();\n}\n}\n");
    if (pFileTimer)
    {
        fprintf(pFileTimer, "\n}\n");
        fclose(pFileTimer);
    }
    fclose(pFile);
}

void STL_FileDefineData(void)
{
    FILE *pFile = NULL;
    pFile = fopen("build/DataPLC.h", "w");
    // ghi tên file vào để tránh trùng lặp ở file khác
    if (!pFile)
        printf("Create file DataPLC.h failed \n");
    fprintf(pFile, "#ifndef INC_CSDL_H_\n#define INC_CSDL_H_\n");

    fprintf(pFile, "\n#include\"main.h\"\n");

    DefineRegionMemory(pFile, I_MEM, SUM_I); // I[3][8]

    DefineRegionMemory(pFile, Q_MEM, SUM_Q); // Q[3][8]

    DefineRegionMemory(pFile, M_MEM, SUM_M); //  M[2][8]

    DefineRegionMemory(pFile, AI_MEM, 1); //  u16_VM[8]

    DefineRegionMemory(pFile, u16_VM_MEM, 1); //  u32_VM[8]

    DefineRegionMemory(pFile, u32_VM_MEM, 1); //  u32_VM[8]

    DefineRegionMemory(pFile, f32_VM_MEM, 1); //  f32_VM[8]

    DefineIO(pFile, I_MEM, SUM_I);
    // Ix_y_PIN GPIO_PIN_k
    // Ix_y_PORT GPIOx

    DefineIO(pFile, Q_MEM, SUM_Q);
    // Qx_y_PIN GPIO_PIN_k
    // Qx_y_PORT GPIOx

    fprintf(pFile, "\nextern volatile uint16_t AI[%d];\n",SUM_AI);
    fprintf(pFile, "void read_Pin_Input(void);\n");
    fprintf(pFile, "void write_Pin_Output(void);\n");
    fprintf(pFile, "void Main_task(void *param) ;\n");
    if (CountTimer > 0)
    {
        fprintf(pFile, "void TimerCallBack(TimerHandle_t xTimer);\n");

        fprintf(pFile, "void initTimer(void);\n");
        fprintf(pFile, "extern TimerHandle_t  handle_timerPLC[%d];\n", CountTimer);
    }

    fprintf(pFile, "\n#endif /*INC_CSDL_H_*/");

    fclose(pFile);
}

void STL_FileData(void)
{
    LinkList *pMain = FirstFinal;
    FILE *pFile;
    system("mkdir build");
    pFile = fopen("build/DataPLC.c", "w");
    if (!pFile)
        printf("Create file  DataPLC.c failed \n");
    fprintf(pFile, "#include\"DataPLC.h\"\n\n");

    fprintf(pFile, "volatile static uint8_t I[%d][8]={};\nvolatile uint16_t AI[%d]={};\nvolatile static uint8_t Q[%d][8]={};\nvolatile static uint8_t M[%d][8]={};\nvolatile static uint16_t u16_VM[8]={};\nvolatile static uint32_t u32_VM[8]={};\nvolatile static float f32_VM[8]={};\n", SUM_I, SUM_AI, SUM_Q, SUM_M);
    while (pMain)
    {
        if (strcmp(pMain->data, "TON") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t %sreset = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t vao%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static  const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static  uint8_t check%s = 0 ;\n", temp);
            continue;
        }
        else if (strcmp(pMain->data, "TOF") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t %sreset = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t vao%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static  const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static  uint8_t check%sOn = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t check%sOff = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t start%s = 0 ;\n", temp);
            continue;
        }
        else if (strcmp(pMain->data, "TONR") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t %sreset = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t vao%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static  const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static  uint8_t check%sOn = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t check%sOff= 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t checkCount%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static  uint8_t start%s = 0 ;\n", temp);
            continue;
        }
        else if (strcmp(pMain->data, "CTU") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t vao%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t reset%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static uint8_t start%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t check%s = 0 ;\n", temp);
            continue;
        }
        else if (strcmp(pMain->data, "CTD") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t vao%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t reset%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static uint32_t nho%s = dat%s ;\n", temp, temp);
            fprintf(pFile, "volatile static uint8_t start%s = 1 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t check%s = 0 ;\n", temp);
            continue;
        }
        else if (strcmp(pMain->data, "CTUD") == 0)
        {
            pMain = pMain->next;
            char *temp = "";
            temp = MS_StrAllocAndAppend(temp, pMain->data);
            pMain = pMain->next;
            fprintf(pFile, "volatile static uint8_t %s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint32_t count%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t tang%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t giam%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t reset%s = 0 ;\n", temp);
            fprintf(pFile, "volatile static const uint32_t dat%s = %s;\n", temp, pMain->data);
            fprintf(pFile, "volatile static uint8_t start%stang = 1 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t start%sgiam = 1 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t check%stang = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t check%sgiam = 0 ;\n", temp);
            fprintf(pFile, "volatile static uint8_t nho%s = 0 ;\n", temp);
            continue;
        }
        pMain = pMain->next;
    }
    pMain = FirstFinal;
    fprintf(pFile, "void Main_task( void *param)\n");
    fprintf(pFile, "{\n");

    fclose(pFile);
}
void STL_AddTimerFuntion(void)
{
    FILE *pFile;
    if (CountTimer > 0)
    {
        int check = CountTimer;
        pFile = fopen("build/DataPLC.c", "a");
        fprintf(pFile, "void TimerCallBack(TimerHandle_t xTimer)\n{\n int id ;\nid = (uint32_t)pvTimerGetTimerID(xTimer) ; \n");
        fprintf(pFile, "switch(id)\n{\n");

        for (int i = 0; i < H_PRIMER_NUMBER; i++)
        {
            if (strncmp(g_Save_IO[i], "TO", 2) == 0)
            {
                check--;
                char *token1, *token2;
                token1 = strtok(g_Save_IO[i], "_");
                char *temp = "";
                temp = MS_StrAllocAndAppend(temp, token1);
                token1 = strtok(NULL, "_");
                if (strncmp(temp, "TONR", 4) == 0)
                {
                    token2 = strtok(temp, "R");
                    token2 = strtok(NULL, "R");
                    fprintf(pFile, "case %s:\n", token1);
                    fprintf(pFile, "count%s++ ;\nbreak;\n", token2);
                }
                else if (strncmp(temp, "TON", 3) == 0)
                {
                    token2 = strtok(temp, "N");
                    token2 = strtok(NULL, "N");
                    fprintf(pFile, "case %s:\n", token1);
                    fprintf(pFile, "count%s++ ;\nbreak;\n", token2);
                }
                else if (strncmp(temp, "TOF", 3) == 0)
                {
                    token2 = strtok(temp, "F");
                    token2 = strtok(NULL, "F");
                    fprintf(pFile, "case %s:\n", token1);
                    fprintf(pFile, "count%s++ ;\nbreak;\n", token2);
                }
            }
            if (!check)
            {
                break;
            }
        }
        fprintf(pFile, "}\n}\n");

        if (CountTimer)
        {
            FILE *pfileInsert;
            char TempArray[128];
            // Mở file bằn hàm fopen
            pfileInsert = fopen("Timer.txt", "r");
            if (pfileInsert == NULL)
            {
                printf("Can not open file Timer");
            }
            else
            {
                //Đọc từng dòng từ file cho tới khi gặp NULL
                while (fgets(TempArray, 128, pfileInsert))
                {
                    fputs(TempArray, pFile);
                }
                fclose(pfileInsert);
                system("del Timer.txt");
            }
        }
        fclose(pFile);
    }
    system("del PLC.txt");
}

void STL_AddReadWriteFunction(void)
{
    FILE *pFile;
    pFile = fopen("build/DataPLC.c", "a");

    fprintf(pFile, "\nvoid read_Pin_Input(void)\n");

    readInputPin(pFile, SUM_I);

    fprintf(pFile, "void write_Pin_Output(void)\n");

    writeOutputPin(pFile, SUM_Q);

    fclose(pFile);

}

