/*
 * Copyright (C) 2009 Kov Chai <tchaikov@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

PinyinLookupTable::PinyinLookupTable(const DecodingInfo *dec_info,
                                     int page_size)
    : LookupTable(page_size),
      m_dec_info(dec_info)
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
    return m_dec_info->get_candidate(index);
}

AttributeList
PinyinLookupTable::get_attributes (int index) const
{
    SCIM_DEBUG_IMENGINE (2) << "get_attributes(" << index << ")\n";
    // TODO: see CandidateView.onDraw()
    return AttributeList();
}

uint32
PinyinLookupTable::number_of_candidates () const
{
    // SCIM_DEBUG_IMENGINE (2) << "number_of_candidates() => "
    //                         << m_dec_info->get_candidates_number()
    //                         << "\n";
    return m_dec_info->get_candidates_number();
}

void
PinyinLookupTable::clear()
{
    SCIM_DEBUG_IMENGINE  (3) <<  __PRETTY_FUNCTION__ << "\n";
    LookupTable::clear();
    // TODO
}

