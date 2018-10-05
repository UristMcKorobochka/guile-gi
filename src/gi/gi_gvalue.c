/* -*- Mode: C; c-basic-offset: 4 -*- */
#include <girepository.h>
#include "gi_gvalue.h"
#include "gi_genum.h"
#include "gi_gflags.h"
#include "gi_gtype.h"
#include "gir_xguile.h"
#include "gi_gobject.h"


SCM
gi_param_gvalue_as_scm (const GValue *gvalue,
			gboolean copy_boxed,
			const GParamSpec *pspec)
{
    if (G_IS_PARAM_SPEC_UNICHAR(pspec)) {
        scm_t_wchar u;
        gchar *encoded;
        SCM retval;

        u = g_value_get_uint (gvalue);
	return SCM_MAKE_CHAR (u);
    }
    else {
	return gi_gvalue_as_scm (gvalue, copy_boxed);
    }
}


GIArgument
gi_giargument_from_g_value(const GValue *value,
			 GITypeInfo *type_info)
{
    GIArgument arg = { 0, };

    GITypeTag type_tag = g_type_info_get_tag (type_info);

    /* For the long handling: long can be equivalent to
       int32 or int64, depending on the architecture, but
       gi doesn't tell us (and same for ulong)
    */
    switch (type_tag) {
        case GI_TYPE_TAG_BOOLEAN:
            arg.v_boolean = g_value_get_boolean (value);
            break;
        case GI_TYPE_TAG_INT8:
            arg.v_int8 = g_value_get_schar (value);
            break;
        case GI_TYPE_TAG_INT16:
        case GI_TYPE_TAG_INT32:
	    if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_LONG))
		arg.v_int32 = (gint32)g_value_get_long (value);
	    else
		arg.v_int32 = (gint32)g_value_get_int (value);
            break;
        case GI_TYPE_TAG_INT64:
	    if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_LONG))
		arg.v_int64 = g_value_get_long (value);
	    else
		arg.v_int64 = g_value_get_int64 (value);
            break;
        case GI_TYPE_TAG_UINT8:
            arg.v_uint8 = g_value_get_uchar (value);
            break;
        case GI_TYPE_TAG_UINT16:
        case GI_TYPE_TAG_UINT32:
	    if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_ULONG))
		arg.v_uint32 = (guint32)g_value_get_ulong (value);
	    else
		arg.v_uint32 = (guint32)g_value_get_uint (value);
            break;
        case GI_TYPE_TAG_UINT64:
	    if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_ULONG))
		arg.v_uint64 = g_value_get_ulong (value);
	    else
		arg.v_uint64 = g_value_get_uint64 (value);
            break;
        case GI_TYPE_TAG_UNICHAR:
            arg.v_uint32 = g_value_get_schar (value);
            break;
        case GI_TYPE_TAG_FLOAT:
            arg.v_float = g_value_get_float (value);
            break;
        case GI_TYPE_TAG_DOUBLE:
            arg.v_double = g_value_get_double (value);
            break;
        case GI_TYPE_TAG_GTYPE:
            arg.v_size = g_value_get_gtype (value);
            break;
        case GI_TYPE_TAG_UTF8:
        case GI_TYPE_TAG_FILENAME:
            /* Callers are responsible for ensuring the GValue stays alive
             * long enough for the string to be copied. */
            arg.v_string = (char *)g_value_get_string (value);
            break;
        case GI_TYPE_TAG_GLIST:
        case GI_TYPE_TAG_GSLIST:
        case GI_TYPE_TAG_ARRAY:
        case GI_TYPE_TAG_GHASH:
            if (G_VALUE_HOLDS_BOXED (value))
                arg.v_pointer = g_value_get_boxed (value);
            else
                /* e. g. GSettings::change-event */
                arg.v_pointer = g_value_get_pointer (value);
            break;
        case GI_TYPE_TAG_INTERFACE:
        {
            GIBaseInfo *info;
            GIInfoType info_type;

            info = g_type_info_get_interface (type_info);
            info_type = g_base_info_get_type (info);

            g_base_info_unref (info);

            switch (info_type) {
                case GI_INFO_TYPE_FLAGS:
                    arg.v_uint = g_value_get_flags (value);
                    break;
                case GI_INFO_TYPE_ENUM:
                    arg.v_int = g_value_get_enum (value);
                    break;
                case GI_INFO_TYPE_INTERFACE:
                case GI_INFO_TYPE_OBJECT:
                    if (G_VALUE_HOLDS_PARAM (value))
                      arg.v_pointer = g_value_get_param (value);
                    else
                      arg.v_pointer = g_value_get_object (value);
                    break;
                case GI_INFO_TYPE_BOXED:
                case GI_INFO_TYPE_STRUCT:
                case GI_INFO_TYPE_UNION:
                    if (G_VALUE_HOLDS (value, G_TYPE_BOXED)) {
                        arg.v_pointer = g_value_get_boxed (value);
                    } else if (G_VALUE_HOLDS (value, G_TYPE_VARIANT)) {
                        arg.v_pointer = g_value_get_variant (value);
                    } else if (G_VALUE_HOLDS (value, G_TYPE_POINTER)) {
                        arg.v_pointer = g_value_get_pointer (value);
                    } else {
                        /* PyErr_Format (PyExc_NotImplementedError, */
                        /*               "Converting GValue's of type '%s' is not implemented.", */
                        /*               g_type_name (G_VALUE_TYPE (value))); */
			g_error ("Converting GValue's of type '%s' is not implemented.",
				 g_type_name (G_VALUE_TYPE (value)));
                    }
                    break;
                default:
                    /* PyErr_Format (PyExc_NotImplementedError, */
                    /*               "Converting GValue's of type '%s' is not implemented.", */
                    /*               g_info_type_to_string (info_type)); */
			g_error ("Converting GValue's of type '%s' is not implemented.",
				 g_info_type_to_string (info_type));
		    
                    break;
            }
            break;
        }
        case GI_TYPE_TAG_ERROR:
            arg.v_pointer = g_value_get_boxed (value);
            break;
        case GI_TYPE_TAG_VOID:
            arg.v_pointer = g_value_get_pointer (value);
            break;
        default:
            break;
    }

    return arg;
}

static int
gi_gvalue_array_from_scm_list(GValue *value, SCM list)
{
    ssize_t len, i;
    GArray *array;

    len = scm_to_size_t (scm_length (list));

    array = g_array_new(FALSE, TRUE, sizeof(GValue));

    for (i = 0; i < len; ++i) {
	SCM item = scm_list_ref (list, scm_from_size_t (i));
        GType type;
        GValue item_value = { 0, };

	type = gi_infer_gtype_from_scm (item);

        g_value_init(&item_value, type);
        gi_gvalue_from_scm(&item_value, item);

        g_array_append_val(array, item_value);
    }

    g_value_take_boxed(value, array);
    return 0;
}


/**
 * pygi_value_to_py_basic_type:
 * @value: the GValue object.
 * @handled: (out): TRUE if the return value is defined
 *
 * This function creates/returns a Python wrapper object that
 * represents the GValue passed as an argument limited to supporting basic types
 * like ints, bools, and strings.
 *
 * Returns: a PyObject representing the value.
 */
SCM
gi_gvalue_to_scm_basic_type (const GValue *value, GType fundamental, gboolean *handled)
{
    *handled = TRUE;
    switch (fundamental) {
        case G_TYPE_CHAR:
            return scm_from_int8 (g_value_get_schar (value));
        case G_TYPE_UCHAR:
            return scm_from_uint8 (g_value_get_uchar (value));
        case G_TYPE_BOOLEAN:
            return scm_from_bool (g_value_get_boolean (value));
        case G_TYPE_INT:
            return scm_from_int (g_value_get_int (value));
        case G_TYPE_UINT:
            return scm_from_uint (g_value_get_uint (value));
        case G_TYPE_LONG:
            return scm_from_long (g_value_get_long(value));
        case G_TYPE_ULONG:
            return scm_from_ulong (g_value_get_ulong (value));
        case G_TYPE_INT64:
            return scm_from_int64 (g_value_get_int64 (value));
        case G_TYPE_UINT64:
            return scm_from_uint64 (g_value_get_uint64 (value));
        case G_TYPE_ENUM:
            /* return gi_genum_from_gtype (G_VALUE_TYPE (value), */
            /*                             g_value_get_enum (value)); */
	    return scm_from_ulong (g_value_get_enum (value));
        case G_TYPE_FLAGS:
            /* return gi_gflags_from_gtype (G_VALUE_TYPE (value), */
            /*                              g_value_get_flags (value)); */
	    return scm_from_ulong (g_value_get_flags (value));
        case G_TYPE_FLOAT:
            return scm_from_double (g_value_get_float (value));
        case G_TYPE_DOUBLE:
            return scm_from_double (g_value_get_double (value));
        case G_TYPE_STRING:
            return scm_from_utf8_string (g_value_get_string (value));
        default:
            *handled = FALSE;
            return NULL;
    }
}

/**
 * value_to_py_structured_type:
 * @value: the GValue object.
 * @copy_boxed: true if boxed values should be copied.
 *
 * This function creates/returns a Python wrapper object that
 * represents the GValue passed as an argument.
 *
 * Returns: a PyObject representing the value or NULL and sets an error;
 */
static SCM
gi_gvalue_to_scm_structured_type (const GValue *value, GType fundamental, gboolean copy_boxed)
{
    // const gchar *type_name;
    return SCM_BOOL_F;
#if 0    
    switch (fundamental) {
    case G_TYPE_INTERFACE:
        if (g_type_is_a(G_VALUE_TYPE(value), G_TYPE_OBJECT))
            return pygobject_new(g_value_get_object(value));
        else
            break;

    case G_TYPE_POINTER:
        if (G_VALUE_HOLDS_GTYPE (value))
            return pyg_type_wrapper_new (g_value_get_gtype (value));
        else
            return pyg_pointer_new(G_VALUE_TYPE(value),
                    g_value_get_pointer(value));
    case G_TYPE_BOXED: {
        PyGTypeMarshal *bm;
        gboolean holds_value_array;

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        holds_value_array = G_VALUE_HOLDS(value, G_TYPE_VALUE_ARRAY);
        G_GNUC_END_IGNORE_DEPRECATIONS

        if (G_VALUE_HOLDS(value, PY_TYPE_OBJECT)) {
            PyObject *ret = (PyObject *)g_value_dup_boxed(value);
            if (ret == NULL) {
                Py_INCREF(Py_None);
                return Py_None;
            }
            return ret;
        } else if (G_VALUE_HOLDS(value, G_TYPE_VALUE)) {
            GValue *n_value = g_value_get_boxed (value);
            return pyg_value_as_pyobject(n_value, copy_boxed);
        } else if (holds_value_array) {
            GValueArray *array = (GValueArray *) g_value_get_boxed(value);
            Py_ssize_t n_values = array ? array->n_values : 0;
            PyObject *ret = PyList_New(n_values);
            int i;
            for (i = 0; i < n_values; ++i)
                PyList_SET_ITEM(ret, i, pyg_value_as_pyobject
                        (array->values + i, copy_boxed));
            return ret;
        } else if (G_VALUE_HOLDS(value, G_TYPE_GSTRING)) {
            GString *string = (GString *) g_value_get_boxed(value);
            PyObject *ret = PYGLIB_PyUnicode_FromStringAndSize(string->str, string->len);
            return ret;
        }
        bm = pyg_type_lookup(G_VALUE_TYPE(value));
        if (bm) {
            return bm->fromvalue(value);
        } else {
            if (copy_boxed)
                return pygi_gboxed_new(G_VALUE_TYPE(value),
                        g_value_get_boxed(value), TRUE, TRUE);
            else
                return pygi_gboxed_new(G_VALUE_TYPE(value),
                        g_value_get_boxed(value),FALSE,FALSE);
        }
    }
    case G_TYPE_PARAM:
        return pyg_param_spec_new(g_value_get_param(value));
    case G_TYPE_OBJECT:
        return pygobject_new(g_value_get_object(value));
    case G_TYPE_VARIANT:
    {
        GVariant *v = g_value_get_variant(value);
        if (v == NULL) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return pygi_struct_new_from_g_type (G_TYPE_VARIANT, g_variant_ref(v), FALSE);
    }
    default:
    {
        PyGTypeMarshal *bm;
        if ((bm = pyg_type_lookup(G_VALUE_TYPE(value))))
            return bm->fromvalue(value);
        break;
    }
    }

    type_name = g_type_name (G_VALUE_TYPE (value));
    if (type_name == NULL) {
        type_name = "(null)";
    }
    PyErr_Format (PyExc_TypeError, "unknown type %s", type_name);
    return NULL;
#endif
}


/* Returns an SCM version of the GValue.  If COPY_BOXED,
   try to make a deep copy of the object. */
SCM
gi_gvalue_as_scm (const GValue *value, gboolean copy_boxed)
{
    SCM guobj;
    gboolean handled;
    GType fundamental = G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value));

    if (fundamental == G_TYPE_CHAR)
	return SCM_MAKE_CHAR (g_value_get_schar (value));
    else if (fundamental == G_TYPE_UCHAR)
	return SCM_MAKE_CHAR (g_value_get_uchar (value));

    guobj = gi_gvalue_to_scm_basic_type (value, fundamental, &handled);
    if (handled)
	return guobj;

    guobj = gi_gvalue_to_scm_structured_type (value, fundamental, copy_boxed);
    return guobj;
}

/**
 * pyg_value_from_pyobject_with_error:
 * @value: the GValue object to store the converted value in.
 * @obj: the Python object to convert.
 *
 * This function converts a Python object and stores the result in a
 * GValue.  The GValue must be initialised in advance with
 * g_value_init().  If the Python object can't be converted to the
 * type of the GValue, then an error is returned.
 *
 * Returns: 0 on success, -1 on error.
 */
int
gi_gvalue_from_scm_with_error(GValue *value, SCM obj)
{
    GType value_type = G_VALUE_TYPE(value);

    switch (G_TYPE_FUNDAMENTAL(value_type)) {
#if 0	
    case G_TYPE_INTERFACE:
        /* we only handle interface types that have a GObject prereq */
        if (g_type_is_a(value_type, G_TYPE_OBJECT)) {
            if (scm_is_false (obj))
                g_value_set_object(value, NULL);
            else {
                if (!SCM_IS_A_P (obj, gi_gobject_type)) {
		    scm_misc_error ("gvalue_from_scm", "GObject is required", SCM_EOL);
                    return -1;
		}
                if (!G_TYPE_CHECK_INSTANCE_TYPE(gi_gobject_get_obj(obj),
						value_type)) {
		    scm_misc_error ("gvalue_from_scm",
				    "Invalie GObject type for assignment",
				    SCM_EOL);
                    return -1;
                }
                g_value_set_object(value, gi_gobject_get_obj(obj));
            }
	} else {
	    scm_misc_error ("gvalue_from_scm", "Unsupported conversion", SCM_EOL);
            return -1;
        }
        break;
#endif
    case G_TYPE_CHAR:
    {
        gint8 temp;
        temp = scm_to_int8 (obj);
	g_value_set_schar (value, temp);
	return 0;
    }
    case G_TYPE_UCHAR:
    {
        guchar temp;
	temp = scm_to_uint8 (obj);
	g_value_set_uchar (value, temp);
	return 0;
    }
    case G_TYPE_BOOLEAN:
    {
        gboolean temp;
	temp = scm_is_true (obj);
	g_value_set_boolean (value, temp);
	return 0;
    }
    case G_TYPE_INT:
    {
        gint temp;
	temp = scm_to_int (obj);
	g_value_set_int (value, temp);
	return 0;
    }
    case G_TYPE_UINT:
    {
        guint temp;
	temp = scm_to_uint (obj);
	g_value_set_uint (value, temp);
	return 0;
    }
    case G_TYPE_LONG:
    {
        glong temp;
	temp = scm_to_ulong (obj);
	g_value_set_long (value, temp);
	return 0;
    }
    case G_TYPE_ULONG:
    {
        gulong temp;
	temp = scm_to_ulong (obj);
	g_value_set_ulong (value, temp);
	return 0;
    }
    case G_TYPE_INT64:
    {
        gint64 temp;
	temp = scm_to_int64 (obj);
	g_value_set_int64 (value, temp);
	return 0;
    }
    case G_TYPE_UINT64:
    {
        guint64 temp;
	temp = scm_to_uint64 (obj);
	g_value_set_uint64 (value, temp);
	return 0;
    }
    case G_TYPE_ENUM:
    {
        gint val = 0;
        /* if (gi_enum_get_value(G_VALUE_TYPE(value), obj, &val) < 0) { */
        /*     return -1; */
        /* } */
	val = scm_to_ulong(obj);
        g_value_set_enum(value, val);
    }
    break;
    case G_TYPE_FLAGS:
    {
        guint val = 0;
        /* if (gi_flags_get_value(G_VALUE_TYPE(value), obj, &val) < 0) { */
        /*     return -1; */
        /* } */
	val = scm_to_ulong(obj);
        g_value_set_flags(value, val);
        return 0;
    }
    break;
    case G_TYPE_FLOAT:
    {
        gfloat temp;
	temp = scm_to_double (obj);
	g_value_set_float (value, temp);
	return 0;
    }
    case G_TYPE_DOUBLE:
    {
        gdouble temp;
	temp = scm_to_double (obj);
	g_value_set_double (value, temp);
	return 0;
    }
    case G_TYPE_STRING:
    {
        gchar *temp;
	temp = scm_to_utf8_string (obj);
	g_value_take_string (value, temp);
	return 0;
    }
    case G_TYPE_POINTER:
#if 0
	if (0)
	    ;
        else if (PyObject_TypeCheck(obj, &PyGPointer_Type) &&
                G_VALUE_HOLDS(value, ((PyGPointer *)obj)->gtype))
            g_value_set_pointer(value, pyg_pointer_get(obj, gpointer));
        else if (PyCapsule_CheckExact (obj))
            g_value_set_pointer(value, PyCapsule_GetPointer (obj, NULL));
        else if (G_VALUE_HOLDS_GTYPE (value))
            g_value_set_gtype (value, pyg_type_from_object (obj));
#endif
	scm_misc_error("gvalue_from_scm", "expected pointer", SCM_EOL);
	break;
#if 0	
    case G_TYPE_BOXED: {
        PyGTypeMarshal *bm;
        gboolean holds_value_array;

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        holds_value_array = G_VALUE_HOLDS(value, G_TYPE_VALUE_ARRAY);
        G_GNUC_END_IGNORE_DEPRECATIONS

        if (obj == Py_None)
            g_value_set_boxed(value, NULL);
        else if (G_VALUE_HOLDS(value, PY_TYPE_OBJECT))
            g_value_set_boxed(value, obj);
        else if (PyObject_TypeCheck(obj, &PyGBoxed_Type) &&
                G_VALUE_HOLDS(value, ((PyGBoxed *)obj)->gtype))
            g_value_set_boxed(value, pyg_boxed_get(obj, gpointer));
        else if (G_VALUE_HOLDS(value, G_TYPE_VALUE)) {
            GType type;
            GValue *n_value;

            type = pyg_type_from_object((PyObject*)Py_TYPE(obj));
            if (G_UNLIKELY (! type)) {
                return -1;
            }
            n_value = g_new0 (GValue, 1);
            g_value_init (n_value, type);
            g_value_take_boxed (value, n_value);
            return pyg_value_from_pyobject_with_error (n_value, obj);
        }
        else if (PySequence_Check(obj) && holds_value_array)
            return pyg_value_array_from_pyobject(value, obj, NULL);

        else if (PySequence_Check(obj) &&
                G_VALUE_HOLDS(value, G_TYPE_ARRAY))
            return pyg_array_from_pyobject(value, obj);
        else if (PYGLIB_PyUnicode_Check(obj) &&
                G_VALUE_HOLDS(value, G_TYPE_GSTRING)) {
            GString *string;
            char *buffer;
            Py_ssize_t len;
            if (PYGLIB_PyUnicode_AsStringAndSize(obj, &buffer, &len))
                return -1;
            string = g_string_new_len(buffer, len);
            g_value_set_boxed(value, string);
            g_string_free (string, TRUE);
            break;
        }
        else if ((bm = pyg_type_lookup(G_VALUE_TYPE(value))) != NULL)
            return bm->tovalue(value, obj);
        else if (PyCapsule_CheckExact (obj))
            g_value_set_boxed(value, PyCapsule_GetPointer (obj, NULL));
        else {
            PyErr_SetString(PyExc_TypeError, "Expected Boxed");
            return -1;
        }
        break;
    }
    case G_TYPE_PARAM:
        /* we need to support both the wrapped _gi.GParamSpec and the GI
         * GObject.ParamSpec */
        if (G_IS_PARAM_SPEC (pygobject_get (obj)))
            g_value_set_param(value, G_PARAM_SPEC (pygobject_get (obj)));
        else if (pyg_param_spec_check (obj))
            g_value_set_param(value, PyCapsule_GetPointer (obj, NULL));
        else {
            PyErr_SetString(PyExc_TypeError, "Expected ParamSpec");
            return -1;
        }
        break;
    case G_TYPE_OBJECT:
        if (obj == Py_None) {
            g_value_set_object(value, NULL);
        } else if (PyObject_TypeCheck(obj, &PyGObject_Type) &&
		   G_TYPE_CHECK_INSTANCE_TYPE(pygobject_get(obj),
					      G_VALUE_TYPE(value))) {
            g_value_set_object(value, pygobject_get(obj));
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected GObject");
            return -1;
        }
        break;
    case G_TYPE_VARIANT:
	{
	    if (obj == Py_None)
		g_value_set_variant(value, NULL);
	    else if (pyg_type_from_object_strict(obj, FALSE) == G_TYPE_VARIANT)
		g_value_set_variant(value, pyg_boxed_get(obj, GVariant));
	    else {
		PyErr_SetString(PyExc_TypeError, "Expected Variant");
		return -1;
	    }
	    break;
	}
#endif    
    default:
	{
#if 0	
	    PyGTypeMarshal *bm;
	    if ((bm = pyg_type_lookup(G_VALUE_TYPE(value))) != NULL) {
		return bm->tovalue(value, obj);
	    } else {
		PyErr_SetString(PyExc_TypeError, "Unknown value type");
		return -1;
	    }
#endif
	    break;
	}
    }

    /* If an error occurred, unset the GValue but don't clear the Python error. */
    /* if (PyErr_Occurred()) { */
    /*     g_value_unset(value); */
    /*     return -1; */
    /* } */

    return 0;
}

int
gi_gvalue_from_scm (GValue *value, SCM obj)
{
    int res = gi_gvalue_from_scm_with_error (value, obj);
    return res;
}

static SCM
scm_gvalue_set_x (SCM self, SCM x)
{
    GValue *val;
    
    scm_assert_foreign_object_type (gi_gvalue_type, self);
    val = gi_gvalue_get_value (self);
    gi_gvalue_from_scm (val, x);
    return SCM_UNSPECIFIED;
}

static SCM
scm_gvalue_get (SCM self)
{
    GValue *val;
    
    scm_assert_foreign_object_type (gi_gvalue_type, self);
    val = gi_gvalue_get_value (self);
    return gi_gvalue_as_scm (val, TRUE);
}

static SCM
scm_make_gvalue (SCM gtype)
{
    GType type;
    GValue *val;
    
    scm_assert_foreign_object_type (gi_gtype_type, gtype);
    
    type = gi_gtype_get_type (gtype);
    val = g_new0(GValue,1);
    g_value_init (val, type);
    return gi_gvalue_c2g (val);
    
}

SCM
gi_gvalue_c2g (GValue *val)
{
    if (val)
	return scm_make_foreign_object_1 (gi_gvalue_type, val);
    
    return SCM_BOOL_F;
}

void
gi_gvalue_finalizer (SCM self)
{
    GValue *val;
    val = gi_gvalue_get_value (self);
    if (val) {
	g_value_unset (val);
	g_free (val);
	gi_gvalue_set_value (self, (GValue *) NULL);
    }
}

static SCM
scm_gvalue_type_name (SCM self)
{
    GValue *val;
    const gchar *name;

    scm_assert_foreign_object_type (gi_gvalue_type, self);
    val = gi_gvalue_get_value (self);
    if (val) {
	name = G_VALUE_TYPE_NAME (val);
	if (name)
	    return scm_from_utf8_string (name);
	else
	    return scm_from_utf8_string ("(unknown)");
    }
    return SCM_BOOL_F;
}

static SCM
scm_gvalue_to_gtype (SCM self)
{
    GValue *val;
    GType type;
    
    scm_assert_foreign_object_type (gi_gvalue_type, self);
    val = gi_gvalue_get_value (self);
    if (val) {
	type = G_VALUE_TYPE (val);
	return gi_gtype_c2g (type);
    }
    return SCM_BOOL_F;
}

static SCM
scm_gvalue_holds_p(SCM self, SCM gtype)
{
    GValue *val;
    GType type;
    gboolean ret;
    
    scm_assert_foreign_object_type (gi_gvalue_type, self);
    scm_assert_foreign_object_type (gi_gtype_type, gtype);
    
    val = gi_gvalue_get_value (self);
    type = gi_gtype_get_type (gtype);
    if (val) {
	ret = G_VALUE_HOLDS (val, type);
	return scm_from_bool (ret);
    }
    return SCM_BOOL_F;
}

static SCM
scm_gvalue_valid_p (SCM self)
{
    GValue *val;
    gboolean ret;
    
    scm_assert_foreign_object_type (gi_gvalue_type, self);
    val = gi_gvalue_get_value (self);
    if (val) {
	ret = G_IS_VALUE (val);
	return scm_from_bool (ret);
    }
    return SCM_BOOL_F;
}

void
gi_init_gvalue (void)
{
    gi_init_gvalue_type ();

    scm_c_define_gsubr ("gvalue-get", 1, 0, 0, scm_gvalue_get);
    scm_c_define_gsubr ("gvalue-set!", 2, 0, 0, scm_gvalue_set_x);
    scm_c_define_gsubr ("make-gvalue", 1, 0, 0, scm_make_gvalue);
    scm_c_define_gsubr ("gvalue-type-name", 1, 0, 0, scm_gvalue_type_name);
    scm_c_define_gsubr ("gvalue->gtype", 1, 0, 0, scm_gvalue_to_gtype);
    scm_c_define_gsubr ("gvalue-holds?", 2, 0, 0, scm_gvalue_holds_p);
    scm_c_define_gsubr ("gvalue-valid?", 1, 0, 0, scm_gvalue_valid_p);

    scm_c_export ("make-gvalue",
		  "gvalue-type-name",
		  "gvalue->gtype",
		  "gvalue-holds?",
		  "gvalue-valid?",
		  NULL);
}
