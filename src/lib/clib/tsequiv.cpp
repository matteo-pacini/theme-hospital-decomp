extern "C" {

enum TS_TYPE {
    TS_CLASS,
    TS_POINTER_SCALAR,
    TS_POINTER,
    TS_SCALAR,
    TS_CLASS_ARRAY,
    TS_ANY,
    TS_POINTER_KIND6,
    TS_POINTER_KIND7,
    TS_SCALAR_KIND8
};

#pragma pack(1)
struct RT_TYPE_SIG {
    TS_TYPE type;
    RT_TYPE_SIG *base;
};
#pragma pack()

}

RT_TYPE_SIG *catBaseSig(RT_TYPE_SIG *sig);
/* The library's symbol, which names the structure rt_type_sig. */
#pragma aux catBaseSig "W?catBaseSig$n(pn$rt_type_sig$$)pn$1$"

// catBaseSig: the pointed-to type signature if sig is a pointer type, else sig.
// FUNCTION: 0xfb838
RT_TYPE_SIG *catBaseSig(RT_TYPE_SIG *sig)
{
    if (sig->type == TS_POINTER)
        sig = sig->base;
    return sig;
}

extern "C" {

int __wcpp_2_ts_equiv_(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero);

// __wcpp_2_ts_equiv__: nonzero if a catch with catch_sig (NULL = catch(...)) accepts a throw of throw_sig; with zero set (a thrown 0), only catch types 1, 6 and 7 (after stripping a pointer) match.
// FUNCTION: 0xfb841
int __wcpp_2_ts_equiv_(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero)
{
    if (catch_sig == 0)
        return 1;
    catch_sig = catBaseSig(catch_sig);
    if (zero) {
        switch (catch_sig->type) {
        case TS_POINTER_SCALAR:
        case TS_POINTER_KIND6:
        case TS_POINTER_KIND7:
            return 1;
        }
        return 0;
    }
    if (catBaseSig(throw_sig) == catch_sig)
        return 1;
    return 0;
}

}
