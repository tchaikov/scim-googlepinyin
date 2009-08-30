#include "function_keys.h"

using std::vector;


FunctionKeys::FunctionKeys()
{
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));

    m_page_up_keys.push_back (KeyEvent (SCIM_KEY_comma, 0));
    m_page_up_keys.push_back (KeyEvent (SCIM_KEY_minus, 0));
    m_page_up_keys.push_back (KeyEvent (SCIM_KEY_bracketleft, 0));
    m_page_up_keys.push_back (KeyEvent (SCIM_KEY_Page_Up, 0));

    m_page_down_keys.push_back (KeyEvent (SCIM_KEY_period, 0));
    m_page_down_keys.push_back (KeyEvent (SCIM_KEY_equal, 0));
    m_page_down_keys.push_back (KeyEvent (SCIM_KEY_bracketright, 0));
    m_page_down_keys.push_back (KeyEvent (SCIM_KEY_Page_Down, 0));

    m_full_width_punct_keys.push_back(KeyEvent (SCIM_KEY_comma, SCIM_KEY_ControlMask));
}

void
FunctionKeys::set_mode_switch_key(const KeyEvent& key, bool enabled)
{
    update_keys(m_mode_switch_keys, key, enabled);
}

void
FunctionKeys::set_page_up_key(const KeyEvent& key, bool enabled)
{
    update_keys(m_page_up_keys, key, enabled);
}

void
FunctionKeys::set_page_down_key(const KeyEvent& key, bool enabled)
{
    update_keys(m_page_down_keys, key, enabled);
}

void
FunctionKeys::update_keys(std::vector<KeyEvent>& keys, const KeyEvent& key, bool enabled)
{
    if (enabled) {
        vector<KeyEvent>::iterator where = find(keys.begin(), keys.end(), key);
        if (where == keys.end())
            keys.push_back(key);
    } else {
        keys.erase(remove(keys.begin(), keys.end(), key), keys.end());
    }
}

bool
FunctionKeys::is_mode_switch_key(const KeyEvent& key) const
{
    return match_key_event(m_mode_switch_keys, key);
}

bool
FunctionKeys::is_page_up_key(const KeyEvent& key) const
{
    return match_key_event(m_page_up_keys, key);
}

bool
FunctionKeys::is_page_down_key(const KeyEvent& key) const
{
    return match_key_event(m_page_down_keys, key);
}

bool
FunctionKeys::is_full_width_punct_key(const KeyEvent& key) const
{
    return match_key_event(m_full_width_punct_keys, key);
}

void
FunctionKeys::remember_last_key(const KeyEvent& key)
{
    m_prev_key = key;
}

bool
FunctionKeys::match_key_event (const std::vector <KeyEvent>& keyvec,
                               const KeyEvent& key) const
{
    std::vector<KeyEvent>::const_iterator kit; 
    SCIM_DEBUG_IMENGINE (3) << "match_key_event()\n";
    
    for (kit = keyvec.begin (); kit != keyvec.end (); ++kit) {
        //SCIM_DEBUG_IMENGINE (3) << kit->code << ", " << kit->mask << "\n";
        if (key.code == kit->code && key.mask == kit->mask)
            if (!(key.mask & SCIM_KEY_ReleaseMask) || m_prev_key.code == key.code)
                return true;
    }
    return false;
}
