#include "function_keys.h"

FunctionKeys::FunctionKeys()
{
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));
    m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));
}

bool
FunctionKeys::is_mode_switch_key(const KeyEvent& key)
{
    return match_key_event(m_mode_switch_keys, key);
}

bool
FunctionKeys::match_key_event (const std::vector <KeyEvent>& keyvec, const KeyEvent& key)
{
    std::vector<KeyEvent>::const_iterator kit; 

    for (kit = keyvec.begin (); kit != keyvec.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
            if (!(key.mask & SCIM_KEY_ReleaseMask) || m_prev_key.code == key.code)
                return true;
    }
    return false;
}
