#ifndef GOOGLEPINYIN_PRIVATE_H
#define GOOGLEPINYIN_PRIVATE_H

// Include scim-sunpin configuration header
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#if defined(HAVE_LIBINTL_H) && defined(ENABLE_NLS)
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#endif // GOOGLEPINYIN_PRIVATE_H
