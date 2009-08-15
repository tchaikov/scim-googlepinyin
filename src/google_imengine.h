#ifndef SUNPINYIN_IMENGINE_H
#define SUNPINYIN_IMENGINE_H

using namespace scim;

class GooglePyFactory : public IMEngineFactoryBase
{
    CGooglepinyinUserData *m_user_data;
    CGooglepinyinOptions   m_pref;
    CIMIData            m_pinyin_data;
    ConfigPointer       m_config;
    bool                m_valid;
    WideString          m_name;
    Connection          m_reload_signal_connection;

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
    bool valid () const { return m_valid; }
    void reload_config(const ConfigPointer& config);
    
private:
    bool init ();
    bool load_system_data ();
    bool load_user_config ();
    void load_user_data ();
};

class GooglePyInstance : public IMEngineInstanceBase
{
    GooglePyFactory        *m_factory;
    GoogleLookupTable      *m_lookup_table;
    
    Connection           m_reload_signal_connection;
    bool                 m_focused;
    ImeState             m_ime_state;
    
  public:
    GooglePyInstance(GooglePyFactory *factory, CGooglepinyinUserData *user_data,
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
    /**
     * expose this inherited protected method
     * so that CScimWinHandler can call it
     */
    using IMEngineInstanceBase::commit_string;
    
    void refresh_status_property(bool cn);
    void refresh_fullsimbol_property(bool full);
    void refresh_fullpunc_property(bool full);
    void redraw_preedit_string(const IPreeditString* ppd);
    void redraw_lookup_table(const ICandidateList* pcl);

private:
    void create_session(CGooglepinyinOptions* pref,
                        CIMIData* pinyin_data,
                        CBigramHistory* history);
    void destroy_session();
    
    void init_lookup_table_labels ();
    void reload_config (const ConfigPointer &config);
    void refresh_all_properties ();
    void initialize_all_properties();
    
    AttributeList build_preedit_attribs(const IPreeditString* ppd);

    bool try_switch_style(const GoogleKeyEvent& key);
    bool try_switch_gbk(const GoogleKeyEvent& key);
    bool try_switch_cn(const GoogleKeyEvent& key);
    bool try_process_key(const GoogleKeyEvent& key);

    void lookup_page_up();
    void lookup_page_down();
};

// emacs: -*- c++-mode -*-
#endif//SUNPINYIN_IMENGINE_H
