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

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_DEBUG

#include <vector>
#include <algorithm>
#include <functional>
#include <gtk/gtk.h>
#include <scim.h>

#include <iostream>

//#include <gtk/scimkeyselection.h>
#include "google_intl.h"
#include "google_imengine_config_keys.h"

using namespace scim;
using namespace std;

#define scim_module_init googlepinyin_imengine_setup_LTX_scim_module_init
#define scim_module_exit googlepinyin_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       googlepinyin_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    googlepinyin_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        googlepinyin_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description googlepinyin_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     googlepinyin_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     googlepinyin_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   googlepinyin_imengine_setup_LTX_scim_setup_module_query_changed


static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer& config);
static void        save_config (const ConfigPointer& config);
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

struct ButtonOption
{
    const char *key;
    const char *label;
    bool        default_value;
    bool        current_value;
    GtkWidget  *button;
};

ButtonOption button_options[] =
{
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_MODE_SWITCH_SHIFT,
        "Shift",
        true,
        true,
        NULL
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_MODE_SWITCH_CONTROL,
        "Ctrl",
        false,
        false,
        NULL
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_MODE_SWITCH_NONE,
        "Do not use it",
        false,
        false,
        NULL
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_PAGE_MINUS_EQUAL,
        "Minus/Equal (-/=)",
        true,
        true,
        NULL
    },
    {
        SCIM_CONFIG_IMENGINE_GOOGLE_PINYIN_PAGE_COMMA_PERIOD,
        "Comman/Period (,/.)",
        false,
        false,
        NULL
    }
};

static const size_t N_BUTTON_OPTIONS = sizeof(button_options)/sizeof(button_options[0]);

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

// Function implementations.
static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (window) return window;
    
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
    OPT_SWITCH_SHIFT,
    OPT_SWITCH_CONTRL,
    OPT_SWITCH_NONE,
    OPT_PAGE_MINUS,
    OPT_PAGE_COMMA
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

    GSList *radio_group = NULL;
    for (int i = OPT_SWITCH_SHIFT; i <= OPT_SWITCH_NONE; ++i) {
        ButtonOption& opt = button_options[i];
        opt.button = gtk_radio_button_new_with_label(radio_group,
                                                     opt.label);
        gtk_widget_show(opt.button);
        gtk_box_pack_start(GTK_BOX(hbox), opt.button, FALSE, FALSE, 0);
        radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(opt.button));
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(opt.button), radio_group);
        gtk_container_set_border_width (GTK_CONTAINER (opt.button), 2);
        g_signal_connect(G_OBJECT(opt.button), "toggled",
                         G_CALLBACK(on_state_switch_toggled),
                         &opt);
    }
    return frame;
}

static void
on_state_switch_toggled(GtkToggleButton *button, gpointer user_data)
{
    ButtonOption *opt = (ButtonOption *)user_data;
    opt->current_value = gtk_toggle_button_get_active(button);
    have_changed = true;
}

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
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
    
    for (int i = OPT_PAGE_MINUS; i <= OPT_PAGE_COMMA; ++i) {
        ButtonOption& opt = button_options[i];
        opt.button = gtk_check_button_new_with_label(opt.label);
        gtk_widget_show(opt.button);
        gtk_box_pack_start(GTK_BOX(hbox), opt.button, FALSE, FALSE, 2);
        gtk_container_set_border_width (GTK_CONTAINER (opt.button), 2);
        g_signal_connect(G_OBJECT(opt.button), "toggled",
                         G_CALLBACK(on_page_flipping_toggled),
                         &opt);
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
    ButtonOption *opt = (ButtonOption *)user_data;
    opt->current_value = gtk_toggle_button_get_active(button);
    have_changed = true;
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
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
    return vbox;
}

static bool
query_changed()
{
    return have_changed;
}


static void
update_button_with_config(ButtonOption opt, const ConfigPointer* config)
{
    bool stat;
    stat = (*config)->read(String (opt.key), opt.default_value);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(opt.button), stat);
}

static void
load_config (const ConfigPointer& config)
{
    if (config.null()) return;
    for_each(button_options, button_options + N_BUTTON_OPTIONS,
             bind2nd(ptr_fun(update_button_with_config), &config));
    have_changed = false;
}

static void
update_config_with_button(const ConfigPointer* config, ButtonOption opt)
{
    bool stat;
    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(opt.button));
    (*config)->write(String(opt.key), stat);
}

static void
save_config (const ConfigPointer& config)
{
    if (config.null()) return;
    for_each(button_options, button_options + N_BUTTON_OPTIONS,
             bind1st(ptr_fun(update_config_with_button), &config));
    have_changed = false;
}


/*
vi:ts=4:nowrap:expandtab
*/
