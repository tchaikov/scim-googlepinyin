/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
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

#ifndef GOOGLEPINYIN_IMENGINE_H
#define GOOGLEPINYIN_IMENGINE_H

#include <string>
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE
#include <scim.h>

using std::wstring;
using namespace scim;

class PinyinLookupTable;
class PinyinDecoderService;
class DecodingInfo;
class PinyinIME;
class FunctionKeys;

class GooglePyFactory : public IMEngineFactoryBase
{
    ConfigPointer       m_config;
    WideString          m_name;
    Connection          m_reload_signal_connection;
    FunctionKeys       *m_func_keys;
    String              m_sys_dict_path;
    String              m_usr_dict_path;
    
    friend class GooglePyInstance;
    
public:
    GooglePyFactory (const ConfigPointer &config);

    virtual ~GooglePyFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
    
public:
    void reload_config(const ConfigPointer& config);
    PinyinDecoderService *create_decoder_service();
    
private:
    bool init ();
    void load_user_config ();
    bool create_directory_if_necessary (const String& directory);
};

class GooglePyInstance : public IMEngineInstanceBase
{
    enum ImMode {
        IM_EN, IM_CN_SIMPLIFIED, IM_CN_TRADITIONAL
    };
    
    GooglePyFactory     *m_factory;
    PinyinLookupTable   *m_lookup_table;
    Connection           m_reload_signal_connection;
    bool                 m_focused;
    bool                 m_forward;
    
    PinyinDecoderService *m_decoder_service;
    PinyinIME            *m_pinyin_ime;
    
public:
    GooglePyInstance(GooglePyFactory *factory,
                     FunctionKeys *func_keys,
                     const String& encoding, int id);
    virtual ~GooglePyInstance();
    /**
     * - mode switch key
     * - toggle full width punctuation mode
     * - toggle full width letter mode
     * - chinese/english switch
     * - caret left/right/home/end
     * - candidate table cursor_up/cursor_down/page_up/page_down/number_select
     * - backspace/delete
     * - space/enter
     */
    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    /**
     * update the configuration of the input method
     */
    virtual void trigger_property (const String &property);

public:
    using IMEngineInstanceBase::commit_string;
    using IMEngineInstanceBase::show_lookup_table;
    using IMEngineInstanceBase::hide_lookup_table;
    using IMEngineInstanceBase::show_preedit_string;
    using IMEngineInstanceBase::hide_preedit_string;
    void refresh_preedit_string(const wstring&, const AttributeList&);
    void refresh_preedit_caret(int);
    void refresh_lookup_table();
    void refresh_status_property(bool cn);
    void refresh_letter_property(bool full);
    void refresh_punct_property(bool full);
    void lookup_page_up();
    void lookup_page_down();
    void lookup_cursor_left();
    void lookup_cursor_right();
    
private:
    void init_lookup_table_labels ();
    void reload_config (const ConfigPointer &config);
    void refresh_all_properties ();
    void initialize_all_properties();
    
    bool try_cancel(const KeyEvent& key);
    bool try_switch_cn(const KeyEvent& key);
    bool try_process_key(const KeyEvent& key);
};

// emacs: -*- c++-mode -*-
#endif //GOOGLEPINYIN_IMENGINE_H
