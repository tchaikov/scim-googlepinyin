#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_DEBUG

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <scim.h>

#include "google_keycode.h"
#include "pinyin_lookup_table.h"
#include "google_intl.h"
#include "pinyin_decoder_service.h"
#include "decoding_info.h"
#include "pinyin_ime.h"
#include "function_keys.h"
#include "google_imengine.h"


#define SCIM_PROP_STATUS                  "/IMEngine/GooglePinyin/Status"
#define SCIM_PROP_LETTER                  "/IMEngine/GooglePinyin/Letter"
#define SCIM_PROP_PUNCT                   "/IMEngine/GooglePinyin/Punct"

#ifndef SCIM_GOOGLEPINYIN_DATADIR
    #define SCIM_GOOGLEPINYIN_DATADIR            "/usr/share/scim/googlepinyin"
#endif

#ifndef SCIM_ICONDIR
    #define SCIM_ICONDIR                      "/usr/share/scim/icons"
#endif

#ifndef SCIM_GOOGLEPINYIN_ICON_FILE
    #define SCIM_GOOGLEPINYIN_ICON_FILE       (SCIM_ICONDIR "/google-pinyin_icon.png")
#endif

#define SCIM_FULL_LETTER_ICON              (SCIM_ICONDIR "/full-letter.png")
#define SCIM_HALF_LETTER_ICON              (SCIM_ICONDIR "/half-letter.png")
#define SCIM_FULL_PUNCT_ICON               (SCIM_ICONDIR "/full-punct.png")
#define SCIM_HALF_PUNCT_ICON               (SCIM_ICONDIR "/half-punct.png")

using namespace scim;

static IMEngineFactoryPointer _scim_pinyin_factory (0); 

static ConfigPointer _scim_config (0);

static Property _status_property   (SCIM_PROP_STATUS, "");
static Property _letter_property   (SCIM_PROP_LETTER, "");
static Property _punct_property    (SCIM_PROP_PUNCT, "");

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_IMENGINE (3) << "scim_module_init\n";
        bindtextdomain (GETTEXT_PACKAGE, SCIM_GOOGLEPINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_pinyin_factory.reset ();
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE (3) << "module_init\n";
        _status_property.set_tip (_("The status of the current input method. Click to change it."));
        _status_property.set_label ("英");
        
        _letter_property.set_icon (SCIM_HALF_LETTER_ICON);
        _letter_property.set_tip (_("The input mode of the letters. Click to toggle between half and full."));
        _letter_property.set_label (_("Full/Half Letter"));

        _punct_property.set_icon (SCIM_HALF_PUNCT_ICON);
        _punct_property.set_tip (_("The input mode of the puncutations. Click to toggle between half and full."));
        _punct_property.set_label (_("Full/Half Punct"));

        _scim_config = config;
        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        SCIM_DEBUG_IMENGINE (3) << "entering scim_imengine_module_create_factory()\n";
        if (engine != 0) return IMEngineFactoryPointer (0);
        if (_scim_pinyin_factory.null ()) {
            GooglePyFactory *factory = new GooglePyFactory (_scim_config); 
            if (factory->valid ())
                _scim_pinyin_factory = factory;
            else
                delete factory;
        }
        return _scim_pinyin_factory;
    }
}

// implementation of GooglePyFactory
GooglePyFactory::GooglePyFactory (const ConfigPointer &config)
    : m_config (config),
      m_valid (false)
{
    SCIM_DEBUG_IMENGINE (3) << "GooglePyFactory()\n";
    set_languages ("zh_CN");
    m_name = utf8_mbstowcs ("GooglePinyin");
    m_valid = init ();
    m_func_keys = new FunctionKeys();
    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &GooglePyFactory::reload_config));
}

bool
GooglePyFactory::init ()
{
    String sys_dict_path =  String(SCIM_GOOGLEPINYIN_DATADIR) +
                            String(SCIM_PATH_DELIM_STRING) +
                            String("dict_pinyin.dat");
    String user_data_directory = String(scim_get_home_dir () +
                                        String (SCIM_PATH_DELIM_STRING) +
                                        String (".scim") + 
                                        String (SCIM_PATH_DELIM_STRING) +
                                        String ("google-pinyin"));
    String usr_dict_path = String(user_data_directory +
                                  String(SCIM_PATH_DELIM_STRING) +
                                  String("usr_dict.dat"));
    SCIM_DEBUG_IMENGINE (3) << "GooglePyFactory::init()\n";
    SCIM_DEBUG_IMENGINE (3) << "sys_dict_path = " << sys_dict_path << "\n";
    SCIM_DEBUG_IMENGINE (3) << "usr_dict_path = " << usr_dict_path << "\n";
    m_decoder_service = new PinyinDecoderService(sys_dict_path,
                                                 usr_dict_path);
    return m_decoder_service->is_initialized();
}

GooglePyFactory::~GooglePyFactory ()
{
    SCIM_DEBUG_IMENGINE (3) << "~GooglePyFactory()\n";
    m_reload_signal_connection.disconnect ();
    delete m_decoder_service;
    delete m_func_keys;
}

WideString
GooglePyFactory::get_name () const
{
    return m_name;
}

WideString
GooglePyFactory::get_authors () const
{
    return utf8_mbstowcs (
                String (_("Genqing Wu, Xiaotao Duan, Wei Sun")));
}

WideString
GooglePyFactory::get_credits () const
{
    return utf8_mbstowcs (
        String (_("Ported by Kov Chai, <tchaikov@gmail.com>")));
}

WideString
GooglePyFactory::get_help () const
{
    String help =
        String (_("Hot Keys:"
                  "\n\n  Shift+Alt:\n"
                  "    Switch between English/Chinese mode."
                  "\n\n  Control+period:\n"
                  "    Switch between full/half width punctuation mode."
                  "\n\n  Shift+space:\n"
                  "    Switch between full/half width letter mode."
                  "\n\n  PageUp:\n"
                  "    Page up in lookup table."
                  "\n\n  PageDown:\n"
                  "    Page down in lookup table."
                  "\n\n  Esc:\n"
                  "    Cancel current syllable.\n"));
    return utf8_mbstowcs (help);
}

String
GooglePyFactory::get_uuid () const
{
    return String ("f15ec6e9-cdd2-4506-a354-0d4b3d329956");
}

String
GooglePyFactory::get_icon_file () const
{
    return String (SCIM_GOOGLEPINYIN_ICON_FILE);
}

IMEngineInstancePointer
GooglePyFactory::create_instance (const String& encoding, int id)
{
    SCIM_DEBUG_IMENGINE (3) <<  "GooglePyFactory::create_instance(" << id << ")\n";    
    return new GooglePyInstance (this, m_decoder_service, m_func_keys, encoding, id);
}

void
GooglePyFactory::reload_config (const ConfigPointer &config)
{
    m_config = config;
}

// implementation of GooglePyInstance
GooglePyInstance::GooglePyInstance (GooglePyFactory *factory,
                                    PinyinDecoderService *decoder_service,
                                    FunctionKeys *func_keys,
                                    const String& encoding,
                                    int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_focused (false)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": GooglePyInstance()\n";
    m_dec_info = new DecodingInfo(decoder_service);
    m_pinyin_ime = new PinyinIME(m_dec_info, func_keys, this);
    m_lookup_table = new PinyinLookupTable(m_dec_info, 10);
    m_reload_signal_connection = factory->m_config->signal_connect_reload (slot (this, &GooglePyInstance::reload_config));
    init_lookup_table_labels ();
}

GooglePyInstance::~GooglePyInstance ()
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": ~GooglePyInstance()\n";
    m_reload_signal_connection.disconnect ();
    delete m_lookup_table;
    delete m_dec_info;
    delete m_pinyin_ime;
}

bool
GooglePyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": process_key_event(" << m_focused << ", "  <<
        key.code << ", " <<
        key.mask << ", " <<
        key.layout << ")\n";
        
    if (!m_focused) return false;
    
    return ( try_cancel(key) ||
             try_process_key(key) );
}

void
GooglePyInstance::move_preedit_caret (unsigned int pos)
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << __PRETTY_FUNCTION__ << "(" << pos << ")\n";
}

// item is the in-page index
void
GooglePyInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << __PRETTY_FUNCTION__ << "(" << item << ")\n";
    m_pinyin_ime->choose_candidate_in_page(item);
}

void
GooglePyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE (3) << ": update_lookup_table_page_size(" << page_size << ")\n";
    if (page_size > 0) {
        m_pinyin_ime->set_candidate_page_size(page_size);
    }
}

void
GooglePyInstance::lookup_table_page_up ()
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << __PRETTY_FUNCTION__ << "()\n";
    m_pinyin_ime->candidate_page_up();
    update_lookup_table(*m_lookup_table);
}

void
GooglePyInstance::lookup_table_page_down ()
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << __PRETTY_FUNCTION__ << "()\n";
    m_pinyin_ime->candidate_page_down();
    update_lookup_table(*m_lookup_table);
}


void
GooglePyInstance::reset ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reset()\n";
    m_pinyin_ime->reset();
    m_lookup_table->clear();
    
    hide_lookup_table ();
    hide_preedit_string ();
    hide_aux_string ();
    
    refresh_all_properties ();
}

void
GooglePyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(3) << get_id() << ": focus_in ()\n";
    m_focused = true;
    
    initialize_all_properties ();
    
    hide_preedit_string ();
    hide_aux_string ();
    if (m_pinyin_ime->is_chinese_mode()) {
        m_pinyin_ime->redraw();
    }
    
    init_lookup_table_labels ();
}

void
GooglePyInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(3) << get_id() << ": focus_out ()\n";
    m_focused = false;
}

void
GooglePyInstance::trigger_property (const String &property)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": trigger_property(" << property << ")\n";
    
    if (property == SCIM_PROP_STATUS) {
        m_pinyin_ime->trigger_input_mode();
    } else if (property == SCIM_PROP_LETTER) {
        
    } else if (property == SCIM_PROP_PUNCT) {
        
    }
}


void
GooglePyInstance::init_lookup_table_labels ()
{
    m_lookup_table->set_page_size (9);
    m_lookup_table->show_cursor ();
}

void
GooglePyInstance::initialize_all_properties ()
{
    PropertyList proplist;

    proplist.push_back (_status_property);
    proplist.push_back (_letter_property);
    proplist.push_back (_punct_property);

    register_properties (proplist);
    refresh_all_properties ();
}

void
GooglePyInstance::refresh_preedit_string(const wstring& preedit,
                                         const AttributeList& attrs)
{
    if (!preedit.empty()) {
        update_preedit_string(preedit, attrs);
        show_preedit_string();
    } else {
        hide_preedit_string();
    }
}

void
GooglePyInstance::refresh_preedit_caret(int caret)
{
    update_preedit_caret(caret);
}

void
GooglePyInstance::refresh_lookup_table()
{
    update_lookup_table(*m_lookup_table);
}

void
GooglePyInstance::refresh_all_properties ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_all_properties()\n";
}


void
GooglePyInstance::refresh_status_property(bool cn)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_status_property(" << cn << ")\n";
    if (!cn) {
        reset();
    }
    _status_property.set_label(cn ? "中" : "英");
    update_property(_status_property);
}

void
GooglePyInstance::refresh_fullsimbol_property(bool full)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_fullsimbol_property(" << full << ")\n";
    _letter_property.set_icon(
        full ? SCIM_FULL_LETTER_ICON : SCIM_HALF_LETTER_ICON);
    update_property(_letter_property);
}

void
GooglePyInstance::refresh_fullpunc_property(bool full)
{
    _punct_property.set_icon(
        full ? SCIM_FULL_PUNCT_ICON : SCIM_HALF_PUNCT_ICON);
    update_property(_punct_property);
}

bool
GooglePyInstance::try_switch_cn(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": try_switch_cn(" << key.code << ")\n";
    if (key.code == SCIM_KEY_space && key.is_shift_down()) {
        m_forward = !m_forward;
        refresh_all_properties();
        reset();
        return true;
    }
    return false;
}

bool
GooglePyInstance::try_cancel(const KeyEvent& key)
{
    if (key.code == SCIM_KEY_Escape && key.mask == 0) {
        // if get_original_spl_str().empty() return false;
        reset();
        return true;
    }
    return false;
}

bool
GooglePyInstance::try_process_key(const KeyEvent& key)
{
    return m_pinyin_ime->process_key(key);
}

void
GooglePyInstance::reload_config(const ConfigPointer &config)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reload_config()\n";
    reset();
}
