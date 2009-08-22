#ifndef FUNCTION_KEYS_H
#define FUNCTION_KEYS_H

#define Uses_SCIM_EVENT
#include <scim.h>

#include <vector>

using namespace scim;

class FunctionKeys
{
    std::vector<KeyEvent> m_mode_switch_keys;
    KeyEvent m_prev_key;
    
public:
    FunctionKeys();
    bool is_mode_switch_key(const KeyEvent&) const;
    bool is_full_width_punct_key(const KeyEvent&) const;
private:
    bool match_key_event(const std::vector<KeyEvent>& , const KeyEvent& ) const;
};

#endif // FUNCTION_KEYS_H
