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
    /**
     * show or hide the candidate dispaly area
     */
    void set_visibility(bool visibility);
    void show_candidates(DecodingInfo *dec_info, bool enable_active_highlight);

    AttributeList get_attributes(int index) const;
    void append_candidate(const wstring&);
    
private:
    void show_page(int page_no, int cand_in_page, bool enable_active_highlight);
    
private:
    GooglePyInstance *m_pinyin;
    PinyinLookupTable *m_lookup_table;
    DecodingInfo *m_dec_info;
    int m_page_no;
    int m_cand_in_page;
    /**
     * Used to decided whether the active candidate should be highlighted or
     * not. If user changes focus to composing view (The view to show Pinyin
     * string), the highlight in candidate view should be removed.
     */
    bool m_active_highlight;
};

#endif // CANDIDATE_VIEW_H
