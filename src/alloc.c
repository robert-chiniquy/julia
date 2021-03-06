/*
  object constructors
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include "julia.h"
#include "newobj_internal.h"
#include "builtin_proto.h"

jl_value_t *jl_true;
jl_value_t *jl_false;

jl_tag_type_t *jl_undef_type;
jl_tvar_t     *jl_typetype_tvar;
jl_tag_type_t *jl_typetype_type;
jl_value_t    *jl_ANY_flag;
jl_typector_t *jl_function_type;
jl_struct_type_t *jl_box_type;
jl_type_t *jl_box_any_type;
jl_typename_t *jl_box_typename;

jl_struct_type_t *jl_typector_type;

jl_struct_type_t *jl_array_type;
jl_typename_t *jl_array_typename;
jl_type_t *jl_array_uint8_type;
jl_type_t *jl_array_any_type;
jl_struct_type_t *jl_weakref_type;
jl_struct_type_t *jl_ascii_string_type;
jl_struct_type_t *jl_utf8_string_type;
jl_struct_type_t *jl_expr_type;
jl_struct_type_t *jl_symbolnode_type;
jl_struct_type_t *jl_linenumbernode_type;
jl_struct_type_t *jl_labelnode_type;
jl_struct_type_t *jl_gotonode_type;
jl_struct_type_t *jl_quotenode_type;
jl_struct_type_t *jl_topnode_type;
jl_bits_type_t *jl_intrinsic_type;
jl_struct_type_t *jl_methtable_type;
jl_struct_type_t *jl_method_type;
jl_struct_type_t *jl_lambda_info_type;
jl_struct_type_t *jl_module_type;
jl_struct_type_t *jl_errorexception_type=NULL;
jl_struct_type_t *jl_typeerror_type;
jl_struct_type_t *jl_loaderror_type;
jl_struct_type_t *jl_backtrace_type;
jl_bits_type_t *jl_pointer_type;
jl_value_t *jl_an_empty_cell=NULL;
jl_value_t *jl_stackovf_exception;
jl_value_t *jl_divbyzero_exception;
jl_value_t *jl_undefref_exception;
jl_value_t *jl_interrupt_exception;
jl_value_t *jl_memory_exception;

jl_sym_t *call_sym;    jl_sym_t *dots_sym;
jl_sym_t *call1_sym;   jl_sym_t *module_sym;
jl_sym_t *quote_sym;   jl_sym_t *amp_sym;
jl_sym_t *top_sym;     jl_sym_t *colons_sym;
jl_sym_t *line_sym;    jl_sym_t *jl_continue_sym;
// head symbols for each expression type
jl_sym_t *goto_sym;    jl_sym_t *goto_ifnot_sym;
jl_sym_t *label_sym;   jl_sym_t *return_sym;
jl_sym_t *lambda_sym;  jl_sym_t *assign_sym;
jl_sym_t *null_sym;    jl_sym_t *body_sym;
jl_sym_t *macro_sym;   jl_sym_t *method_sym;
jl_sym_t *enter_sym;   jl_sym_t *leave_sym;
jl_sym_t *exc_sym;     jl_sym_t *error_sym;
jl_sym_t *static_typeof_sym;
jl_sym_t *new_sym;     jl_sym_t *multivalue_sym;
jl_sym_t *const_sym;   jl_sym_t *thunk_sym;
jl_sym_t *anonymous_sym;  jl_sym_t *underscore_sym;

DLLEXPORT jl_value_t *jl_new_struct(jl_struct_type_t *type, ...)
{
    if (type->instance != NULL) return type->instance;
    va_list args;
    size_t nf = type->names->length;
    size_t i;
    va_start(args, type);
    jl_value_t *jv = newobj((jl_type_t*)type, nf);
    for(i=0; i < nf; i++) {
        ((jl_value_t**)jv)[i+1] = va_arg(args, jl_value_t*);
    }
    if (nf == 0) type->instance = jv;
    va_end(args);
    return jv;
}

DLLEXPORT jl_value_t *jl_new_struct_uninit(jl_struct_type_t *type)
{
    if (type->instance != NULL) return type->instance;
    size_t nf = type->names->length;
    size_t i;
    jl_value_t *jv = newobj((jl_type_t*)type, nf);
    for(i=0; i < nf; i++) {
        ((jl_value_t**)jv)[i+1] = NULL;
    }
    if (nf == 0) type->instance = jv;
    return jv;
}

DLLEXPORT jl_value_t *jl_new_structt(jl_struct_type_t *type, jl_tuple_t *t)
{
    assert(type->names->length == t->length);
    jl_value_t *jv = jl_new_struct_uninit(type);
    for(size_t i=0; i < t->length; i++) {
        ((jl_value_t**)jv)[i+1] = jl_tupleref(t, i);
    }
    return jv;
}

jl_tuple_t *jl_tuple(size_t n, ...)
{
    va_list args;
    size_t i;
    if (n == 0) return jl_null;
    va_start(args, n);
    jl_tuple_t *jv = (jl_tuple_t*)newobj((jl_type_t*)jl_tuple_type, n+1);
    jv->length = n;
    for(i=0; i < n; i++) {
        jl_tupleset(jv, i, va_arg(args, jl_value_t*));
    }
    va_end(args);
    return jv;
}

jl_tuple_t *jl_tuple1(void *a)
{
    jl_tuple_t *t = (jl_tuple_t*)alloc_3w();
    t->type = (jl_type_t*)jl_tuple_type;
    t->length = 1;
    jl_tupleset(t, 0, a);
    return t;
}

jl_tuple_t *jl_tuple2(void *a, void *b)
{
    jl_tuple_t *t = (jl_tuple_t*)alloc_4w();
    t->type = (jl_type_t*)jl_tuple_type;
    t->length = 2;
    jl_tupleset(t, 0, a);
    jl_tupleset(t, 1, b);
    return t;
}

jl_tuple_t *jl_alloc_tuple_uninit(size_t n)
{
    if (n == 0) return jl_null;
    jl_tuple_t *jv = (jl_tuple_t*)newobj((jl_type_t*)jl_tuple_type, n+1);
    jv->length = n;
    return jv;
}

jl_tuple_t *jl_alloc_tuple(size_t n)
{
    if (n == 0) return jl_null;
    jl_tuple_t *jv = jl_alloc_tuple_uninit(n);
    size_t i;
    for(i=0; i < n; i++) {
        jl_tupleset(jv, i, NULL);
    }
    return jv;
}

jl_tuple_t *jl_tuple_append(jl_tuple_t *a, jl_tuple_t *b)
{
    jl_tuple_t *c = jl_alloc_tuple_uninit(a->length + b->length);
    size_t i=0, j;
    for(j=0; j < a->length; j++) {
        jl_tupleset(c, i, jl_tupleref(a,j));
        i++;
    }
    for(j=0; j < b->length; j++) {
        jl_tupleset(c, i, jl_tupleref(b,j));
        i++;
    }
    return c;
}

jl_tuple_t *jl_tuple_fill(size_t n, jl_value_t *v)
{
    if (n==0) return jl_null;
    jl_tuple_t *tup = jl_alloc_tuple_uninit(n);
    size_t i;
    for(i=0; i < n; i++) {
        jl_tupleset(tup, i, v);
    }
    return tup;
}

jl_function_t *jl_new_closure(jl_fptr_t proc, jl_value_t *env)
{
    jl_function_t *f = (jl_function_t*)alloc_4w();
    f->type = (jl_type_t*)jl_any_func;
    f->fptr = proc;
    f->env = env;
    f->linfo = NULL;
    return f;
}

DLLEXPORT
jl_lambda_info_t *jl_new_lambda_info(jl_value_t *ast, jl_tuple_t *sparams)
{
    jl_lambda_info_t *li =
        (jl_lambda_info_t*)newobj((jl_type_t*)jl_lambda_info_type,
                                  LAMBDA_INFO_NW);
    li->ast = ast;
    li->file = (jl_value_t*)null_sym;
    li->line = jl_box_long(0);
    if (ast != NULL && jl_is_expr(ast)) {
        jl_expr_t *body1 = (jl_expr_t*)jl_exprarg(jl_lam_body((jl_expr_t*)ast),0);
        if (jl_is_expr(body1) && ((jl_expr_t*)body1)->head == line_sym) {
            li->file = jl_exprarg(body1, 1);
            li->line = jl_exprarg(body1, 0);
        }
    }
    li->module = jl_current_module;
    li->sparams = sparams;
    li->tfunc = (jl_value_t*)jl_null;
    li->fptr = NULL;
    li->roots = NULL;
    li->functionObject = NULL;
    li->specTypes = NULL;
    li->inferred = jl_false;
    li->inInference = 0;
    li->inCompile = 0;
    li->unspecialized = NULL;
    li->specializations = NULL;
    li->name = anonymous_sym;
    return li;
}

// symbols --------------------------------------------------------------------

static jl_sym_t *symtab = NULL;

static jl_sym_t *mk_symbol(const char *str)
{
    jl_sym_t *sym;
    size_t len = strlen(str);

    sym = (jl_sym_t*)malloc(sizeof(jl_sym_t)-sizeof(void*) + len + 1);
    sym->type = (jl_type_t*)jl_sym_type;
    sym->left = sym->right = NULL;
#ifdef __LP64__
    sym->hash = memhash(str, len)^0xAAAAAAAAAAAAAAAAL;
#else
    sym->hash = memhash32(str, len)^0xAAAAAAAA;
#endif
    strcpy(&sym->name[0], str);
    return sym;
}

static void unmark_symbols_(jl_sym_t *root)
{
    while (root != NULL) {
        root->type = (jl_type_t*)(((uptrint_t)root->type)&~1UL);
        unmark_symbols_(root->left);
        root = root->right;
    }
}

void jl_unmark_symbols(void) { unmark_symbols_(symtab); }

static jl_sym_t **symtab_lookup(jl_sym_t **ptree, const char *str)
{
    int x;

    while(*ptree != NULL) {
        x = strcmp(str, (*ptree)->name);
        if (x == 0)
            return ptree;
        if (x < 0)
            ptree = &(*ptree)->left;
        else
            ptree = &(*ptree)->right;
    }
    return ptree;
}

jl_sym_t *jl_symbol(const char *str)
{
    jl_sym_t **pnode;

    pnode = symtab_lookup(&symtab, str);
    if (*pnode == NULL)
        *pnode = mk_symbol(str);
    return *pnode;
}

DLLEXPORT jl_sym_t *jl_symbol_n(const char *str, int32_t len)
{
    char name[len+1];
    memcpy(name, str, len);
    name[len] = '\0';
    return jl_symbol(name);
}

DLLEXPORT jl_sym_t *jl_get_root_symbol() { return symtab; }

static uint32_t gs_ctr = 0;  // TODO: per-thread
uint32_t jl_get_gs_ctr(void) { return gs_ctr; }
void jl_set_gs_ctr(uint32_t ctr) { gs_ctr = ctr; }

DLLEXPORT jl_sym_t *jl_gensym(void)
{
    static char name[16];
    char *n;
    n = uint2str(&name[2], sizeof(name)-2, gs_ctr, 10);
    *(--n) = '#'; *(--n) = '#';
    gs_ctr++;
    return jl_symbol(n);
}

// allocating types -----------------------------------------------------------

jl_typename_t *jl_new_typename(jl_sym_t *name)
{
    jl_typename_t *tn=(jl_typename_t*)newobj((jl_type_t*)jl_typename_type, 3);
    tn->name = name;
    tn->primary = NULL;
    tn->cache = jl_null;
    return tn;
}

static void unbind_tvars(jl_tuple_t *parameters)
{
    size_t i;
    for(i=0; i < parameters->length; i++) {
        jl_tvar_t *tv = (jl_tvar_t*)jl_tupleref(parameters, i);
        if (jl_is_typevar(tv))
            tv->bound = 0;
    }
}

jl_tag_type_t *jl_new_tagtype(jl_value_t *name, jl_tag_type_t *super,
                              jl_tuple_t *parameters)
{
    jl_typename_t *tn=NULL;
    JL_GC_PUSH(&tn);

    if (jl_is_typename(name))
        tn = (jl_typename_t*)name;
    else
        tn = jl_new_typename((jl_sym_t*)name);
    jl_tag_type_t *t = (jl_tag_type_t*)newobj((jl_type_t*)jl_tag_kind,
                                              TAG_TYPE_NW);
    t->name = tn;
    t->super = super;
    unbind_tvars(parameters);
    t->parameters = parameters;
    t->fptr = NULL;
    t->env = NULL;
    t->linfo = NULL;
    if (t->name->primary == NULL)
        t->name->primary = (jl_value_t*)t;
    JL_GC_POP();
    return t;
}

jl_func_type_t *jl_new_functype(jl_type_t *a, jl_type_t *b)
{
    JL_GC_PUSH(&a);
    if (a != (jl_type_t*)jl_bottom_type &&
        !jl_subtype((jl_value_t*)a, (jl_value_t*)jl_tuple_type, 0) &&
        !jl_subtype((jl_value_t*)jl_tuple_type, (jl_value_t*)a, 0))
        a = (jl_type_t*)jl_tuple1(a);
    jl_func_type_t *t = (jl_func_type_t*)newobj((jl_type_t*)jl_func_kind, 2);
    t->from = a;
    t->to = b;
    JL_GC_POP();
    return t;
}

jl_function_t *jl_instantiate_method(jl_function_t *f, jl_tuple_t *sp);

void jl_add_constructors(jl_struct_type_t *t)
{
    if (t->name == jl_array_typename) {
        t->fptr = jl_f_no_function;
        return;
    }

    jl_initialize_generic_function((jl_function_t*)t, t->name->name);

    if (t->ctor_factory == (jl_value_t*)jl_nothing ||
        t->ctor_factory == (jl_value_t*)jl_null) {
        assert(t->parameters->length == 0);
    }
    else {
        assert(t->parameters->length > 0);
        if (t != (jl_struct_type_t*)t->name->primary) {
            // instantiating
            assert(jl_is_function(t->ctor_factory));
            
            // add type's static parameters to the ctor factory
            size_t np = t->parameters->length;
            jl_tuple_t *sparams = jl_alloc_tuple_uninit(np*2);
            jl_function_t *cfactory = NULL;
            JL_GC_PUSH(&sparams, &cfactory);
            size_t i;
            for(i=0; i < np; i++) {
                jl_tupleset(sparams, i*2+0,
                            jl_tupleref(((jl_struct_type_t*)t->name->primary)->parameters, i));
                jl_tupleset(sparams, i*2+1,
                            jl_tupleref(t->parameters, i));
            }
            cfactory = jl_instantiate_method((jl_function_t*)t->ctor_factory,
                                             sparams);
            cfactory->linfo->ast = jl_prepare_ast(cfactory->linfo,
                                                  cfactory->linfo->sparams);
            
            // call user-defined constructor factory on (type,)
            jl_value_t *cfargs[1] = { (jl_value_t*)t };
            jl_apply(cfactory, cfargs, 1);
            JL_GC_POP();
        }
    }
}

JL_CALLABLE(jl_f_ctor_trampoline)
{
    jl_add_constructors((jl_struct_type_t*)F);
    return jl_apply((jl_function_t*)F, args, nargs);
}

jl_struct_type_t *jl_new_struct_type(jl_sym_t *name, jl_tag_type_t *super,
                                     jl_tuple_t *parameters,
                                     jl_tuple_t *fnames, jl_tuple_t *ftypes)
{
    jl_typename_t *tn = jl_new_typename(name);
    JL_GC_PUSH(&tn);
    jl_struct_type_t *t = (jl_struct_type_t*)newobj((jl_type_t*)jl_struct_kind,
                                                    STRUCT_TYPE_NW);
    t->name = tn;
    t->name->primary = (jl_value_t*)t;
    t->super = super;
    unbind_tvars(parameters);
    t->parameters = parameters;
    t->names = fnames;
    t->types = ftypes;
    t->fptr = jl_f_ctor_trampoline;
    t->env = (jl_value_t*)t;
    t->linfo = NULL;
    t->ctor_factory = (jl_value_t*)jl_null;
    t->instance = NULL;
    if (!jl_is_leaf_type((jl_value_t*)t))
        t->uid = 0;
    else
        t->uid = jl_assign_type_uid();
    JL_GC_POP();
    return t;
}

extern int jl_boot_file_loaded;

jl_bits_type_t *jl_new_bitstype(jl_value_t *name, jl_tag_type_t *super,
                                jl_tuple_t *parameters, size_t nbits)
{
    jl_bits_type_t *t=NULL;
    jl_typename_t *tn=NULL;
    JL_GC_PUSH(&t, &tn);

    if (!jl_boot_file_loaded && jl_is_symbol(name)) {
        // hack to avoid making two versions of basic types needed
        // during bootstrapping
        if (!strcmp(((jl_sym_t*)name)->name, "Int32"))
            t = jl_int32_type;
        else if (!strcmp(((jl_sym_t*)name)->name, "Int64"))
            t = jl_int64_type;
        else if (!strcmp(((jl_sym_t*)name)->name, "Bool"))
            t = jl_bool_type;
    }
    int makenew = (t==NULL);
    if (makenew) {
        t = (jl_bits_type_t*)newobj((jl_type_t*)jl_bits_kind, BITS_TYPE_NW);
        if (jl_is_typename(name))
            tn = (jl_typename_t*)name;
        else
            tn = jl_new_typename((jl_sym_t*)name);
        t->name = tn;
    }
    t->super = super;
    unbind_tvars(parameters);
    t->parameters = parameters;
    if (jl_int32_type != NULL)
        t->bnbits = jl_box_int32(nbits);
    else
        t->bnbits = (jl_value_t*)jl_null;
    t->nbits = nbits;
    if (!jl_is_leaf_type((jl_value_t*)t))
        t->uid = 0;
    else if (makenew)
        t->uid = jl_assign_type_uid();
    t->fptr = NULL;
    t->env = NULL;
    t->linfo = NULL;
    if (t->name->primary == NULL)
        t->name->primary = (jl_value_t*)t;
    JL_GC_POP();
    return t;
}

jl_uniontype_t *jl_new_uniontype(jl_tuple_t *types)
{
    jl_uniontype_t *t = (jl_uniontype_t*)newobj((jl_type_t*)jl_union_kind, 1);
    // don't make unions of 1 type; Union(T)==T
    assert(types->length != 1);
    t->types = types;
    return t;
}

// type constructor -----------------------------------------------------------

jl_typector_t *jl_new_type_ctor(jl_tuple_t *params, jl_type_t *body)
{
    jl_typector_t *tc = (jl_typector_t*)newobj((jl_type_t*)jl_typector_type,2);
    unbind_tvars(params);
    tc->parameters = params;
    tc->body = body;
    return (jl_typector_t*)tc;
}

// bits constructors ----------------------------------------------------------

#define BOXN_FUNC(nb,nw)                                        \
jl_value_t *jl_box##nb(jl_bits_type_t *t, int##nb##_t x)        \
{                                                               \
    assert(jl_is_bits_type(t));                                 \
    assert(jl_bitstype_nbits(t)/8 == sizeof(x));                \
    jl_value_t *v = alloc_##nw##w();                            \
    v->type = (jl_type_t*)t;                                    \
    *(int##nb##_t*)jl_bits_data(v) = x;                         \
    return v;                                                   \
}
BOXN_FUNC(8,  2)
BOXN_FUNC(16, 2)
BOXN_FUNC(32, 2)
#ifdef __LP64__
BOXN_FUNC(64, 2)
#else
BOXN_FUNC(64, 3)
#endif

#define UNBOX_FUNC(j_type,c_type)                                       \
c_type jl_unbox_##j_type(jl_value_t *v)                                 \
{                                                                       \
    assert(jl_is_bits_type(v->type));                                   \
    assert(jl_bitstype_nbits(v->type)/8 == sizeof(c_type));             \
    return *(c_type*)jl_bits_data(v);                                   \
}
UNBOX_FUNC(int8,   int8_t)
UNBOX_FUNC(uint8,  uint8_t)
UNBOX_FUNC(int16,  int16_t)
UNBOX_FUNC(uint16, uint16_t)
UNBOX_FUNC(int32,  int32_t)
UNBOX_FUNC(uint32, uint32_t)
UNBOX_FUNC(int64,  int64_t)
UNBOX_FUNC(uint64, uint64_t)
UNBOX_FUNC(bool,   int8_t)
UNBOX_FUNC(float32, float)
UNBOX_FUNC(float64, double)

#define BOX_FUNC(typ,c_type,pfx,nw)             \
jl_value_t *pfx##_##typ(c_type x)               \
{                                               \
    jl_value_t *v = alloc_##nw##w();            \
    v->type = (jl_type_t*)jl_##typ##_type;      \
    *(c_type*)jl_bits_data(v) = x;              \
    return v;                                   \
}
BOX_FUNC(float32, float,  jl_box, 2)
#ifdef __LP64__
BOX_FUNC(float64, double, jl_box, 2)
#else
BOX_FUNC(float64, double, jl_box, 3)
#endif

#define NBOX_C 1024

#define SIBOX_FUNC(typ,c_type,nw)                       \
static jl_value_t *boxed_##typ##_cache[NBOX_C];         \
jl_value_t *jl_box_##typ(c_type x)                      \
{                                                       \
    if ((u##c_type)(x+NBOX_C/2) < NBOX_C)               \
        return boxed_##typ##_cache[(x+NBOX_C/2)];       \
    jl_value_t *v = alloc_##nw##w();                    \
    v->type = (jl_type_t*)jl_##typ##_type;              \
    *(c_type*)jl_bits_data(v) = x;                      \
    return v;                                           \
}
#define UIBOX_FUNC(typ,c_type,nw)               \
static jl_value_t *boxed_##typ##_cache[NBOX_C]; \
jl_value_t *jl_box_##typ(c_type x)              \
{                                               \
    if (x < NBOX_C)                             \
        return boxed_##typ##_cache[x];          \
    jl_value_t *v = alloc_##nw##w();            \
    v->type = (jl_type_t*)jl_##typ##_type;      \
    *(c_type*)jl_bits_data(v) = x;              \
    return v;                                   \
}
SIBOX_FUNC(int16,  int16_t, 2)
SIBOX_FUNC(int32,  int32_t, 2)
UIBOX_FUNC(uint16, uint16_t, 2)
UIBOX_FUNC(uint32, uint32_t, 2)
UIBOX_FUNC(char,   uint32_t, 2)
#ifdef __LP64__
SIBOX_FUNC(int64,  int64_t, 2)
UIBOX_FUNC(uint64, uint64_t, 2)
#else
SIBOX_FUNC(int64,  int64_t, 3)
UIBOX_FUNC(uint64, uint64_t, 3)
#endif

static jl_value_t *boxed_int8_cache[256];
jl_value_t *jl_box_int8(int32_t x)
{
    return boxed_int8_cache[(uint8_t)x];
}
static jl_value_t *boxed_uint8_cache[256];
jl_value_t *jl_box_uint8(uint32_t x)
{
    return boxed_uint8_cache[(uint8_t)x];
}

void jl_init_int32_int64_cache(void)
{
    int64_t i;
    for(i=0; i < NBOX_C; i++) {
        boxed_int32_cache[i]  = jl_box32(jl_int32_type, i-NBOX_C/2);
        boxed_int64_cache[i]  = jl_box64(jl_int64_type, i-NBOX_C/2);
    }
}

void jl_init_box_caches(void)
{
    int64_t i;
    for(i=0; i < 256; i++) {
        boxed_int8_cache[i]  = jl_box8(jl_int8_type, i);
        boxed_uint8_cache[i] = jl_box8(jl_uint8_type, i);
    }
    for(i=0; i < NBOX_C; i++) {
        boxed_int16_cache[i]  = jl_box16(jl_int16_type, i-NBOX_C/2);
        boxed_uint16_cache[i] = jl_box16(jl_uint16_type, i);
        boxed_uint32_cache[i] = jl_box32(jl_uint32_type, i);
        boxed_char_cache[i]   = jl_box32(jl_char_type, i);
        boxed_uint64_cache[i] = jl_box64(jl_uint64_type, i);
    }
}

#ifdef JL_GC_MARKSWEEP
void jl_mark_box_caches(void)
{
    int64_t i;
    for(i=0; i < 256; i++) {
        jl_gc_markval(boxed_int8_cache[i]);
        jl_gc_markval(boxed_uint8_cache[i]);
    }
    for(i=0; i < NBOX_C; i++) {
        jl_gc_markval(boxed_int16_cache[i]);
        jl_gc_markval(boxed_int32_cache[i]);
        jl_gc_markval(boxed_int64_cache[i]);
        jl_gc_markval(boxed_uint16_cache[i]);
        jl_gc_markval(boxed_uint32_cache[i]);
        jl_gc_markval(boxed_char_cache[i]);
        jl_gc_markval(boxed_uint64_cache[i]);
    }
}
#endif

jl_value_t *jl_box_bool(int8_t x)
{
    if (x)
        return jl_true;
    return jl_false;
}

// Expr constructor for internal use ------------------------------------------

jl_expr_t *jl_exprn(jl_sym_t *head, size_t n)
{
    jl_array_t *ar = n==0 ? (jl_array_t*)jl_an_empty_cell : jl_alloc_cell_1d(n);
    JL_GC_PUSH(&ar);
    jl_expr_t *ex = (jl_expr_t*)alloc_4w();
    ex->type = (jl_type_t*)jl_expr_type;
    ex->head = head;
    ex->args = ar;
    ex->etype = (jl_value_t*)jl_any_type;
    JL_GC_POP();
    return ex;
}

// this constructor has to be built-in for bootstrapping, because we can't
// do anything without being able to make Exprs.
JL_CALLABLE(jl_f_new_expr)
{
    JL_NARGS(Expr, 3, 3);
    JL_TYPECHK(Expr, symbol, args[0]);
    if (!jl_typeis(args[1], (jl_value_t*)jl_array_any_type)) {
        jl_type_error("Expr", (jl_value_t*)jl_array_any_type, args[1]);
    }
    jl_expr_t *ex = (jl_expr_t*)alloc_4w();
    ex->type = (jl_type_t*)jl_expr_type;
    ex->head = (jl_sym_t*)args[0];
    ex->args = (jl_array_t*)args[1];
    ex->etype = args[2];
    return (jl_value_t*)ex;
}

JL_CALLABLE(jl_f_new_box)
{
    JL_NARGS(Box, 1, 1);
    jl_value_t *box = (jl_value_t*)alloc_2w();
    box->type = jl_box_any_type;
    ((jl_value_t**)box)[1] = args[0];
    return box;
}
