extern "C" {

enum TS_TYPE {
    TS_CLASS,
    TS_POINTER_SCALAR,
    TS_POINTER,
    TS_SCALAR,
    TS_CLASS_ARRAY,
    TS_ANY,
    TS_UNUSED6,
    TS_UNUSED7,
    TS_UNUSED8
};

#pragma pack(1)
struct RT_TYPE_SIG {
    TS_TYPE type;
    RT_TYPE_SIG *base;
};
#pragma pack()

}

RT_TYPE_SIG *catBaseSig(RT_TYPE_SIG *sig);
/* config/functions.txt has no name for catBaseSig. */
#pragma aux catBaseSig "fn_000fb838_"

// FUNCTION: 0xfb838
RT_TYPE_SIG *catBaseSig(RT_TYPE_SIG *sig)
{
    if (sig->type == TS_POINTER)
        sig = sig->base;
    return sig;
}

extern "C" {

int ts_equiv(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero);
/* config/functions.txt gives the type-signature comparison the name _setjmp_. */
#pragma aux ts_equiv "_setjmp_"

// FUNCTION: 0xfb841
int ts_equiv(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero)
{
    if (catch_sig == 0)
        return 1;
    catch_sig = catBaseSig(catch_sig);
    if (zero) {
        switch (catch_sig->type) {
        case TS_POINTER_SCALAR:
        case TS_UNUSED6:
        case TS_UNUSED7:
            return 1;
        }
        return 0;
    }
    if (catBaseSig(throw_sig) == catch_sig)
        return 1;
    return 0;
}

}
