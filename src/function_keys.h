#ifndef FUNCTION_KEYS_H
#define FUNCTION_KEYS_H

#define Uses_SCIM_EVENT
#include <scim.h>

#include <vector>

using namespace scim;

class FunctionKeys
{
    std::vector<KeyEvent> m_mode_switch_keys;
    std::vector<KeyEvent> m_page_up_keys;
    std::vector<KeyEvent> m_page_down_keys;
    std::vector<KeyEvent> m_full_width_punct_keys;
    KeyEvent m_prev_key;
    
public:
    FunctionKeys();
    
    void set_mode_switch_key(const KeyEvent&, bool enabled);
    void set_page_up_key(const KeyEvent&, bool enabled);
    void set_page_down_key(const KeyEvent&, bool enabled);
    
    bool is_mode_switch_key(const KeyEvent&) const;
    bool is_page_up_key(const KeyEvent&) const;
    bool is_page_down_key(const KeyEvent&) const;
    bool is_full_width_punct_key(const KeyEvent&) const;
    void remember_last_key(const KeyEvent&);
    
private:
    bool match_key_event(const std::vector<KeyEvent>& , const KeyEvent& ) const;
    /**
     * add the key into keys if enabled is true, otherwise try to remove key from keys
     */ 
    void update_keys(std::vector<KeyEvent>& keys, const KeyEvent& key, bool enabled);
};

#endif // FUNCTION_KEYS_H
