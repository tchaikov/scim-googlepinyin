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
    #define SCIM_GOOGLEPINYIN_ICON_FILE       (SCIM_ICONDIR "/googlepinyin_logo.xpm")
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
    : m_user_data(0),
      m_config (config),
      m_valid (false)
{
    SCIM_DEBUG_IMENGINE (3) << "GooglePyFactory()\n";
    set_languages ("zh_CN");
    m_name = utf8_mbstowcs ("GooglePinyin");
    m_decoder_service = new PinyinDecoderService()
    m_valid = load_system_data() && init ();
    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &GooglePyFactory::reload_config));
}

bool
GooglePyFactory::init ()
{
    String sys_dict_path =  String(SCIM_GOOGLEPINYIN_DATADIR) +
                            String(SCIM_PATH_DELIM_STRING) +
                            String("py_dict.dat");
    String user_data_directory = String(scim_get_home_dir () +
                                        String (SCIM_PATH_DELIM_STRING) +
                                        String (".scim") + 
                                        String (SCIM_PATH_DELIM_STRING) +
                                        String ("google"));
    String usr_dict_path = String(user_data_directory +
                                  String(SCIM_PATH_DELIM_STRING) +
                                  String("usr_dict.dat"));
    m_decoder_service = PinyinDecoderService(sys_dict_path,
                                             usr_dict_path);
    return m_decoder_service.is_initialized();
}

GooglePyFactory::~GooglePyFactory ()
{
    SCIM_DEBUG_IMENGINE (3) << "~GooglePyFactory()\n";
    m_reload_signal_connection.disconnect ();
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
                String (_("Lei Zhang, <Phill.Zhang@sun.com>; Shuguagn Yan, <Ervin.Yan@sun.com>")));
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
    return new GooglePyInstance (this, m_decoder_service, encoding, id);
}

void
GooglePyFactory::reload_config (const ConfigPointer &config)
{
    m_config = config;
}

// implementation of GooglePyInstance
GooglePyInstance::GooglePyInstance (GooglePyFactory *factory,
                                    PinyinDecoderService *decoder_service,
                                    const String& encoding,
                                    int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_decoder_service(decoder_service),
      m_lookup_table (0),
      m_focused (false)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": GooglePyInstance()\n";
    create_session(m_pref, m_pinyin_data, m_user_data->get_history());
    m_reload_signal_connection = factory->m_config->signal_connect_reload (slot (this, &GooglePyInstance::reload_config));
    init_lookup_table_labels ();
}

GooglePyInstance::~GooglePyInstance ()
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": ~GooglePyInstance()\n";
    m_reload_signal_connection.disconnect ();
    destroy_session();
}

bool
GooglePyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": process_key_event(" << m_focused << ", "  <<
        key.code << ", " <<
        key.mask << ", " <<
        key.layout << ")\n";
        
    if (!m_focused) return false;

    if (key.is_key_release ())
        return true;
    
    return ( try_switch_cn(key) ||
             try_select_candidate(key) ||
             try_process_key(key) );
}

bool
GooglePyInstance::try_process_key(const GoogleKeyEvent& key)
{
    if (m_pref->isPageUpKey(key.code, key.value, key.modifier)) {
        lookup_page_up ();
    }
    else if (m_pref->isPageDnKey(key.code, key.value, key.modifier)) {
        lookup_page_down ();
    }
    
    return m_pv->onKeyEvent(key.code, key.value, key.modifier);
}

bool
GooglePyInstance::process_state_predict(const KeyEvent& key)
{
    
}

bool
GooglePyInstance::process_state_input(const KeyEvent& key)
{




void input_comma_period(WideString pre_edit, char ch)
{}

bool
GooglePyInstance::try_choose_candidate(const KeyEvent& key)
{
    if (key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9 && key.mask == 0) {
        int index;
        if (key.code == SCIM_KEY_0)
            index = 9;
        else
            index = key.code - SCIM_KEY_1;
        if (index < m_lookup_table->get_current_page_size()) {
            index += m_lookup_table->get_current_page_start();
            choose_and_update(index);
        }
        return true;
    }
    return false;
}

bool
GooglePyInstance::choose_and_update(int index)
{
    if (m_ime_state != STATE_PREDICT) {
        // Get result candidate list, if choice_id < 0, do a new decoding.
        // If choice_id >=0, select the candidate, and get the new candidate
        // list.
        m_dec_info.choose_decoding_candidate(index);
    } else {
        // Choose a prediction item.
        m_dec_info.choose_predict_choice(index);
    }

    if (!m_dec_info.get_composing_str().empty()) {
        WideString result_str = m_dec_info.get_composing_str_active_part();
        if (m_ime_state == STATE_IDLE) {
            if (m_dec_info.get_spl_str_decoded_len() == 0) {
                m_ime_state = STATE_COMPOSING;
            } else {
                m_ime_state = STATE_INPUT;
            }
        } else {
            if (m_dec_info.selection_finished()) {
                m_ime_state = STATE_COMPOSING;
            }
        }
        show_lookup_table();
    } else {
        reset();
    }
}

void
GooglePyInstance::select_candidate (unsigned int item)
{
    m_pv->onCandidateSelectRequest(item);
//  m_pv->makeSelection(item);
}

void
GooglePyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0) {
        SCIM_DEBUG_IMENGINE (3) << ": update_lookup_table_page_size(" << page_size << ")\n";
        m_pv->s_CandiWindowSize = page_size;
        m_lookup_table->set_page_size(page_size);
    }
}

void
GooglePyInstance::lookup_table_page_up ()
{
    // XXX, it would be great, if View class expose a page_up() method
//    m_pv->onKeyEvent(IM_VK_PAGE_UP, 0, 0);
    // classic View overrides this method
    // but modern View uses the default dummy implementation
    lookup_page_up();
    m_pv->onCandidatePageRequest(-1, true);
}

void
GooglePyInstance::lookup_page_up()
{
    m_lookup_table->page_up();
    //    m_lookup_table->set_page_size(m_pv->s_CandiWindowSize);
}

void
GooglePyInstance::lookup_page_down()
{
    m_lookup_table->page_down();
    //    m_lookup_table->set_page_size(m_pv->s_CandiWindowSize);
}

void
GooglePyInstance::lookup_table_page_down ()
{
    // XXX, it would be great, if View class expose a page_up() method
    //    m_pv->onKeyEvent(IM_VK_PAGE_DOWN, 0, 0);
    // classic View overrides this method
    // but modern View uses the default dummy implementation
    lookup_page_down ();
    m_pv->onCandidatePageRequest(1, true);
}

void
GooglePyInstance::move_preedit_caret (unsigned int /*pos*/)
{
}

void
GooglePyInstance::reset ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reset()\n";

    m_ime_state = STATE_IDLE;

    m_lookup_table->clear ();
    m_dec_info.reset();
    
    hide_lookup_table ();
    hide_preedit_string ();
    hide_aux_string ();
    //refresh_all_properties ();
}

void
GooglePyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(3) << get_id() << ": focus_in ()\n";
    m_focused = true;
    
    initialize_all_properties ();
    
    hide_preedit_string ();
    //hide_aux_string ();
    
    init_lookup_table_labels ();
    
    CGooglepinyinOptions* pref =
        dynamic_cast<CGooglepinyinOptions*>( m_pv->getPreference() );

    if ( (m_pref->m_GBK != pref->m_GBK && get_encoding() != "GB2312") ||
         m_pref->m_ViewType != pref->m_ViewType ) {
        destroy_session();
        // imi_cle hides preedit and candidates
        create_session(m_pref, m_pinyin_data, m_user_data->get_history());
    } else {
        pref->m_MinusAsPageUp   = m_pref->m_MinusAsPageUp;
        pref->m_CommaAsPageUp   = m_pref->m_CommaAsPageUp;
        pref->m_BracketAsPageUp = m_pref->m_BracketAsPageUp;
    }
    
    //hide_aux_string ();

    m_pv->updateWindows(CIMIView::PREEDIT_MASK | CIMIView::CANDIDATE_MASK);
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
        const int is_CN = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_CN);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, is_CN?0:1);
    } else if (property == SCIM_PROP_LETTER) {
        const int is_fullsimbol = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSIMBOL);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSIMBOL, is_fullsimbol?0:1);
    } else if (property == SCIM_PROP_PUNCT) {
        const int is_fullpunc = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, is_fullpunc?0:1);
    }
}


void
GooglePyInstance::init_lookup_table_labels ()
{
    m_pv->s_CandiWindowSize = 10;

    m_lookup_table->set_page_size (m_pv->s_CandiWindowSize);
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
GooglePyInstance::refresh_all_properties ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_all_properties()\n";
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_CN,
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_CN));
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_FULLPUNC,
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC));
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_FULLSIMBOL, 
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSIMBOL));
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
    if (key.code == SCIM_KEY_KP_Space && key.is_shift_down()) {
        if (key.is_key_release()) return true;
        trigger_property (SCIM_PROP_STATUS);
        return true;
    }
    return false;
}

void
GooglePyInstance::create_session(CGooglepinyinOptions* pref,
                              CIMIData* pinyin_data,
                              CBigramHistory* history)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": create_session()\n";
    
    CIMIContext* ic = new CIMIContext();
    ic->setCoreData(pinyin_data);
    ic->setHistoryMemory(history);
    ic->setNonCompleteSyllable(true);
    ic->clear();

    pref->m_GBK = (get_encoding() != "GB2312");
    int viewType = pref->m_ViewType?
        CIMIViewFactory::SVT_MODERN:
        CIMIViewFactory::SVT_CLASSIC;
    CIMIView* pv = CIMIViewFactory::createView(viewType);
    pv->setPreference(pref);
    pv->attachWinHandler(m_wh);
    pv->attachIC(ic);

    GoogleLookupTable* lookup_table = new GoogleLookupTable();
    
    CScimWinHandler* wh = new CScimWinHandler(this, lookup_table);
    wh->setOptions(pv->getPreference());
    pv->attachWinHandler(wh);
    
    m_wh = wh;
    m_pv = pv;
    m_lookup_table = lookup_table;
}

void
GooglePyInstance::destroy_session()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": destroy_session()\n";
    
    // wh and ic are not pointers, I don't think it's necessary to delete them
    // either
    delete m_pv->getIC();
    delete m_pv->getWinHandler();
    delete m_pv;
    delete m_lookup_table;
    
    m_pv = 0;
    m_wh = 0;
    m_lookup_table = 0;
}

AttributeList
GooglePyInstance::build_preedit_attribs (const IPreeditString* ppd)
{
    AttributeList attrs;
    const int sz = ppd->charTypeSize();
    for (int i = 0; i < sz; ) {
        const int ct = ppd->charTypeAt(i);
        if (ct & IPreeditString::ILLEGAL) {
            const int start = i;
            for (++i; (i<sz) && (ppd->charTypeAt(i) & IPreeditString::ILLEGAL); ++i) ;
            attrs.push_back( Attribute(start, i-start,
                                       SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        } else if (ct & IPreeditString::NORMAL_CHAR) {
            if (ct & IPreeditString::USER_CHOICE) {
                const int start = i;
                for (++i; (i<sz) && (ppd->charTypeAt(i) & IPreeditString::USER_CHOICE); ++i) ;
                attrs.push_back( Attribute(start, i-start,
                                           SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_UNDERLINE));
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    return attrs;
}

void
GooglePyInstance::redraw_preedit_string (const IPreeditString* ppd)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": redraw_preedit_string()\n";
    if (ppd->size() != 0) {
        AttributeList attrs;
        const int caret = ppd->caret();
        if (caret > 0 && caret <= ppd->size()) {
            attrs.push_back( Attribute(ppd->candi_start(),
                                       ppd->charTypeSize(),
                                       SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        }
        update_preedit_string( wstr_to_widestr(ppd->string(), ppd->size()) );
        show_preedit_string ();
        update_preedit_caret (caret);
    } else {
        hide_preedit_string ();
    }
}

void
GooglePyInstance::redraw_lookup_table(const ICandidateList* pcl)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": redraw_lookup_table()\n";
    SCIM_DEBUG_IMENGINE (3) << "page size = " << m_pv->s_CandiWindowSize << "\n";
    
    m_lookup_table->update(*pcl);
    if (m_lookup_table->number_of_candidates()) {
        update_lookup_table(*m_lookup_table);
        show_lookup_table();
    } else {
        hide_lookup_table();
    }
}

void
GooglePyInstance::reload_config(const ConfigPointer &config)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reload_config()\n";
    reset();
    if (m_factory->valid()) {
        m_factory->load_user_data();
    }
}
