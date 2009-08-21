
#include <cassert>
#include <vector>
#include <scim.h>

#include "decoding_info.h"
#include "pinyin_lookup_table.h"

using namespace scim;

WideString
w2wide(const wstring& w)
{
    WideString wide_str;
    copy(w.begin(), w.end(), back_inserter(wide_str));
    return wide_str;
}

PinyinLookupTable::PinyinLookupTable(DecodingInfo *decoding_info,
                                     int page_size)
    : LookupTable(page_size),
      m_decoding_info(decoding_info)
{
    std::vector <WideString> labels;
    char buf [2] = { 0, 0 };
    for (int i = 0; i < 9; ++i) {
        buf [0] = '1' + i;
        labels.push_back (utf8_mbstowcs (buf));
    }

    labels.push_back (utf8_mbstowcs ("0"));
    fix_page_size(false);
    set_candidate_labels (labels);
}

PinyinLookupTable::~PinyinLookupTable()
{
}

/**
 * @param index the candidate index in the lookup table
 */
WideString
PinyinLookupTable::get_candidate (int index) const
{
    // the start point should be synced
    SCIM_DEBUG_IMENGINE  (3) <<  "get_candidate(" << index << ")\n";
    m_decoding_info->get_candidate(index);
}

AttributeList
PinyinLookupTable::get_attributes (int index) const
{
    SCIM_DEBUG_IMENGINE (2) << __PRETTY_FUNCTION__ << index << "!\n";
    // TODO: see CandidateView.onDraw()
    return AttributeList();
}

uint32
PinyinLookupTable::number_of_candidates () const
{
    return m_decoding_info->get_candidates_number();
}

void
PinyinLookupTable::clear()
{
    SCIM_DEBUG_IMENGINE  (3) <<  __PRETTY_FUNCTION__ << "\n";
    LookupTable::clear();
    // TODO
}
