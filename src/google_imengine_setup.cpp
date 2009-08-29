#define Uses_SCIM_CONFIG_BASE

#include <vector>
#include <algorithm>
#include <gtk/gtk.h>
#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "google_intl.h"
#include "google_imengine_config_keys.h"

using namespace scim;
using namespace std;

#define scim_module_init pinyin_imengine_setup_LTX_scim_module_init
#define scim_module_exit pinyin_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       pinyin_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    pinyin_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        pinyin_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description pinyin_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     pinyin_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     pinyin_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   pinyin_imengine_setup_LTX_scim_setup_module_query_changed


static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_GOOGLEPINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Google Pinyin"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("Google Pinyin based on its Android version."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

// Internal data structure
struct KeyboardConfigData
{
    const char *key;
    String      data;
};

// Internal data declaration.
static bool have_changed                 = false;

// Common callbacks
static void
on_state_switch_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
on_page_flipping_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static GtkWidget *
create_pinyin_page(GtkWidget *notebook, gint page_num);

static void
update_keyboard_config(int key_index, const char* key, gboolean stat);

static bool is_key_in(const String& keys, const char *key);
static String add_key_to(const String& keys, const char *key);
static String remove_key_from(const String& keys, const char *key);

// Function implementations.
static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (window) window;
    
    GtkWidget *notebook;
    
    // Create the Notebook.
    notebook = gtk_notebook_new ();
    gtk_widget_show (notebook);
    
    
    // Create the label for this note page.
    GtkWidget *label;
    label = gtk_label_new (_("Keyboard"));
    gtk_widget_show (label);
    
    GtkWidget *page = create_pinyin_page(notebook, 0);
    
    gtk_notebook_append_page( GTK_NOTEBOOK (notebook), page, label);
    window = notebook;
    return window;
}

enum {
    CONFIG_SWITCH,
    CONFIG_PAGE_UP,
    CONFIG_PAGE_DOWN
};

static KeyboardConfigData config_keyboards[] =
{
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_MODE_SWITCH_KEY,
        "Shift+Shift_L+KeyRelease"
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_PAGE_UP_KEY,
        "Page_Up"
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_PAGE_DOWN_KEY,
        "Page_Down"
    },
    {
        NULL,
        ""
    }
};

struct StateSwitchingKeys
{
    const char *label;
    const char *key;
    GtkWidget  *button;
};

static StateSwitchingKeys state_switching_keys[] =
{
    {_("Shift"), "Shift". NULL},
    {_("Ctrl"),  "Shift", NULL},
    {_("None"),  NULL, NULL}
};

static GtkWidget *
create_state_switch_frame()
{
    // container frame
    GtkWidget *frame;
    frame = gtk_frame_new(_("State Switch"));
    gtk_widget_show(frame);
    
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    
    // radio group
    GtkWidget *hbox;
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    GtkWidget *label;
    label = gtk_label_new("Switching between Chinese and English");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkWidget *radio_buttons[3] = { NULL };
    GSList *radio_group = NULL;
    for (int i = 0; i < 3; ++i) {
        radio_buttons[i] =
            gtk_radio_button_new_with_label(radio_group,
                                            state_switching_keys[i+1].label);
        gtk_widget_show(radio_buttons[i]);
        gtk_box_pack_start(GTK_BOX(hbox), radio_buttons[i], FALSE, FALSE, 0);
        radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_buttons[i]));
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(radio_buttons[i]), radio_group);
        gtk_container_set_border_width (GTK_CONTAINER (radio_buttons[i]), 2);
        g_signal_connect(G_OBJECT(radio_buttons[i]), "toggled",
                         G_CALLBACK(on_state_switch_toggled),
                         &state_switching_keys[i]);
    }
    return frame;
}

static void
on_state_switch_toggled(GtkToggleButton *button, gpointer user_data)
{
    StateSwitchingKeys *key_info = (StateSwitchingKeys *)user_data;
    gboolean stat = gtk_toggle_button_get_active(button);
    if (key_info->key == NULL) {
        if (stat)
            config_keyboards[CONFIG_SWITCH].data.clear();
        return;
    }
    update_keyboard_config(CONFIG_SWITCH, key_info->key, stat);
}

static void
update_keyboard_config(int key_index, const char* key, gboolean stat)
{
    KeyboardConfigData& config = config_keyboards[key_index];
    if (stat) {
        config.data = add_key_to(config.data, key);
    } else {
        config.data = remove_key_from(config.data, key);
    }
}

String add_key_to(const String& keys, const char *key)
{
    vector<String> v;
    int n_keys = scim_split_string_list(v, keys);
    vector<String>::iterator where = find(v.begin(), v.end(), key);
    if (where == v.end()) {
        v.push_back(key);
    }
    return scim_combine_string_list(v);
}

String remove_key_from(const String& keys, const char *key)
{
    vector<String> v;
    int n_keys = scim_split_string_list(v, keys);
    vector<String>::iterator where = find(v.begin(), v.end(), key);
    if (where != v.end()) {
        v.erase(where);
    }
    return scim_combine_string_list(v);
}

struct PageFlippingKeys
{
    const char *label;
    const char *page_down;
    const char *page_up;
};

static PageFlippingKeys page_flipping_keys[] =
{
    {_("Minus/Equal (-/=)"),  "Minus", "Equal"},
    {_("Comma/Period (,/.)"), "Comma", "Period"}
};

static GtkWidget *
create_candidate_view_frame()
{
    // container frame
    GtkWidget *frame;
    frame = gtk_frame_new(_("Candidate View"));
    gtk_widget_show(frame);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

    // check box group
    GtkWidget *hbox;
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    GtkWidget *label;
    label = gtk_label_new("Page Flipping");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkWidget *check_buttons[2] = { NULL };
    for (int i = 0; i < 2; ++i) {
        check_buttons[i] = gtk_check_button_new_with_label(page_flipping_keys[i].label);
        gtk_widget_show(check_buttons[i]);
        gtk_box_pack_start(GTK_BOX(hbox), check_buttons[i], FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (check_buttons[i]), 2);
        g_signal_connect(G_OBJECT(check_buttons[i]), "toggled",
                         G_CALLBACK(on_page_flipping_toggled),
                         &check_buttons[i]);
    }
    // TODO (chaik)
    // - hotkey for choosing 2nd and 3rd candidate
    // - choose character by inputting words
    // - switch between simplified/traditional Chinese
    return frame;
}

static void
on_page_flipping_toggled(GtkToggleButton *button, gpointer user_data)
{
    PageFlippingKeys *key_info = (PageFlippingKeys *)user_data;
    
    gboolean stat = gtk_toggle_button_get_active(button);
    update_keyboard_config(CONFIG_PAGE_DOWN, key_info->page_down, stat);
    update_keyboard_config(CONFIG_PAGE_UP, key_info->page_up, stat);
}

// Create the keyboard configurations page
static GtkWidget *
create_pinyin_page(GtkWidget *notebook, gint page_num)
{
    GtkWidget *vbox;
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    
    GtkWidget *frame;
    frame = create_state_switch_frame();
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
    frame = create_candidate_view_frame();
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 2);
    return vbox;
}

static void
on_value_changed(GtkWidget *widget, gpointer user_data)
{
    have_changed = true;
}

static bool
query_changed()
{
    return have_changed;
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null()) return;
    for (int i = 0; i < config_keyboards[i].key; ++i) {
        config_keyboards[i].data =
            config->read (String(config_keyboards[i].key),
                          config_keyboards[i].data);
    }
    for (int i = 0; i < 3; ++i) {
        gboolean stat;
        if (i != 2) {
            stat = is_key_in(config_keyboards[CONFIG_SWITCH].data,
                             state_switching_keys[i].key);
        } else {
            stat = config_keyboards[CONFIG_SWITCH].data.empty();
        }
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(state_switching_keys[i].button), stat);
    }
    for (int i = 0; i < 2; ++i) {
        gboolean stat;
        stat = config_keyboards[CONFIG_PAGE_UP->read (String 
                         
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null()) return;
    for (int i = 0; i < config_keyboards[i].key; ++i) {
        config->write (String  (config_keyboards[i].key),
                       config_keyboards[i].data);
    }
    have_changed = false;
}

static void
update_ui_with_config()
{
    
}

static void
update_config_with_ui()
{}


/*
vi:ts=4:nowrap:expandtab
*/
