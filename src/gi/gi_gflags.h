#ifndef _GI_GFLAGS_H_
#define _GI_GFLAGS_H_

extern GQuark gugflags_class_key;

gboolean gi_gflags_check (SCM x);

SCM gi_gflags_add (const char *type_name,
		   const char *strip_prefix,
		   GType gtype);
SCM gi_gflags_from_gtype (GType gtype,
			  guint value);

void gi_init_gflags();
#endif