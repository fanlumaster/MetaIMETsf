#pragma once

#include "File.h"
#include "MetasequoiaIMEBaseStructure.h"

class CBaseDictionaryEngine
{
  public:
    CBaseDictionaryEngine(LCID locale, _In_ CFile *pDictionaryFile);
    virtual ~CBaseDictionaryEngine();

    virtual VOID CollectWord(_In_ CStringRange *psrgKeyCode, _Out_ CMetasequoiaImeArray<CStringRange> *pasrgWordString)
    {
        psrgKeyCode;
        pasrgWordString = nullptr;
    }

    virtual VOID CollectWord(_In_ CStringRange *psrgKeyCode, _Out_ CMetasequoiaImeArray<CCandidateListItem> *pItemList)
    {
        psrgKeyCode;
        pItemList = nullptr;
    }

    virtual VOID SortListItemByFindKeyCode(_Inout_ CMetasequoiaImeArray<CCandidateListItem> *pItemList);

  protected:
    CFile *_pDictionaryFile;
    LCID _locale;

  private:
    VOID MergeSortByFindKeyCode(_Inout_ CMetasequoiaImeArray<CCandidateListItem> *pItemList, int leftRange, int rightRange);
    int CalculateCandidateCount(int leftRange, int rightRange);
};
