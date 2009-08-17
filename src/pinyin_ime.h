#ifndef PINYIN_IME_H
#define PINYIN_IME_H

#include "decoding_info.h"

class PinyinDecoderService;
class CandidateView;
class ComposingView;

namespace scim {
    struct KeyEvent;
}
using scim::KeyEvent;

/**
 * Main class of the Pinyin input method.
 */
class PinyinIME
{
    PinyinDecoderService* m_decoder_service;
    DecodingInfo m_dec_info;
    ImeState::State m_ime_state;
    CandidateView *m_cand_view;
    ComposingView *m_cmps_view;
    
    /**
     * the absolute offset of candidate
     */
    size_t m_candidate_index;
    
public:
    bool process_key(const KeyEvent& key);
    
private:
    bool process_in_chinese(const KeyEvent& key);
    bool process_state_idle(const KeyEvent& key);
    bool process_state_input(const KeyEvent& key);
    bool process_state_predict(const KeyEvent& key);
    bool process_state_edit_composing(const KeyEvent& key);
    void choose_candidate(int cand_no);
    void choose_and_update(int index);
    bool process_surface_change(const KeyEvent& key);

    void reset_to_idle_state(bool);
    void change_to_state_composing(bool update_ui);
    void change_to_state_input(bool update_ui);
    void commit_result_text(const wstring& result_text);
    void update_composing_text(bool visible);
    void input_comma_period(const wstring& pre_edit, char ch, bool dismiss_cand_window);
    void reset_to_idle_state();
    void show_candidate_window(bool show_composing_view);
    void dismiss_candidate_window();
    void reset_candidate_window();
    
};

#endif // PINYIN_IME_H
