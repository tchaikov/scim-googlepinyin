/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef CANDIDATE_VIEW_H
#define CANDIDATE_VIEW_H

#define Uses_SCIM_ATTRIBUTE
#include <scim.h>
#include <string>

class GooglePyInstance;
class PinyinLookupTable;
class DecodingInfo;
using scim::AttributeList;
using std::wstring;

class CandidateView
{
public:
    CandidateView(GooglePyInstance *pinyin, DecodingInfo *dec_info);
    /**
     * active the previous candidate
     */
    bool cursor_left();
    /**
     * active the next candidate
     */
    bool cursor_right();
    bool page_up();
    bool page_down();
    void enable_active_highlight(bool enable);
    int get_current_page() const;
    int get_active_candidate_pos() const;
    int get_page_size() const;
    void set_page_size(int);
    
    /**
     * show or hide the candidate dispaly area
     */
    void set_visibility(bool visibility);
    void show_candidates(DecodingInfo *dec_info, bool enable_active_highlight);

    AttributeList get_attributes(int index) const;
    void append_candidate(const wstring&);

    void redraw();
    
private:
    bool cursor_back();
    bool cursor_forward();
    void show_page(int page_no, int cand_in_page, bool enable_active_highlight);
    
private:
    GooglePyInstance *m_pinyin;
    PinyinLookupTable *m_lookup_table;
    DecodingInfo *m_dec_info;
    int m_page_no;
    size_t m_cand_in_page;
    int m_page_size;
    
    /**
     * Used to decided whether the active candidate should be highlighted or
     * not. If user changes focus to composing view (The view to show Pinyin
     * string), the highlight in candidate view should be removed.
     */
    bool m_active_highlight;
};

#endif // CANDIDATE_VIEW_H
