#ifndef GOOGLEPINYIN_LOOKUP_TABLE_H
#define GOOGLEPINYIN_LOOKUP_TABLE_H

#define Uses_SCIM_LOOKUP_TABLE
#include <scim.h>

using namespace scim;

class DecodingInfo;

class PinyinLookupTable : public LookupTable
{
    const DecodingInfo* m_dec_info;
    
    PinyinLookupTable (const PinyinLookupTable &);
    const PinyinLookupTable & operator= (const PinyinLookupTable &);

public:
    PinyinLookupTable(const DecodingInfo *decoding_info,
                      int page_size = 10);
    ~PinyinLookupTable();
    virtual WideString get_candidate (int index) const;
    virtual AttributeList get_attributes (int index) const;
    virtual uint32 number_of_candidates () const;
    virtual void clear();
};

#endif//GOOGLEPINYIN_LOOKUP_TABLE_H
